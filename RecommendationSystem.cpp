#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <limits>
#include <cmath>

double RecommendationSystem::cosine_similarity(
    const std::vector<double>& v1, const std::vector<double>& v2) const {
    try {
        if (v1.size() != v2.size()) {
            throw std::invalid_argument("Vectors must be of equal size");
        }

        double dot_product = 0.0, norm1 = 0.0, norm2 = 0.0;
        for (size_t i = 0; i < v1.size(); ++i) {
            dot_product += v1[i] * v2[i];
            norm1 += v1[i] * v1[i];
            norm2 += v2[i] * v2[i];
        }

        const double epsilon = 1e-10;
        if (norm1 < epsilon || norm2 < epsilon) {
            return 0.0;
        }

        norm1 = std::sqrt(norm1);
        norm2 = std::sqrt(norm2);

        double similarity = dot_product / (norm1 * norm2);
        return std::max(-1.0, std::min(1.0, similarity));
    } catch (...) {
        return -1.0;
    }
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const {
    sp_movie temp = std::make_shared<Movie>(name, year);
    auto it = movies_features.find(temp);
    if (it != movies_features.end()) {
        return it->first;
    }
    return nullptr;
}

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name,
                                              int year,
                                              const std::vector<double>& features) {
    validate_feature_vector(features);
    sp_movie movie = get_movie(name, year);
    if (!movie) {
        movie = std::make_shared<Movie>(name, year);
        movies_features[movie] = features;
    }
    return movie;
}

void RecommendationSystem::validate_feature_vector(const std::vector<double>& features) const {
    if (features.empty()) {
        throw std::invalid_argument("Feature vector cannot be empty");
    }
    for (double f : features) {
        if (f < 1.0 || f > 10.0) {
            throw std::invalid_argument("Feature values must be between 1 and 10");
        }
    }
    if (!movies_features.empty() &&
        features.size() != movies_features.begin()->second.size()) {
        throw std::invalid_argument("Feature vector size mismatch");
    }
}

std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const {
    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    double avg_rating = 0.0;
    size_t count = 0;
    for (const auto& [mv, rating] : rankings) {
        if (movies_features.find(mv) != movies_features.end()) {
            avg_rating += rating;
            ++count;
        }
    }

    if (count == 0) {
        throw std::runtime_error("No valid rated movies found");
    }
    avg_rating /= count;

    size_t feature_count = movies_features.begin()->second.size();
    std::vector<double> preference(feature_count, 0.0);

    for (const auto& [mv, rating] : rankings) {
        auto it = movies_features.find(mv);
        if (it == movies_features.end()) continue;

        double weight = rating - avg_rating;
        const auto& feats = it->second;
        for (size_t i = 0; i < feature_count; ++i) {
            preference[i] += weight * feats[i];
        }
    }

    double norm = 0.0;
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
    try {
        if (user.get_rank().empty()) {
            return nullptr;
        }
        const auto pref = get_preference_vector(user);
        sp_movie best = nullptr;
        double best_sim = -1.0;

        for (const auto& [movie, feats] : movies_features) {
            if (user.get_rank().count(movie) > 0) continue;

            double sim = cosine_similarity(pref, feats);
            if (sim > best_sim) {
                best_sim = sim;
                best = movie;
            }
        }
        return best;
    } catch (...) {
        return nullptr;
    }
}

double RecommendationSystem::predict_movie_score(const User& user, const sp_movie& movie, int k) {
    if (!movie || k <= 0) throw std::invalid_argument("Invalid parameters");
    const auto& rankings = user.get_rank();
    if (rankings.empty()) throw std::runtime_error("No ratings");

    auto target_it = movies_features.find(movie);
    if (target_it == movies_features.end()) throw std::runtime_error("Movie not found");

    std::vector<std::pair<double, double>> similarities;
    for (const auto& [rated_movie, rating] : rankings) {
        auto feat_it = movies_features.find(rated_movie);
        if (feat_it == movies_features.end()) continue;

        double sim = cosine_similarity(target_it->second, feat_it->second);
        if (sim > -1.0) similarities.emplace_back(sim, rating);
    }

    if (similarities.empty()) throw std::runtime_error("No similarities");

    std::partial_sort(similarities.begin(),
                     similarities.begin() + std::min(k, (int)similarities.size()),
                     similarities.end(),
                     [](const auto& a, const auto& b) { return a.first > b.first; });

    double weighted_sum = 0, sum_weights = 0;
    for (int i = 0; i < std::min(k, (int)similarities.size()); ++i) {
        weighted_sum += similarities[i].first * similarities[i].second;
        sum_weights += similarities[i].first;
    }

    if (sum_weights < 1e-10) throw std::runtime_error("Invalid weights");
    return weighted_sum / sum_weights;
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) {
    if (k <= 0) return nullptr;
    const auto& rankings = user.get_rank();
    if (rankings.empty()) return nullptr;

    try {
        sp_movie best = nullptr;
        double best_score = -std::numeric_limits<double>::infinity();

        for (const auto& [movie, _] : movies_features) {
            if (rankings.count(movie)) continue;
            try {
                double score = predict_movie_score(user, movie, k);
                if (score > best_score) {
                    best_score = score;
                    best = movie;
                }
            } catch (...) {
                continue;
            }
        }
        return best;
    } catch (...) {
        return nullptr;
    }
}
std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs) {
    std::vector<sp_movie> sorted;
    for (const auto& [movie, _] : rs.movies_features) {
        sorted.push_back(movie);
    }

    std::sort(sorted.begin(), sorted.end(),
              [](const sp_movie& a, const sp_movie& b) {
                  return *a < *b;
              });

    // Key change: Don't add newline if it's the last item
    for (size_t i = 0; i < sorted.size(); ++i) {
        os << *sorted[i];
    }
    return os;
}