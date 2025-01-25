#include "RecommendationSystem.h"
#include "User.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name, int year,
                                             const std::vector<double>& features) {
    for (double feature : features) {
        if (feature < 1 || feature > 10) {
            throw std::runtime_error("Feature values must be between 1 and 10");
        }
    }

    sp_movie movie = std::make_shared<Movie>(name, year);
    movies_features[movie] = features;
    return movie;
}

sp_movie RecommendationSystem::get_movie(const std::string& name, 
                                       int year) const {
    for (const auto& pair : movies_features) {
        if (pair.first->get_name() == name && 
            pair.first->get_year() == year) {
            return pair.first;
        }
    }
    return nullptr;
}

double RecommendationSystem::cosine_similarity(const std::vector<double>& v1,
                                             const std::vector<double>& v2) const {
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

    if (norm1 == 0 || norm2 == 0) return 0;
    return dot_product / (norm1 * norm2);
}

std::vector<double> RecommendationSystem::get_preference_vector(
    const User& user) const {
    const auto& rankings = user.get_rank();
    
    double avg_rating = 0;
    for (const auto& pair : rankings) {
        avg_rating += pair.second;
    }
    avg_rating /= rankings.size();

    size_t features_size = movies_features.begin()->second.size();
    std::vector<double> preference(features_size, 0);

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
    std::vector<double> preference = get_preference_vector(user);
    double max_similarity = -1;
    sp_movie best_movie;
    const auto& rankings = user.get_rank();

    for (const auto& pair : movies_features) {
        if (rankings.find(pair.first) != rankings.end()) continue;

        double similarity = cosine_similarity(preference, pair.second);
        if (similarity > max_similarity) {
            max_similarity = similarity;
            best_movie = pair.first;
        }
    }

    return best_movie;
}

double RecommendationSystem::predict_movie_score(const User& user, 
                                               const sp_movie& movie, int k) {
    const auto& rankings = user.get_rank();
    std::vector<std::pair<double, sp_movie>> similarities;

    for (const auto& pair : rankings) {
        double sim = cosine_similarity(movies_features[movie], 
                                     movies_features[pair.first]);
        similarities.push_back({sim, pair.first});
    }

    std::sort(similarities.rbegin(), similarities.rend());
    similarities.resize(std::min(k, static_cast<int>(similarities.size())));

    double weighted_sum = 0;
    double weight_sum = 0;

    for (const auto& pair : similarities) {
        weighted_sum += pair.first * rankings.at(pair.second);
        weight_sum += pair.first;
    }

    return weighted_sum / weight_sum;
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) {
    const auto& rankings = user.get_rank();
    double max_predicted_score = -1;
    sp_movie best_movie;

    for (const auto& pair : movies_features) {
        if (rankings.find(pair.first) != rankings.end()) continue;

        double predicted_score = predict_movie_score(user, pair.first, k);
        if (predicted_score > max_predicted_score) {
            max_predicted_score = predicted_score;
            best_movie = pair.first;
        }
    }

    return best_movie;
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs) {
    for (const auto& pair : rs.movies_features) {
        os << *(pair.first);
    }
    return os;
}