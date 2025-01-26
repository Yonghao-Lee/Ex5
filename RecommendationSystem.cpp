#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <iostream>

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
    sp_movie found = get_movie(name, year);
    if (found) {
        return found;
    }
    sp_movie mv = std::make_shared<Movie>(name, year);
    movies_features[mv] = features;
    return mv;
}

double RecommendationSystem::cosine_similarity(const std::vector<double>& v1,
                                               const std::vector<double>& v2) const
{
    if (v1.size() != v2.size()) {
        return -1.0;
    }
    double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
    for (size_t i = 0; i < v1.size(); i++) {
        dot += v1[i] * v2[i];
        norm1 += v1[i] * v1[i];
        norm2 += v2[i] * v2[i];
    }
    if (norm1 < 1e-10 || norm2 < 1e-10) {
        return 0.0;
    }
    return dot / (std::sqrt(norm1) * std::sqrt(norm2));
}

void RecommendationSystem::validate_feature_vector(const std::vector<double>& features) const {
    if (features.empty()) {
        throw std::invalid_argument("Empty feature vector");
    }
    for (double f : features) {
        if (f < 1.0 || f > 10.0) {
            throw std::invalid_argument("Features must be between 1..10");
        }
    }
    if (!movies_features.empty()) {
        size_t dim = movies_features.begin()->second.size();
        if (features.size() != dim) {
            throw std::invalid_argument("Feature size mismatch");
        }
    }
}

std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const {
    const auto& rank = user.get_rank();
    if (rank.empty()) {
        throw std::runtime_error("No ratings to build preference from");
    }
    double sumRating = 0.0;
    int count = 0;
    for (auto const& [movie, rating] : rank) {
        if (movies_features.find(movie) != movies_features.end()) {
            sumRating += rating;
            ++count;
        }
    }
    if (count == 0) {
        throw std::runtime_error("No valid rated movies for this user");
    }
    double avg = sumRating / count;

    size_t dim = movies_features.begin()->second.size();
    std::vector<double> pref(dim, 0.0);

    for (auto const& [movie, rating] : rank) {
        auto it = movies_features.find(movie);
        if (it == movies_features.end()) continue;

        double weight = rating - avg;
        const auto& feats = it->second;
        for (size_t i = 0; i < dim; i++) {
            pref[i] += weight * feats[i];
        }
    }

    double norm = 0.0;
    for (double val : pref) {
        norm += val*val;
    }
    norm = std::sqrt(norm);
    if (norm > 1e-10) {
        for (double &val : pref) {
            val /= norm;
        }
    }
    return pref;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const {
    std::vector<double> pref = get_preference_vector(user);

    sp_movie best = nullptr;
    double bestSim = -1.0;
    const auto& rank = user.get_rank();

    for (auto const& [movie, feats] : movies_features) {
        // skip if user already rated
        if (rank.find(movie) != rank.end()) {
            continue;
        }
        double sim = cosine_similarity(pref, feats);
        if (sim > bestSim) {
            bestSim = sim;
            best = movie;
        }
    }
    if (!best) {
        throw std::runtime_error("No content-based recommendation found");
    }
    return best;
}

double RecommendationSystem::predict_movie_score(const User& user,
                                                 const sp_movie& movie,
                                                 int k) const
{
    if (!movie) {
        throw std::invalid_argument("Null movie pointer to predict");
    }
    if (k <= 0) {
        throw std::invalid_argument("k must be positive");
    }
    const auto& rank = user.get_rank();
    if (rank.empty()) {
        throw std::runtime_error("User has no ratings");
    }
    auto it = movies_features.find(movie);
    if (it == movies_features.end()) {
        throw std::runtime_error("Movie not in system");
    }
    const auto& targetFeats = it->second;

    std::vector<std::pair<double, double>> sims;
    sims.reserve(rank.size());
    for (auto const& [m, rating] : rank) {
        auto it2 = movies_features.find(m);
        if (it2 == movies_features.end()) {
            continue;
        }
        double sim = cosine_similarity(targetFeats, it2->second);
        if (sim >= -1.0) {
            sims.emplace_back(sim, rating);
        }
    }
    if (sims.empty()) {
        throw std::runtime_error("No similarities found for user");
    }

    // partial sort top k
    if (static_cast<int>(sims.size()) > k) {
        std::partial_sort(sims.begin(), sims.begin() + k, sims.end(),
                          [](auto &a, auto &b) { return a.first > b.first; });
        sims.resize(k);
    } else {
        std::sort(sims.begin(), sims.end(),
                  [](auto &a, auto &b){ return a.first > b.first; });
    }

    double sumW = 0.0, sumWR = 0.0;
    for (auto const& [sim, userRating] : sims) {
        sumW  += sim;
        sumWR += sim*userRating;
    }
    if (sumW < 1e-10) {
        throw std::runtime_error("Sum of similarities too small");
    }
    return sumWR / sumW;
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) const {
    if (k <= 0) {
        throw std::invalid_argument("k must be positive");
    }
    const auto& rank = user.get_rank();
    if (rank.empty()) {
        throw std::runtime_error("User has no ratings (CF).");
    }

    sp_movie best = nullptr;
    double bestScore = -std::numeric_limits<double>::infinity();
    for (auto const& [movie, feats] : movies_features) {
        if (rank.find(movie) != rank.end()) {
            continue;
        }
        try {
            double score = predict_movie_score(user, movie, k);
            if (score > bestScore) {
                bestScore = score;
                best = movie;
            }
        } catch (...) {
            // skip if predict fails
        }
    }
    if (!best) {
        throw std::runtime_error("No CF recommendation found");
    }
    return best;
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs)
{
    std::vector<sp_movie> sorted;
    sorted.reserve(rs.get_movies().size());
    for (auto const& [mv, feats] : rs.get_movies()) {
        sorted.push_back(mv);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const sp_movie& a, const sp_movie& b){
                  return *a < *b;
              });
    for (auto const& mv : sorted) {
        os << mv->get_name() << " (" << mv->get_year() << ")\n";
    }
    return os;
}