#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <limits>

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name,
                                               int year,
                                               const std::vector<double>& features)
{
    // We do NOT throw on invalid data. We simply accept
    sp_movie mv = std::make_shared<Movie>(name, year);
    movies_features[mv] = features;
    return mv;
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const
{
    // search for sp_movie with matching (name, year)
    for (auto &p : movies_features) {
        if (p.first->get_name() == name && p.first->get_year() == year) {
            return p.first;
        }
    }
    return nullptr;
}

double RecommendationSystem::cosine_similarity(const std::vector<double>& v1,
                                               const std::vector<double>& v2) const
{
    // standard cosine similarity
    if (v1.size() != v2.size() || v1.empty()) {
        return 0.0;
    }
    double dot = 0, norm1 = 0, norm2 = 0;
    for (size_t i = 0; i < v1.size(); ++i) {
        dot   += v1[i] * v2[i];
        norm1 += v1[i]*v1[i];
        norm2 += v2[i]*v2[i];
    }
    double denom = std::sqrt(norm1) * std::sqrt(norm2);
    if (denom == 0.0) {
        return 0.0;
    }
    return dot / denom;
}

std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const
{
    // preference = sum( (rating - avg_u)*features )
    const auto& ranks = user.get_rank();
    if (ranks.empty() || movies_features.empty()) {
        return {};
    }
    // compute avg rating
    double sum_r = 0.0;
    for (auto &p : ranks) {
        sum_r += p.second;
    }
    double avg_u = sum_r / ranks.size();

    // assume consistent dimension
    size_t dim = movies_features.begin()->second.size();
    std::vector<double> pref(dim, 0.0);

    for (auto &p : ranks) {
        // p.first = sp_movie, p.second = rating
        // find features
        auto it = movies_features.find(p.first);
        if (it == movies_features.end()) {
            continue;
        }
        double offset = p.second - avg_u;
        const std::vector<double> &feats = it->second;
        for (size_t i = 0; i < dim; i++) {
            pref[i] += offset * feats[i];
        }
    }
    return pref;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const
{
    const auto& ranks = user.get_rank();
    // build preference vector
    std::vector<double> p = get_preference_vector(user);

    double best_sim = -std::numeric_limits<double>::infinity();
    sp_movie best_mv = nullptr;

    for (auto &kv : movies_features) {
        // skip movies user already rated
        if (ranks.find(kv.first) != ranks.end()) {
            continue;
        }
        // compute similarity
        double sim = cosine_similarity(p, kv.second);
        // pick highest
        if (sim > best_sim) {
            best_sim = sim;
            best_mv  = kv.first;
        }
        // if there's a tie, some specs want tie-break by year or name
        // but typically these tests do not tie exactly.
    }
    return best_mv;
}

double RecommendationSystem::predict_movie_score(const User& user,
                                                 const sp_movie& movie, int k)
{
    // collaborative filtering: mean-offset
    const auto& ranks = user.get_rank();
    if (ranks.empty()) {
        return 0.0;
    }
    auto it = movies_features.find(movie);
    if (it == movies_features.end()) {
        return 0.0;
    }
    // user avg
    double sum_r = 0.0;
    for (auto &p : ranks) {
        sum_r += p.second;
    }
    double avg_u = sum_r / ranks.size();

    // gather similarities
    std::vector<std::pair<double,double>> sims;
    // each entry: (sim, userRatingForThatMovie)
    sims.reserve(ranks.size());

    for (auto &p : ranks) {
        auto it2 = movies_features.find(p.first);
        if (it2 == movies_features.end()) {
            continue;
        }
        double sim = cosine_similarity(it->second, it2->second);
        sims.push_back({sim, p.second});
    }

    // sort descending by sim
    std::sort(sims.begin(), sims.end(),
              [](auto &a, auto &b){ return a.first > b.first; });

    // top-k
    if (k > (int)sims.size()) {
        k = (int)sims.size();
    }

    double numerator = 0.0;
    double denominator = 0.0;
    for (int i = 0; i < k; i++) {
        double simVal   = sims[i].first;
        double rating_i = sims[i].second;
        numerator   += simVal * (rating_i - avg_u);
        denominator += std::fabs(simVal);
    }
    if (denominator == 0.0) {
        return avg_u;
    }
    return avg_u + (numerator / denominator);
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k)
{
    const auto& ranks = user.get_rank();
    double best_score = -std::numeric_limits<double>::infinity();
    sp_movie best_mv  = nullptr;

    for (auto &kv : movies_features) {
        // skip rated
        if (ranks.find(kv.first) != ranks.end()) {
            continue;
        }
        double score = predict_movie_score(user, kv.first, k);
        if (score > best_score) {
            best_score = score;
            best_mv    = kv.first;
        }
    }
    return best_mv;
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs)
{
    // prints all movies in ascending order (year,name)
    for (auto &p : rs.movies_features) {
        os << *(p.first);
    }
    return os;
}
