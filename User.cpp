#include "User.h"
#include "RecommendationSystem.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>

void User::add_movie_to_user(const std::string& name,
                             int year,
                             const std::vector<double>& features,
                             double rate)
{
    if (!rs) {
        throw std::runtime_error("No RecommendationSystem attached to user");
    }
    if (rate < 1.0 || rate > 10.0) {
        throw std::invalid_argument("Rating must be in [1..10]");
    }
    sp_movie mv = rs->add_movie_to_rs(name, year, features);
    if (!mv) {
        throw std::runtime_error("Failed to create/find movie in RS");
    }
    ratings[mv] = rate;
}

sp_movie User::get_rs_recommendation_by_content() const {
    if (!rs) {
        throw std::runtime_error("No RecommendationSystem");
    }
    return rs->recommend_by_content(*this);
}

sp_movie User::get_rs_recommendation_by_cf(int k) const {
    if (!rs) {
        throw std::runtime_error("No RecommendationSystem");
    }
    return rs->recommend_by_cf(*this, k);
}

double User::get_rs_prediction_score_for_movie(const std::string& name,
                                               int year,
                                               int k) const
{
    if (!rs || k <= 0) {
        return 0;
    }
    try {
        sp_movie mv = rs->get_movie(name, year);
        if (!mv) {
            return 0;
        }
        return rs->predict_movie_score(*this, mv, k);
    } catch (...) {
        return 0;
    }
}

std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "name: " << user.get_name() << "\n";
    if (user.rs) {
        const auto& movie_map = user.rs->get_movies();
        std::vector<sp_movie> sorted;
        sorted.reserve(movie_map.size());
        for (auto const& [mv, _] : movie_map) {
            sorted.push_back(mv);
        }
        std::sort(sorted.begin(), sorted.end(),
                  [](const sp_movie& a, const sp_movie& b) {
                      return *a < *b;
                  });
        for (auto const& m : sorted) {
            os << m->get_name() << " (" << m->get_year() << ")\n";
        }
    }
    return os;
}