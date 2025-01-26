#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <limits>
#include <queue>
#include <cmath>      // for std::sqrt
#include <stdexcept>  // for exceptions
#include <iostream>   // for std::cerr

double RecommendationSystem::cosine_similarity(
    const std::vector<double>& v1, const std::vector<double>& v2) const {
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

        const double epsilon = 1e-10;
        if (norm1 < epsilon || norm2 < epsilon) {
            // If either vector is effectively zero-length, similarity is 0
            return 0.0;
        }

        norm1 = std::sqrt(norm1);
        norm2 = std::sqrt(norm2);

        double similarity = dot_product / (norm1 * norm2);
        // clamp to [-1, 1]
        if (similarity < -1.0) similarity = -1.0;
        if (similarity > 1.0)  similarity = 1.0;

        return similarity;
    } catch (const std::exception&) {
        // On error, return worst-case similarity
        return -1.0;
    }
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const {
    // Attempt direct lookup
    sp_movie temp = std::make_shared<Movie>(name, year);
    auto it = movies_features.find(temp);
    if (it != movies_features.end()) {
        return it->first;
    }

    // Otherwise fallback to linear search
    for (const auto& [movie, _] : movies_features) {
        if (movie->get_name() == name && movie->get_year() == year) {
            return movie;
        }
    }
    return nullptr;
}

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name,
                                               int year,
                                               const std::vector<double>& features) {
    validate_feature_vector(features);

    // Check if movie already exists
    sp_movie existing = get_movie(name, year);
    if (existing) {
        return existing;
    }

    // Otherwise create it
    sp_movie movie = std::make_shared<Movie>(name, year);
    movies_features[movie] = features;
    return movie;
}

std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const {
    const auto& rankings = user.get_rank();
    if (rankings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    // Compute average rating
    double sum_ratings = 0.0;
    int count = 0;
    for (const auto& [mv, rating] : rankings) {
        if (movies_features.find(mv) != movies_features.end()) {
            sum_ratings += rating;
            ++count;
        }
    }
    if (count == 0) {
        throw std::runtime_error("No valid rated movies in the system for this user");
    }

    double avg = sum_ratings / count;

    // Build weighted preference vector
    size_t feature_count = movies_features.begin()->second.size();
    std::vector<double> preference(feature_count, 0.0);

    for (const auto& [mv, rating] : rankings) {
        auto it = movies_features.find(mv);
        if (it == movies_features.end()) {
            continue;
        }
        double weight = rating - avg;
        const std::vector<double>& feats = it->second;
        for (size_t i = 0; i < feature_count; i++) {
            preference[i] += weight * feats[i];
        }
    }

    // Normalize
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

void RecommendationSystem::validate_feature_vector(const std::vector<double>& features) const {
    if (features.empty()) {
        throw std::invalid_argument("Feature vector cannot be empty");
    }
    for (double f : features) {
        if (f < 1.0 || f > 10.0) {
            throw std::invalid_argument("Feature values must be between 1 and 10");
        }
    }
    if (!movies_features.empty()) {
        // Compare size with an existing movie's vector
        size_t existing_size = movies_features.begin()->second.size();
        if (features.size() != existing_size) {
            throw std::invalid_argument("Feature vector size mismatch");
        }
    }
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const {
    try {
        std::vector<double> pref = get_preference_vector(user);

        sp_movie best = nullptr;
        double best_score = -1.0;

        const auto& user_ratings = user.get_rank();
        for (const auto& [movie, feats] : movies_features) {
            // Skip already-rated movies
            if (user_ratings.find(movie) != user_ratings.end()) {
                continue;
            }
            double sim = cosine_similarity(pref, feats);
            if (sim > best_score) {
                best_score = sim;
                best = movie;
            }
        }
        return best;
    } catch (...) {
        // On any error, return null
        return nullptr;
    }
}

double RecommendationSystem::predict_movie_score(const User& user, const sp_movie& movie, int k) {
    if (!movie) {
        throw std::invalid_argument("Cannot predict score for null movie");
    }
    if (k <= 0) {
        throw std::invalid_argument("k must be positive");
    }

    const auto& user_ratings = user.get_rank();
    if (user_ratings.empty()) {
        throw std::runtime_error("User has no ratings");
    }

    // Get the target movie's features
    auto it = movies_features.find(movie);
    if (it == movies_features.end()) {
        throw std::runtime_error("Movie not found in the system");
    }
    const std::vector<double>& target_features = it->second;

    // Calculate similarity to each rated movie
    std::vector<std::pair<double, double>> sims; // (similarity, rating)
    sims.reserve(user_ratings.size());
    for (const auto& [rated_mv, rating] : user_ratings) {
        auto it_rated = movies_features.find(rated_mv);
        if (it_rated == movies_features.end()) {
            continue;
        }
        double sim = cosine_similarity(target_features, it_rated->second);
        if (sim > -1.0) {
            sims.push_back({sim, rating});
        }
    }
    if (sims.empty()) {
        throw std::runtime_error("No valid similarities found for user");
    }

    // partial_sort top k by sim descending
    if ((int)sims.size() > k) {
        std::partial_sort(
            sims.begin(), sims.begin() + k, sims.end(),
            [](auto const & a, auto const & b){
                return a.first > b.first;
            }
        );
        sims.resize(k);
    } else {
        std::sort(sims.begin(), sims.end(),
                  [](auto const & a, auto const & b){
                      return a.first > b.first;
                  });
    }

    // Weighted average
    double sum_sim = 0.0;
    double sum_weighted = 0.0;
    for (auto & [similarity, user_rating] : sims) {
        sum_sim += similarity;
        sum_weighted += similarity * user_rating;
    }

    if (sum_sim < 1e-12) {
        throw std::runtime_error("Sum of similarities too small");
    }
    return sum_weighted / sum_sim;
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) {
    if (k <= 0) {
        return nullptr;
    }
    const auto& user_ratings = user.get_rank();
    if (user_ratings.empty()) {
        return nullptr;
    }

    try {
        sp_movie best;
        double best_score = std::numeric_limits<double>::lowest();

        for (const auto& [movie, feats] : movies_features) {
            // Skip movies user already rated
            if (user_ratings.find(movie) != user_ratings.end()) {
                continue;
            }
            double score = predict_movie_score(user, movie, k);
            if (score > best_score) {
                best_score = score;
                best = movie;
            }
        }
        return best;
    } catch (...) {
        return nullptr;
    }
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs) {
    // Retrieve all movies and sort them
    std::vector<sp_movie> sorted;
    sorted.reserve(rs.get_movies().size());
    for (auto const & [mv, _] : rs.get_movies()) {
        sorted.push_back(mv);
    }

    std::sort(sorted.begin(), sorted.end(),
              [](const sp_movie& a, const sp_movie& b){
                  return *a < *b; // uses Movie::operator<
              });

    // Print each movie on its own line
    for (auto const & mv : sorted) {
        os << *mv << "\n";
    }

    return os;
}