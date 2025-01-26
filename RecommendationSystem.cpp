#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <limits>
#include <cmath>

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name,
                                             int year,
                                             const std::vector<double>& features) {
    // Validate features in [1,10]
    for (double f : features) {
        if (f < 1.0 || f > 10.0) {
            throw std::runtime_error("Feature value out of range");
        }
    }
    sp_movie mv = std::make_shared<Movie>(name, year);
    movies_features[mv] = features;
    return mv;
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const {
    for (const auto& p : movies_features) {
        if (p.first->get_name() == name && p.first->get_year() == year) {
            return p.first;
        }
    }
    return nullptr;
}

double RecommendationSystem::cosine_similarity(const std::vector<double>& v1,
                                             const std::vector<double>& v2) const {
    if (v1.size() != v2.size() || v1.empty()) return 0.0;

    double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
    for (size_t i = 0; i < v1.size(); i++) {
        dot += v1[i] * v2[i];
        norm1 += v1[i] * v1[i];
        norm2 += v2[i] * v2[i];
    }

    double norms = std::sqrt(norm1) * std::sqrt(norm2);
    return norms == 0.0 ? 0.0 : dot / norms;
}

std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const {
    const auto& ranks = user.get_rank();
    if (ranks.empty()) return {};

    // Calculate mean rating
    double m = 0.0;
    for (const auto& p : ranks) {
        m += p.second;
    }
    m /= ranks.size();

    // Get feature dimension
    if (movies_features.empty()) return {};
    size_t dim = movies_features.begin()->second.size();
    std::vector<double> p(dim, 0.0);

    // Calculate preference vector using normalized ratings
    for (const auto& r : ranks) {
        auto it = movies_features.find(r.first);
        if (it == movies_features.end()) continue;

        double s_bar = r.second - m;  // Normalize rating
        const auto& features = it->second;
        for (size_t i = 0; i < dim; i++) {
            p[i] += s_bar * features[i];
        }
    }
    return p;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const {
    std::vector<double> p = get_preference_vector(user);
    if (p.empty()) return nullptr;

    const auto& ranks = user.get_rank();
    double best_sim = -std::numeric_limits<double>::infinity();
    sp_movie best_movie = nullptr;

    for (const auto& kv : movies_features) {
        // Skip rated movies
        if (ranks.find(kv.first) != ranks.end()) continue;

        double sim = cosine_similarity(p, kv.second);
        // Updated tie-breaking logic
        if (sim > best_sim ||
            (sim == best_sim && best_movie &&
             (kv.first->get_year() < best_movie->get_year() ||
              (kv.first->get_year() == best_movie->get_year() &&
               kv.first->get_name() < best_movie->get_name())))) {
            best_sim = sim;
            best_movie = kv.first;
        }
    }
    return best_movie;
}

double RecommendationSystem::predict_movie_score(const User& user,
                                              const sp_movie& movie, int k) {
    const auto& ranks = user.get_rank();
    if (ranks.empty() || k <= 0) return 0.0;

    auto movie_it = movies_features.find(movie);
    if (movie_it == movies_features.end()) return 0.0;

    // Calculate mean rating
    double m = 0.0;
    for (const auto& p : ranks) {
        m += p.second;
    }
    m /= ranks.size();

    // Get k nearest neighbors
    std::vector<std::pair<double,double>> neighbors;
    neighbors.reserve(ranks.size());

    for (const auto& r : ranks) {
        auto it = movies_features.find(r.first);
        if (it == movies_features.end()) continue;

        double sim = cosine_similarity(movie_it->second, it->second);
        if (sim > 0.0) {  // Only consider positive similarities
            neighbors.push_back({sim, r.second});
        }
    }

    // Sort by similarity descending
    std::sort(neighbors.begin(), neighbors.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    k = std::min(k, (int)neighbors.size());
    if (k == 0) return m;

    // Calculate weighted average using normalized ratings
    double num = 0.0, den = 0.0;
    for (int i = 0; i < k; i++) {
        double normalized_rating = neighbors[i].second - m;
        num += neighbors[i].first * normalized_rating;
        den += neighbors[i].first;
    }

    return den == 0.0 ? m : m + (num / den);
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) {
    const auto& ranks = user.get_rank();
    double best_score = -std::numeric_limits<double>::infinity();
    sp_movie best_movie = nullptr;

    for (const auto& mv : movies_features) {
        if (ranks.find(mv.first) != ranks.end()) continue;

        double score = predict_movie_score(user, mv.first, k);
        if (score > best_score ||
            (score == best_score && best_movie &&
             (mv.first->get_year() < best_movie->get_year() ||
              (mv.first->get_year() == best_movie->get_year() &&
               mv.first->get_name() < best_movie->get_name())))) {
            best_score = score;
            best_movie = mv.first;
        }
    }
    return best_movie;
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs) {
    for (const auto& p : rs.movies_features) {
        os << *(p.first);
    }
    return os;
}