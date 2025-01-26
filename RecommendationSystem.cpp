#include "RecommendationSystem.h"
#include <cmath>
#include <algorithm>

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name, int year,
                                             const std::vector<double>& features) {
    for (const auto& feature : features) {
        if (feature < 1 || feature > 10) {
            throw std::invalid_argument("Feature values must be between 1 and 10");
        }
    }
    sp_movie new_movie = std::make_shared<Movie>(name, year);
    movies_[new_movie] = features;
    return new_movie;
}

sp_movie RecommendationSystem::get_movie(const std::string& name, int year) const {
    for (const auto& entry : movies_) {
        if (entry.first->get_name() == name && entry.first->get_year() == year) {
            return entry.first;
        }
    }
    return nullptr;
}

void RecommendationSystem::make_pref(const rank_map& ranks, std::vector<double>& pref_vec) {
    if (movies_.empty()) return;

    // Calculate average rating
    double avg = 0;
    for (const auto& rank : ranks) {
        avg += rank.second;
    }
    avg /= ranks.size();

    // Initialize preference vector
    pref_vec.assign(movies_.begin()->second.size(), 0);

    // Calculate normalized preferences
    for (const auto& rank : ranks) {
        const auto& movie = rank.first;
        double normalized_rating = rank.second - avg;
        const auto& features = movies_[movie];

        for (size_t i = 0; i < features.size(); i++) {
            pref_vec[i] += normalized_rating * features[i];
        }
    }
}

double RecommendationSystem::check_similarity(const sp_movie& movie1, const sp_movie& movie2) {
    const auto& features1 = movies_[movie1];
    const auto& features2 = movies_[movie2];

    double dot_product = 0;
    double norm1 = 0;
    double norm2 = 0;

    for (size_t i = 0; i < features1.size(); i++) {
        dot_product += features1[i] * features2[i];
        norm1 += features1[i] * features1[i];
        norm2 += features2[i] * features2[i];
    }

    double norm_product = std::sqrt(norm1) * std::sqrt(norm2);
    return norm_product > 0 ? dot_product / norm_product : 0;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) {
    const rank_map& ranks = user.get_ranks();
    std::vector<double> pref_vec;
    make_pref(ranks, pref_vec);
    return find_rec(ranks, pref_vec);
}

sp_movie RecommendationSystem::find_rec(const rank_map& ranks, const std::vector<double>& pref_vec) {
    sp_movie best_movie = nullptr;
    double max_similarity = -1;

    for (const auto& movie_entry : movies_) {
        const sp_movie& movie = movie_entry.first;
        if (ranks.find(movie) != ranks.end()) continue;

        const auto& features = movie_entry.second;
        double dot_product = 0;
        double pref_norm = 0;
        double feat_norm = 0;

        for (size_t i = 0; i < features.size(); i++) {
            dot_product += pref_vec[i] * features[i];
            pref_norm += pref_vec[i] * pref_vec[i];
            feat_norm += features[i] * features[i];
        }

        double similarity = 0;
        if (pref_norm > 0 && feat_norm > 0) {
            similarity = dot_product / std::sqrt(pref_norm * feat_norm);
        }

        if (similarity > max_similarity) {
            max_similarity = similarity;
            best_movie = movie;
        }
    }
    return best_movie;
}

double RecommendationSystem::predict_movie_score(const User& user, const sp_movie& movie, int k) {
    const rank_map& ranks = user.get_ranks();
    std::vector<std::pair<double, sp_movie>> similarities;

    for (const auto& rank : ranks) {
        double sim = check_similarity(rank.first, movie);
        similarities.emplace_back(sim, rank.first);
    }

    std::sort(similarities.begin(), similarities.end(), std::greater<>());
    double weighted_sum = 0;
    double weight_sum = 0;

    for (size_t i = 0; i < static_cast<size_t>(k) && i < similarities.size(); i++) {
        weighted_sum += similarities[i].first * ranks.at(similarities[i].second);
        weight_sum += similarities[i].first;
    }

    return weight_sum > 0 ? weighted_sum / weight_sum : 0;
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) {
    const rank_map& ranks = user.get_ranks();
    sp_movie best_movie = nullptr;
    double max_score = -1;

    for (const auto& movie_entry : movies_) {
        const sp_movie& movie = movie_entry.first;
        if (ranks.find(movie) != ranks.end()) continue;

        double score = predict_movie_score(user, movie, k);
        if (score > max_score) {
            max_score = score;
            best_movie = movie;
        }
    }
    return best_movie;
}

std::ostream& operator<<(std::ostream& s, const RecommendationSystem& rs) {
    for (const auto& entry : rs.movies_) {
        s << *(entry.first);
    }
    return s;
}