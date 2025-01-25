#include "RecommendationSystem.h"
#include "User.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name, int year,
                                             const std::vector<double>& features) {
    validate_feature_vector(features);

    // Check if movie already exists
    sp_movie existing = get_movie(name, year);
    if (existing != nullptr) {
        return existing;
    }

    sp_movie movie = std::make_shared<Movie>(name, year);
    movies_features[movie] = features;
    return movie;
}

double RecommendationSystem::cosine_similarity(const std::vector<double>& v1,
                                             const std::vector<double>& v2) const {
    if (v1.size() != v2.size()) {
        throw std::invalid_argument("Vectors must be of equal size");
    }

    double dot_product = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;

    for (size_t i = 0; i < v1.size(); ++i) {
        dot_product += v1[i] * v2[i];
        norm1 += v1[i] * v1[i];
        norm2 += v2[i] * v2[i];
    }

    norm1 = std::sqrt(norm1);
    norm2 = std::sqrt(norm2);

    // Use a small epsilon for floating-point comparison
    if (norm1 < std::numeric_limits<double>::epsilon() ||
        norm2 < std::numeric_limits<double>::epsilon()) {
        return 0.0;
    }

    double similarity = dot_product / (norm1 * norm2);

    // Ensure the result is within valid bounds [-1, 1]
    return std::max(-1.0, std::min(1.0, similarity));
}
std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const {
    if (movies_features.empty()) {
        throw std::runtime_error("No movies in recommendation system");
    }

    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    size_t features_size = movies_features.begin()->second.size();
    std::vector<double> preference(features_size, 0);

    // First, calculate the average rating for normalization
    double avg_rating = 0;
    size_t rated_count = 0;
    for (const auto& pair : rankings) {
        if (movies_features.find(pair.first) != movies_features.end()) {
            avg_rating += pair.second;
            rated_count++;
        }
    }

    if (rated_count == 0) {
        throw std::runtime_error("No valid movies found in user ratings");
    }

    avg_rating /= rated_count;

    // Now calculate the weighted preference vector
    for (const auto& pair : rankings) {
        auto feature_it = movies_features.find(pair.first);
        if (feature_it == movies_features.end()) {
            continue;
        }

        // Calculate weight based on how far the rating is from average
        double weight = pair.second - avg_rating;
        const std::vector<double>& movie_features = feature_it->second;

        for (size_t i = 0; i < features_size; ++i) {
            preference[i] += weight * movie_features[i];
        }
    }

    // Normalize the preference vector
    double norm = 0;
    for (double val : preference) {
        norm += val * val;
    }
    norm = std::sqrt(norm);

    if (norm > std::numeric_limits<double>::epsilon()) {
        for (double& val : preference) {
            val /= norm;
        }
    }

    return preference;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const {
    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    std::vector<double> preference;
    try {
        preference = get_preference_vector(user);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to calculate preference vector: " +
                               std::string(e.what()));
    }

    // Create a priority queue to store top recommendations
    std::vector<std::pair<double, sp_movie>> recommendations;
    recommendations.reserve(movies_features.size());

    for (const auto& pair : movies_features) {
        // Skip movies the user has already rated
        if (rankings.find(pair.first) != rankings.end()) {
            continue;
        }

        try {
            double similarity = cosine_similarity(preference, pair.second);
            recommendations.emplace_back(similarity, pair.first);
        } catch (const std::exception& e) {
            continue; // Skip movies that cause errors
        }
    }

    // Sort recommendations by similarity score
    std::sort(recommendations.begin(), recommendations.end(),
              [](const auto& a, const auto& b) {
                  if (std::abs(a.first - b.first) <
                      std::numeric_limits<double>::epsilon()) {
                      // If similarities are equal, use movie properties as tiebreaker
                      return *(a.second) < *(b.second);
                  }
                  return a.first > b.first;
              });

    if (recommendations.empty()) {
        throw std::runtime_error("No suitable movies found for recommendation");
    }

    return recommendations[0].second;
}


double RecommendationSystem::predict_movie_score(const User& user,
                                               const sp_movie& movie, int k) {
    if (!movie || k <= 0) {
        throw std::invalid_argument("Invalid movie or k value");
    }

    auto movie_features = movies_features.find(movie);
    if (movie_features == movies_features.end()) {
        throw std::runtime_error("Movie not found in system");
    }

    const auto& user_rankings = user.get_rank();
    if (user_rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    // Calculate similarities between the target movie and all rated movies
    std::vector<std::pair<double, double>> similarities_and_ratings;
    similarities_and_ratings.reserve(user_rankings.size());

    for (const auto& [rated_movie, rating] : user_rankings) {
        auto rated_features = movies_features.find(rated_movie);
        if (rated_features == movies_features.end()) continue;

        double similarity = cosine_similarity(movie_features->second,
                                           rated_features->second);
        similarities_and_ratings.emplace_back(similarity, rating);
    }

    // Sort by similarity and get top k
    std::sort(similarities_and_ratings.begin(), similarities_and_ratings.end(),
              std::greater<std::pair<double, double>>());

    double weighted_sum = 0.0;
    double weight_sum = 0.0;
    size_t used_neighbors = 0;

    // Use only positive similarities for prediction
    for (const auto& [similarity, rating] : similarities_and_ratings) {
        if (similarity <= 0 || used_neighbors >= k) break;

        weighted_sum += similarity * rating;
        weight_sum += std::abs(similarity);
        used_neighbors++;
    }

    if (weight_sum < std::numeric_limits<double>::epsilon()) {
        // If no good similarities found, return average rating
        return std::accumulate(user_rankings.begin(), user_rankings.end(), 0.0,
                             [](double sum, const auto& pair) {
                                 return sum + pair.second;
                             }) / user_rankings.size();
    }

    return weighted_sum / weight_sum;
}
sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) {
    if (k <= 0) {
        throw std::invalid_argument("k must be positive");
    }

    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    if (movies_features.empty()) {
        throw std::runtime_error("No movies in recommendation system");
    }

    double max_predicted_score = std::numeric_limits<double>::lowest();
    sp_movie best_movie = nullptr;

    for (const auto& pair : movies_features) {
        // Ensure the movie pointer is valid
        if (!pair.first) {
            continue;
        }

        // Skip movies the user has already rated
        if (rankings.find(pair.first) != rankings.end()) {
            continue;
        }

        try {
            double predicted_score = predict_movie_score(user, pair.first, k);
            if (predicted_score > max_predicted_score) {
                max_predicted_score = predicted_score;
                best_movie = pair.first;
            }
        } catch (const std::exception& e) {
            // Log error and continue with next movie
            continue;
        }
    }

    if (!best_movie) {
        throw std::runtime_error("No suitable movie found for recommendation");
    }

    return best_movie;
}

void RecommendationSystem::validate_feature_vector(const std::vector<double>& features) const {
    if (features.empty()) {
        throw std::invalid_argument("Feature vector cannot be empty");
    }
    for (double feature : features) {
        if (feature < 1 || feature > 10) {
            throw std::invalid_argument("Feature values must be between 1 and 10");
        }
    }
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const {
    for (const auto& pair : movies_features) {
        if (pair.first->get_name() == name && pair.first->get_year() == year) {
            return pair.first;
        }
    }
    return nullptr;
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs) {
    for (const auto& pair : rs.movies_features) {
        os << *(pair.first);
    }
    return os;
}