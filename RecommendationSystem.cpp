#include "RecommendationSystem.h"
#include "User.h"
#include <queue>
#include <stdexcept>

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name,
                                            int year,
                                            const std::vector<double>& features) {
    for (const auto& feature : features) {
        if (feature < 1.0 || feature > 10.0) {
            throw std::runtime_error("Feature values must be between 1 and 10");
        }
    }
    sp_movie new_movie = std::make_shared<Movie>(name, year);
    movies_[new_movie] = features;
    return new_movie;
}

sp_movie RecommendationSystem::get_movie(const std::string &name, int year) const {
    for (const auto& entry : movies_) {
        const sp_movie& movie = entry.first;
        if (movie->get_name() == name && movie->get_year() == year) {
            return movie;
        }
    }
    return nullptr;
}

void RecommendationSystem::make_pref(const rank_map& ranks,
                                   std::vector<double>& pref_vec) {
    if (pref_vec.empty() && !movies_.empty()) {
        pref_vec.resize(movies_.begin()->second.size(), 0.0);
    }

    for (const auto& pair : ranks) {
        const auto& movie = pair.first;
        const auto& normalized_rating = pair.second;
        auto it = movies_.find(movie);
        if (it != movies_.end()) {
            const auto& features = it->second;
            for (size_t i = 0; i < features.size(); ++i) {
                pref_vec[i] += normalized_rating * features[i];
            }
        }
    }
}

sp_movie RecommendationSystem::find_rec(const rank_map& ranks,
                                      const std::vector<double>& pref_vec) {
    sp_movie recommend = nullptr;
    double highest_similarity = -1.0;

    for (const auto& movie_pair : movies_) {
        const sp_movie& movie = movie_pair.first;
        const std::vector<double>& features = movie_pair.second;

        if (ranks.find(movie) != ranks.end()) {
            continue;
        }

        double similarity = check_similarity(movie, pref_vec);
        if (similarity > highest_similarity) {
            highest_similarity = similarity;
            recommend = movie;
        }
    }

    return recommend;
}

double RecommendationSystem::check_similarity(const sp_movie& movie1,
                                            const std::vector<double>& vec2) {
    const auto& features1 = movies_.at(movie1);
    const auto& features2 = vec2;

    double dot_product = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;

    for (size_t i = 0; i < features1.size(); ++i) {
        dot_product += features1[i] * features2[i];
        norm1 += features1[i] * features1[i];
        norm2 += features2[i] * features2[i];
    }

    double norm_product = std::sqrt(norm1) * std::sqrt(norm2);
    return (norm_product > 0) ? (dot_product / norm_product) : 0.0;
}

sp_movie RecommendationSystem::recommend_by_content(const User& user) {
    rank_map ranks = user.get_ranks();
    double average = 0;
    size_t count = 0;

    for (const auto& rating : ranks) {
        average += rating.second;
        count++;
    }

    if (count > 0) {
        average /= count;
    }

    // Normalize ratings
    for (auto& rating : ranks) {
        rating.second -= average;
    }

    std::vector<double> pref_vec;
    make_pref(ranks, pref_vec);
    return find_rec(ranks, pref_vec);
}

double RecommendationSystem::predict_movie_score(const User& user,
                                               const sp_movie& movie,
                                               int k) {
    const rank_map& ranks = user.get_ranks();

    // Use priority queue to maintain k most similar movies
    std::priority_queue<std::pair<double, sp_movie> > pq;

    for (const auto& rated_movie : ranks) {
        double similarity = check_similarity(rated_movie.first, movies_.at(movie));
        pq.push({similarity, rated_movie.first});
        if (pq.size() > static_cast<size_t>(k)) {
            pq.pop();
        }
    }

    double weighted_sum = 0.0;
    double weight_sum = 0.0;

    while (!pq.empty()) {
        auto [similarity, rated_movie] = pq.top();
        pq.pop();

        weighted_sum += similarity * ranks.at(rated_movie);
        weight_sum += similarity;
    }

    return weight_sum > 0 ? weighted_sum / weight_sum : 0.0;
}

sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) {
    sp_movie best_movie = nullptr;
    double highest_score = -1.0;

    for (const auto& movie_pair : movies_) {
        const sp_movie& movie = movie_pair.first;
        if (user.get_ranks().find(movie) != user.get_ranks().end()) {
            continue;
        }

        double score = predict_movie_score(user, movie, k);
        if (score > highest_score) {
            highest_score = score;
            best_movie = movie;
        }
    }

    return best_movie;
}

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs) {
    for (const auto& entry : rs.movies_) {
        os << *entry.first;
    }
    return os;
}