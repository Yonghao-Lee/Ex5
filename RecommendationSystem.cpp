// RecommendationSystem.cpp
#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <limits>
#include <queue>



double RecommendationSystem::cosine_similarity(
    const std::vector<double>& v1, const std::vector<double>& v2) const {
    // More robust similarity calculation
    try {
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

        // Handle zero vectors more robustly
        const double epsilon = 1e-10;  // Increased from numeric_limits for stability
        if (norm1 < epsilon || norm2 < epsilon) {
            return 0.0;
        }

        norm1 = std::sqrt(norm1);
        norm2 = std::sqrt(norm2);

        double similarity = dot_product / (norm1 * norm2);
        // Ensure result is strictly within [-1, 1]
        return std::max(-1.0, std::min(1.0, similarity));
    } catch (const std::exception&) {
        return -1.0;  // Return minimum similarity on error
    }
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const {
    // More efficient movie lookup
    Movie temp_movie(name, year);
    auto sp_temp = std::make_shared<Movie>(temp_movie);

    auto it = movies_features.find(sp_temp);
    if (it != movies_features.end()) {
        return it->first;
    }

    // Fallback to linear search if needed
    for (const auto& [movie, _] : movies_features) {
        if (movie->get_name() == name && movie->get_year() == year) {
            return movie;
        }
    }

    return nullptr;
}

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name, int year,
                                          const std::vector<double>& features) {
    try {
        validate_feature_vector(features);

        // First check if movie exists
        sp_movie existing = get_movie(name, year);
        if (existing) {
            return existing;
        }

        // Create new movie
        sp_movie movie = std::make_shared<Movie>(name, year);
        movies_features[movie] = features;
        return movie;
    } catch (const std::exception& e) {
        // Log error and rethrow
        std::cerr << "Error adding movie: " << e.what() << std::endl;
        throw;
    }
}

std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const {
    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    // Calculate average rating more robustly
    double avg_rating = 0;
    size_t valid_ratings = 0;
    for (const auto& [movie, rating] : rankings) {
        if (movies_features.find(movie) != movies_features.end()) {
            avg_rating += rating;
            valid_ratings++;
        }
    }

    if (valid_ratings == 0) {
        throw std::runtime_error("No valid movies found in user ratings");
    }

    avg_rating /= valid_ratings;

    // Initialize preference vector based on first movie's features
    const size_t feature_count = movies_features.begin()->second.size();
    std::vector<double> preference(feature_count, 0.0);

    // Calculate weighted preferences
    for (const auto& [movie, rating] : rankings) {
        auto it = movies_features.find(movie);
        if (it == movies_features.end()) continue;

        double weight = rating - avg_rating;
        const auto& features = it->second;

        for (size_t i = 0; i < feature_count; ++i) {
            preference[i] += weight * features[i];
        }
    }

    // Normalize preference vector
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

void RecommendationSystem::validate_feature_vector(const std::vector<double>& features) const {
    if (features.empty()) {
        throw std::invalid_argument("Feature vector cannot be empty");
    }

    // Validate feature values and size consistency
    for (double feature : features) {
        if (feature < 1 || feature > 10) {
            throw std::invalid_argument("Feature values must be between 1 and 10");
        }
    }

    // Check vector size consistency with existing movies
    if (!movies_features.empty() &&
        features.size() != movies_features.begin()->second.size()) {
        throw std::invalid_argument("Feature vector size mismatch");
    }
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const {
    try {
        // Get user's preference vector
        std::vector<double> preferences = get_preference_vector(user);

        // Find movie with highest similarity to preferences
        sp_movie best_movie = nullptr;
        double highest_similarity = -1.0;
        const auto& rankings = user.get_rank();

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
    } catch (const std::exception&) {
        return nullptr;
    }
}

double RecommendationSystem::predict_movie_score(const User& user_rankings,
                                               const sp_movie& movie, int k) {
    if (!movie || k <= 0) {
        throw std::invalid_argument("Invalid movie or k parameter");
    }

    const auto& rankings = user_rankings.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    // Get target movie features
    const auto& target_features = movies_features[movie];

    // Calculate similarities with rated movies
    std::vector<std::pair<double, double>> similarities_and_ratings;
    for (const auto& [rated_movie, rating] : rankings) {
        const auto& rated_features = movies_features[rated_movie];
        double similarity = cosine_similarity(target_features, rated_features);
        if (similarity > -1.0) {  // Valid similarity
            similarities_and_ratings.emplace_back(similarity, rating);
        }
    }

    if (similarities_and_ratings.empty()) {
        throw std::runtime_error("No valid similarities found");
    }

    // Sort and keep top k
    std::partial_sort(similarities_and_ratings.begin(),
                     similarities_and_ratings.begin() +
                         std::min(k, (int)similarities_and_ratings.size()),
                     similarities_and_ratings.end(),
                     [](const auto& a, const auto& b) {
                         return a.first > b.first;
                     });

    // Calculate weighted average
    double sum_weights = 0, weighted_sum = 0;
    int processed = 0;
    for (const auto& [similarity, rating] : similarities_and_ratings) {
        if (processed >= k) break;
        weighted_sum += similarity * rating;
        sum_weights += similarity;
        processed++;
    }

    if (sum_weights < std::numeric_limits<double>::epsilon()) {
        throw std::runtime_error("Sum of weights too small");
    }

    return weighted_sum / sum_weights;
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user_rankings, int k) {
    if (k <= 0 || user_rankings.get_rank().empty()) {
        return nullptr;
    }

    try {
        sp_movie best_movie = nullptr;
        double highest_score = std::numeric_limits<double>::lowest();

        // Find unwatched movie with highest predicted score
        for (const auto& [movie, _] : movies_features) {
            if (user_rankings.get_rank().find(movie) != user_rankings.get_rank().end()) {
                continue;  // Skip watched movies
            }

            double predicted_score = predict_movie_score(user_rankings, movie, k);
            if (predicted_score > highest_score) {
                highest_score = predicted_score;
                best_movie = movie;
            }
        }

        return best_movie;
    } catch (const std::exception&) {
        return nullptr;
    }
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs) {
    std::vector<sp_movie> sorted_movies;
    for (const auto& [movie, _] : rs.movies_features) {
        sorted_movies.push_back(movie);
    }

    std::sort(sorted_movies.begin(), sorted_movies.end(),
              [](const sp_movie& a, const sp_movie& b) {
                  return *a < *b;
              });

    for (size_t i = 0; i < sorted_movies.size(); ++i) {
        os << *sorted_movies[i];
        if (i < sorted_movies.size() - 1) {
            os << std::endl;
        }
    }
    return os;
}