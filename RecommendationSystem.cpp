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