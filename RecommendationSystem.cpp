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

    if (std::abs(norm1) < std::numeric_limits<double>::epsilon() ||
        std::abs(norm2) < std::numeric_limits<double>::epsilon()) {
        return 0;
    }

    return dot_product / (norm1 * norm2);
}

std::vector<double> RecommendationSystem::get_preference_vector(
    const User& user) const {

    if (movies_features.empty()) {
        throw std::runtime_error("No movies in recommendation system");
    }

    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    size_t features_size = movies_features.begin()->second.size();
    std::vector<double> preference(features_size, 0);

    // Calculate average rating
    double avg_rating = 0;
    for (const auto& pair : rankings) {
        avg_rating += pair.second;
    }
    avg_rating /= rankings.size();

    // Calculate preference vector
    for (const auto& pair : rankings) {
        double normalized_rating = pair.second - avg_rating;
        const auto& movie_features = movies_features.at(pair.first);

        for (size_t i = 0; i < features_size; ++i) {
            preference[i] += normalized_rating * movie_features[i];
        }
    }

    return preference;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const {
    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    std::vector<double> preference = get_preference_vector(user);
    double max_similarity = -1;
    sp_movie best_movie = nullptr;

    for (const auto& pair : movies_features) {
        // Skip movies the user has already rated
        if (rankings.find(pair.first) != rankings.end()) {
            continue;
        }

        double similarity = cosine_similarity(preference, pair.second);
        if (similarity > max_similarity) {
            max_similarity = similarity;
            best_movie = pair.first;
        }
    }

    if (best_movie == nullptr) {
        throw std::runtime_error("No suitable movie found for recommendation");
    }

    return best_movie;
}

double RecommendationSystem::predict_movie_score(const User& user,
                                               const sp_movie& movie, int k) {
    if (k <= 0) {
        throw std::invalid_argument("k must be positive");
    }

    if (movie == nullptr) {
        throw std::invalid_argument("Invalid movie pointer");
    }

    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    // Calculate similarities with all rated movies
    std::vector<std::pair<double, sp_movie>> similarities;
    similarities.reserve(rankings.size());

    for (const auto& pair : rankings) {
        double sim = cosine_similarity(movies_features[movie],
                                     movies_features[pair.first]);
        similarities.push_back({sim, pair.first});
    }

    // Sort by similarity in descending order
    std::sort(similarities.begin(), similarities.end(),
              std::greater<std::pair<double, sp_movie>>());

    // Take top k similarities
    size_t actual_k = std::min(static_cast<size_t>(k), similarities.size());
    double weighted_sum = 0;
    double weight_sum = 0;

    for (size_t i = 0; i < actual_k; ++i) {
        const auto& pair = similarities[i];
        weighted_sum += pair.first * rankings.at(pair.second);
        weight_sum += std::abs(pair.first);
    }

    if (std::abs(weight_sum) < std::numeric_limits<double>::epsilon()) {
        return 0;
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

    double max_predicted_score = -1;
    sp_movie best_movie = nullptr;

    for (const auto& pair : movies_features) {
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
            continue; // Skip movies that cause prediction errors
        }
    }

    if (best_movie == nullptr) {
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