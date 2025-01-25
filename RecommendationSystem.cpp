// RecommendationSystem.cpp
#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <limits>
#include <queue>

void RecommendationSystem::validate_feature_vector(
    const std::vector<double>& features) const {
    if (features.empty()) {
        throw std::invalid_argument("Feature vector cannot be empty");
    }

    // Check each feature is within valid range
    for (double feature : features) {
        if (feature < 1 || feature > 10) {
            throw std::invalid_argument("Feature values must be between 1 and 10");
        }
    }

    // If there are existing movies, validate feature vector size
    if (!movies_features.empty() &&
        features.size() != movies_features.begin()->second.size()) {
        throw std::invalid_argument("Feature vector size mismatch");
    }
}

double RecommendationSystem::cosine_similarity(
    const std::vector<double>& v1, const std::vector<double>& v2) const {

    if (v1.size() != v2.size()) {
        throw std::invalid_argument("Vectors must be of equal size");
    }

    // Calculate dot product and norms
    double dot_product = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;

    for (size_t i = 0; i < v1.size(); ++i) {
        dot_product += v1[i] * v2[i];
        norm1 += v1[i] * v1[i];
        norm2 += v2[i] * v2[i];
    }

    // Handle zero vectors
    const double epsilon = std::numeric_limits<double>::epsilon();
    if (norm1 < epsilon || norm2 < epsilon) {
        return 0.0;
    }

    norm1 = std::sqrt(norm1);
    norm2 = std::sqrt(norm2);

    // Ensure result is within valid bounds [-1, 1]
    double similarity = dot_product / (norm1 * norm2);
    return std::max(-1.0, std::min(1.0, similarity));
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const {
    // Create a temporary movie for comparison
    Movie temp_movie(name, year);

    // Use lower_bound for efficient search in ordered map
    auto it = movies_features.lower_bound(std::make_shared<Movie>(temp_movie));

    if (it != movies_features.end() &&
        it->first->get_name() == name &&
        it->first->get_year() == year) {
        return it->first;
    }

    return nullptr;
}

std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const {
    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    // Calculate average rating
    double avg_rating = 0;
    for (const auto& [movie, rating] : rankings) {
        avg_rating += rating;
    }
    avg_rating /= rankings.size();

    // Initialize preference vector
    const size_t feature_count = movies_features.begin()->second.size();
    std::vector<double> preference(feature_count, 0.0);

    // Calculate weighted preferences
    for (const auto& [movie, rating] : rankings) {
        const auto& features = movies_features.at(movie);
        double weight = rating - avg_rating;
        for (size_t i = 0; i < feature_count; ++i) {
            preference[i] += weight * features[i];
        }
    }

    return preference;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const {
    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    std::vector<double> preferences = get_preference_vector(user);
    sp_movie best_movie = nullptr;
    double highest_similarity = -1.0;

    for (const auto& [movie, features] : movies_features) {
        // Skip movies the user has already rated
        if (rankings.find(movie) != rankings.end()) {
            continue;
        }

        double similarity = cosine_similarity(preferences, features);
        if (similarity > highest_similarity) {
            highest_similarity = similarity;
            best_movie = movie;
        }
    }

    return best_movie;
}

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name, int year,
                                           const std::vector<double>& features) {
    // First validate the feature vector
    validate_feature_vector(features);

    // Check if movie already exists
    sp_movie movie = get_movie(name, year);
    if (movie) {
        return movie;  // Return existing movie if found
    }

    // Create new movie and add to system
    movie = std::make_shared<Movie>(name, year);
    movies_features[movie] = features;
    return movie;
}

double RecommendationSystem::predict_movie_score(const User& user,
                                               const sp_movie& movie, int k) {
    if (!movie || k <= 0) {
        throw std::invalid_argument("Invalid movie or k parameter");
    }

    const auto& user_rankings = user.get_rank();
    if (user_rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    // Find k most similar movies that the user has rated
    std::vector<std::pair<double, sp_movie>> similarities;
    const auto& target_features = movies_features[movie];

    for (const auto& [rated_movie, rating] : user_rankings) {
        double similarity = cosine_similarity(target_features,
                                           movies_features[rated_movie]);
        similarities.emplace_back(similarity, rated_movie);
    }

    // Sort by similarity and keep top k
    std::partial_sort(similarities.begin(),
                     similarities.begin() + std::min(k, (int)similarities.size()),
                     similarities.end(),
                     [](const auto& a, const auto& b) {
                         return a.first > b.first;
                     });

    // Calculate weighted average of ratings
    double weighted_sum = 0;
    double weight_sum = 0;
    int count = 0;

    for (const auto& [similarity, similar_movie] : similarities) {
        if (count >= k) break;
        weighted_sum += similarity * user_rankings.at(similar_movie);
        weight_sum += similarity;
        count++;
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

    sp_movie best_movie = nullptr;
    double highest_score = std::numeric_limits<double>::lowest();

    // For each unwatched movie, predict score and keep track of highest
    for (const auto& [movie, features] : movies_features) {
        if (rankings.find(movie) != rankings.end()) {
            continue;  // Skip movies user has already rated
        }

        double predicted_score = predict_movie_score(user, movie, k);
        if (predicted_score > highest_score) {
            highest_score = predicted_score;
            best_movie = movie;
        }
    }

    return best_movie;
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs) {
    for (const auto& pair : rs.movies_features) {
        os << *(pair.first);  // Use Movie's operator
    }
    return os;
}