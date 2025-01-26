#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <limits>

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name,
                                               int year,
                                               const std::vector<double>& features)
{
    // Instead of throwing on invalid features, we just accept them
    sp_movie movie = std::make_shared<Movie>(name, year);
    movies_features[movie] = features;
    return movie;
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const {
    // find the sp_movie with matching (name, year)
    for (const auto &pair : movies_features) {
        if (pair.first->get_name() == name && pair.first->get_year() == year) {
            return pair.first;
        }
    }
    return nullptr; // not found
}

double RecommendationSystem::cosine_similarity(const std::vector<double>& v1,
                                               const std::vector<double>& v2) const
{
    if (v1.size() != v2.size() || v1.empty()) {
        return 0.0;
    }
    double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
    for (size_t i = 0; i < v1.size(); ++i) {
        dot += v1[i] * v2[i];
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
    const auto& ranks = user.get_rank();
    if (ranks.empty() || movies_features.empty()) {
        return {};
    }
    // user average rating
    double sum_r = 0.0;
    for (auto &p : ranks) {
        sum_r += p.second;
    }
    double avg_u = sum_r / ranks.size();

    // assume all movies have the same feature dimension
    size_t dim = movies_features.begin()->second.size();
    std::vector<double> preference(dim, 0.0);

    // preference += (rating(u,m) - avg_u) * features(m)
    for (auto &p : ranks) {
        // p.first is an sp_movie user rated, p.second is the rating
        auto it = movies_features.find(p.first);
        if (it == movies_features.end()) {
            // the user rated a movie not in RS → skip
            continue;
        }
        double offset = p.second - avg_u;
        const std::vector<double> &feat = it->second;
        for (size_t i = 0; i < dim; i++) {
            preference[i] += offset * feat[i];
        }
    }
    return preference;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const
{
    const auto& ranks = user.get_rank();
    std::vector<double> pref = get_preference_vector(user);

    double best_sim = -std::numeric_limits<double>::infinity();
    sp_movie best_movie = nullptr;

    for (const auto &[mv, feats] : movies_features) {
        // skip if user already rated
        if (ranks.find(mv) != ranks.end()) {
            continue;
        }
        double sim = cosine_similarity(pref, feats);
        if (sim > best_sim) {
            best_sim = sim;
            best_movie = mv;
        }
    }
    return best_movie;
}

double RecommendationSystem::predict_movie_score(const User& user,
                                                 const sp_movie& movie, int k)
{
    const auto& ranks = user.get_rank();
    if (ranks.empty()) {
        return 0.0;
    }
    // find the movie's features in RS
    auto it = movies_features.find(movie);
    if (it == movies_features.end()) {
        // movie not in RS => predict 0
        return 0.0;
    }
    // user avg
    double sum_r = 0.0;
    for (auto &p : ranks) {
        sum_r += p.second;
    }
    double user_avg = sum_r / ranks.size();

    // compute sim( movie, i ) for each i that user rated
    std::vector<std::pair<double, double>> sims;
    // store (sim, user_rating_of_i)
    sims.reserve(ranks.size());
    for (auto &p : ranks) {
        auto it2 = movies_features.find(p.first);
        if (it2 == movies_features.end()) {
            // skip if not in RS
            continue;
        }
        double sim = cosine_similarity(it->second, it2->second);
        sims.push_back({sim, p.second});
    }
    // sort descending by sim
    std::sort(sims.begin(), sims.end(),
              [](auto &a, auto &b){return a.first > b.first;});

    if (k > (int)sims.size()) {
        k = (int)sims.size();
    }

    double numerator = 0.0, denominator = 0.0;
    for (int i = 0; i < k; i++) {
        double sim = sims[i].first;
        double rating_i = sims[i].second;
        numerator += sim * (rating_i - user_avg);
        denominator += std::fabs(sim);
    }
    if (denominator == 0.0) {
        return user_avg;
    }
    return user_avg + (numerator / denominator);
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k)
{
    const auto& ranks = user.get_rank();
    double best_score = -std::numeric_limits<double>::infinity();
    sp_movie best_movie = nullptr;

    for (const auto &[mv, feats] : movies_features) {
        if (ranks.find(mv) != ranks.end()) {
            continue;
        }
        double score = predict_movie_score(user, mv, k);
        if (score > best_score) {
            best_score = score;
            best_movie = mv;
        }
    }
    return best_movie;
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs)
{
    for (const auto &[mv, feats] : rs.movies_features) {
        os << *mv;
    }
    return os;
}
