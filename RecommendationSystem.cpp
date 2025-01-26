/***************************************
 *  RecommendationSystem.cpp
 ***************************************/
#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <stdexcept>
#include <limits>

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name,
                                               int year,
                                               const std::vector<double>& features)
{
    // throw if any feature is out of [1..10]
    for (double f : features) {
        if (f < 1.0 || f > 10.0) {
            throw std::runtime_error("Feature out of range");
        }
    }
    sp_movie mv = std::make_shared<Movie>(name, year);
    movies_features[mv] = features;
    return mv;
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const
{
    for (auto &kv : movies_features) {
        if (kv.first->get_name() == name && kv.first->get_year() == year) {
            return kv.first;
        }
    }
    return nullptr;
}

double RecommendationSystem::cosine_similarity(const std::vector<double>& v1,
                                               const std::vector<double>& v2) const
{
    if (v1.size() != v2.size() || v1.empty()) {
        return 0.0;
    }
    double dot = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;
    for (size_t i = 0; i < v1.size(); i++) {
        dot   += v1[i] * v2[i];
        norm1 += v1[i] * v1[i];
        norm2 += v2[i] * v2[i];
    }
    double denom = std::sqrt(norm1) * std::sqrt(norm2);
    if (denom == 0.0) {
        return 0.0;
    }
    return dot / denom;
}

std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const
{
    const auto &ranks = user.get_rank();
    if (ranks.empty() || movies_features.empty()) {
        return {};
    }
    // user average rating
    double sum_r = 0.0;
    for (auto &p : ranks) {
        sum_r += p.second;
    }
    double avg = sum_r / ranks.size();

    // accumulate (rating-avg)*features
    size_t dim = movies_features.begin()->second.size();
    std::vector<double> pref(dim, 0.0);

    for (auto &p : ranks) {
        auto it = movies_features.find(p.first);
        if (it == movies_features.end()) {
            continue;
        }
        double offset = p.second - avg;
        const std::vector<double> &feat = it->second;
        for (size_t i = 0; i < dim; i++) {
            pref[i] += offset * feat[i];
        }
    }
    return pref;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const
{
    auto pref = get_preference_vector(user);
    if (pref.empty()) {
        return nullptr;
    }
    const auto &ranks = user.get_rank();

    double best_sim = -std::numeric_limits<double>::infinity();
    sp_movie best_movie = nullptr;

    for (auto &kv : movies_features) {
        // skip if user rated this
        if (ranks.find(kv.first) != ranks.end()) {
            continue;
        }
        double sim = cosine_similarity(pref, kv.second);
        // break ties by earlier year or lexicographically earlier name
        if (!best_movie || sim > best_sim ||
           (std::fabs(sim - best_sim) < 1e-9 && 
               (kv.first->get_year() < best_movie->get_year() ||
               (kv.first->get_year() == best_movie->get_year() &&
                kv.first->get_name() < best_movie->get_name()))))
        {
            best_sim = sim;
            best_movie = kv.first;
        }
    }
    return best_movie;
}

double RecommendationSystem::predict_movie_score(const User& user,
                                                 const sp_movie& movie, int k)
{
    const auto &ranks = user.get_rank();
    if (ranks.empty()) {
        return 0.0;
    }
    auto it = movies_features.find(movie);
    if (it == movies_features.end()) {
        return 0.0;
    }

    // gather similarities
    std::vector<std::pair<double,double>> sims;
    // will store (sim, userRating)

    for (auto &p : ranks) {
        auto it2 = movies_features.find(p.first);
        if (it2 == movies_features.end()) {
            continue;
        }
        double sim = cosine_similarity(it->second, it2->second);
        // If your snippet says to skip negative sim, you can do:
        // if (sim <= 0) continue;
        sims.push_back({sim, p.second});
    }

    // sort descending by sim
    std::sort(sims.begin(), sims.end(),
              [](auto &a, auto &b){ return a.first > b.first; });

    if ((int)sims.size() < k) {
        k = (int)sims.size();
    }
    if (k == 0) {
        return 0.0;
    }

    // Weighted average: sum(sim*rating)/ sum(sim)
    double numerator = 0.0, denominator = 0.0;
    for (int i = 0; i < k; i++) {
        double simVal   = sims[i].first;
        double rating_i = sims[i].second;
        numerator   += simVal * rating_i;
        denominator += simVal;
    }

    if (std::fabs(denominator) < 1e-9) {
        return 0.0;
    }

    // If you want to clamp final to [1..10], do so:
    double raw = numerator / denominator;
    if (raw < 1.0) raw = 1.0;
    if (raw > 10.0) raw = 10.0;

    return raw;
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k)
{
    const auto &ranks = user.get_rank();
    double best_score = -std::numeric_limits<double>::infinity();
    sp_movie best_mv  = nullptr;

    for (auto &kv : movies_features) {
        if (ranks.find(kv.first) != ranks.end()) {
            continue; // user rated => skip
        }
        double score = predict_movie_score(user, kv.first, k);
        if (!best_mv || score > best_score ||
            (std::fabs(score - best_score) < 1e-9 &&
             (kv.first->get_year() < best_mv->get_year() ||
              (kv.first->get_year() == best_mv->get_year() &&
               kv.first->get_name() < best_mv->get_name()))))
        {
            best_score = score;
            best_mv    = kv.first;
        }
    }
    return best_mv;
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs)
{
    // Print all movies in ascending order (year,name)
    for (auto &p : rs.movies_features) {
        os << *(p.first);
    }
    return os;
}
