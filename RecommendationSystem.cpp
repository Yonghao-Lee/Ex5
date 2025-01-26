#include "RecommendationSystem.h"
#include "User.h"
#include <algorithm>
#include <cmath>

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const {
    sp_movie temp = std::make_shared<Movie>(name, year);
    auto it = movies_features.find(temp);
    return it != movies_features.end() ? it->first : nullptr;
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

double RecommendationSystem::cosine_similarity(const std::vector<double>& v1,
                                             const std::vector<double>& v2) const {
    if (v1.size() != v2.size()) return -1.0;

    double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
    for (size_t i = 0; i < v1.size(); ++i) {
        dot += v1[i] * v2[i];
        norm1 += v1[i] * v1[i];
        norm2 += v2[i] * v2[i];
    }

    if (norm1 < 1e-10 || norm2 < 1e-10) return 0.0;
    return dot / (std::sqrt(norm1) * std::sqrt(norm2));
}

std::vector<double> RecommendationSystem::get_preference_vector(const User& user) const {
    const auto& rankings = user.get_rank();
    if (rankings.empty()) throw std::runtime_error("No ratings");

    double avg = 0.0;
    size_t count = 0;
    for (const auto& [movie, rating] : rankings) {
        if (movies_features.find(movie) != movies_features.end()) {
            avg += rating;
            ++count;
        }
    }
    if (count == 0) throw std::runtime_error("No valid movies");
    avg /= count;

    size_t feat_size = movies_features.begin()->second.size();
    std::vector<double> pref(feat_size, 0.0);

    for (const auto& [movie, rating] : rankings) {
        auto it = movies_features.find(movie);
        if (it == movies_features.end()) continue;

        double weight = rating - avg;
        const auto& feats = it->second;
        for (size_t i = 0; i < feat_size; ++i) {
            pref[i] += weight * feats[i];
        }
    }

    double norm = 0.0;
    for (double val : pref) norm += val * val;
    norm = std::sqrt(norm);

    if (norm > 1e-10) {
        for (double& val : pref) val /= norm;
    }

    return pref;
}

void RecommendationSystem::validate_feature_vector(const std::vector<double>& features) const {
    if (features.empty()) throw std::invalid_argument("Empty features");

    for (double f : features) {
        if (f < 1.0 || f > 10.0) {
            throw std::invalid_argument("Features must be between 1-10");
        }
    }

    if (!movies_features.empty() &&
        features.size() != movies_features.begin()->second.size()) {
        throw std::invalid_argument("Feature size mismatch");
    }
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) const {
    try {
        std::vector<double> pref = get_preference_vector(user);

        sp_movie best = nullptr;
        double best_sim = -1.0;
        const auto& rankings = user.get_rank();

        for (const auto& [movie, feats] : movies_features) {
            if (rankings.find(movie) != rankings.end()) continue;

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

double RecommendationSystem::predict_movie_score(const User& user,
                                               const sp_movie& movie, int k) const {
    if (!movie || k <= 0) throw std::invalid_argument("Invalid parameters");

    const auto& rankings = user.get_rank();
    if (rankings.empty()) throw std::runtime_error("No ratings");

    auto movie_it = movies_features.find(movie);
    if (movie_it == movies_features.end()) {
        throw std::runtime_error("Movie not found");
    }

    std::vector<std::pair<double, double>> sims;
    for (const auto& [rated_movie, rating] : rankings) {
        auto feat_it = movies_features.find(rated_movie);
        if (feat_it == movies_features.end()) continue;

        double sim = cosine_similarity(movie_it->second, feat_it->second);
        if (sim > -1.0) sims.emplace_back(sim, rating);
    }

    if (sims.empty()) throw std::runtime_error("No similarities");

    std::partial_sort(sims.begin(),
                     sims.begin() + std::min(k, (int)sims.size()),
                     sims.end(),
                     [](const auto& a, const auto& b) {
                         return a.first > b.first;
                     });

    double weighted_sum = 0.0, sum_weights = 0.0;
    for (int i = 0; i < std::min(k, (int)sims.size()); ++i) {
        weighted_sum += sims[i].first * sims[i].second;
        sum_weights += sims[i].first;
    }

    if (sum_weights < 1e-10) throw std::runtime_error("Invalid weights");
    return weighted_sum / sum_weights;
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) const {
    if (k <= 0) return nullptr;
    const auto& rankings = user.get_rank();
    if (rankings.empty()) return nullptr;

    try {
        sp_movie best = nullptr;
        double best_score = -std::numeric_limits<double>::infinity();

        for (const auto& [movie, _] : movies_features) {
            if (rankings.find(movie) != rankings.end()) continue;

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
std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs)
{
    // Example: print all movies in ascending order, each on its own line
    std::vector<sp_movie> sorted;
    sorted.reserve(rs.get_movies().size());

    for (auto const& [movie, features] : rs.get_movies())
    {
        sorted.push_back(movie);
    }

    std::sort(sorted.begin(), sorted.end(),
              [](const sp_movie& a, const sp_movie& b) {
                  return *a < *b;  // uses Movie::operator<
              });

    for (auto const& mv : sorted)
    {
        os << mv->get_name() << " (" << mv->get_year() << ")\n";
    }
    return os;
}