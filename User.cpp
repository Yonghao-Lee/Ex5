#include "User.h"
#include <algorithm>

void User::add_movie_to_user(const std::string& name, int year,
                            const std::vector<double>& features,
                            double rate) {
    if (!rs) {
        throw std::runtime_error("Recommendation system not initialized");
    }
    if (rate < 1 || rate > 10) {
        throw std::invalid_argument("Rating must be between 1 and 10");
    }

    sp_movie movie = rs->add_movie_to_rs(name, year, features);
    if (!movie) {
        throw std::runtime_error("Failed to create/get movie");
    }
    ratings[movie] = rate;
}

sp_movie User::get_rs_recommendation_by_content() const {
    if (!rs || ratings.empty()) {
        return nullptr;
    }
    try {
        return rs->recommend_by_content(*this);
    } catch (...) {
        return nullptr;
    }
}

sp_movie User::get_rs_recommendation_by_cf(int k) const {
    if (!rs || k <= 0 || ratings.empty()) return nullptr;
    try {
        return rs->recommend_by_cf(*this, k);
    } catch (...) {
        return nullptr;
    }
}

double User::get_rs_prediction_score_for_movie(const std::string& name,
                                             int year, int k) const {
    if (!rs || k <= 0) return 0;
    try {
        sp_movie movie = rs->get_movie(name, year);
        if (!movie) return 0;
        return rs->predict_movie_score(*this, movie, k);
    } catch (...) {
        return 0;
    }
}

std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "name: " << user.get_name() << "\n";
    if (user.rs) {
        std::vector<sp_movie> sorted;
        for (const auto& [movie, _] : user.rs->get_movies()) {
            sorted.push_back(movie);
        }
        std::sort(sorted.begin(), sorted.end(),
                  [](const sp_movie& a, const sp_movie& b) {
                      return *a < *b;
                  });

        for (const auto& movie : sorted) {
            os << movie->get_name() << " (" << movie->get_year() << ")\n";
        }
    }
    return os;
}