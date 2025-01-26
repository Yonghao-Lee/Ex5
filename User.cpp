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
        throw std::runtime_error("Recommendation system not initialized");
    }
    if (rate < 1.0 || rate > 10.0) {
        throw std::invalid_argument("Rating must be between 1 and 10");
    }

    // Add/Fetch movie from RS
    sp_movie movie = rs->add_movie_to_rs(name, year, features);
    if (!movie) {
        throw std::runtime_error("Failed to create/get movie");
    }
    // Update the user's rating
    ratings[movie] = rate;
}

sp_movie User::get_rs_recommendation_by_content() const {
    if (!rs) {
        return nullptr;
    }
    if (ratings.empty()) {
        return nullptr;
    }
    try {
        return rs->recommend_by_content(*this);
    } catch (...) {
        return nullptr;
    }
}

sp_movie User::get_rs_recommendation_by_cf(int k) const {
    if (!rs || k <= 0) {
        return nullptr;
    }
    if (ratings.empty()) {
        return nullptr;
    }
    try {
        return rs->recommend_by_cf(*this, k);
    } catch (...) {
        return nullptr;
    }
}

double User::get_rs_prediction_score_for_movie(const std::string& name,
                                               int year, int k) const
{
    if (!rs || k <= 0) {
        return 0.0;
    }
    try {
        sp_movie movie = rs->get_movie(name, year);
        if (!movie) {
            return 0.0;
        }
        return rs->predict_movie_score(*this, movie, k);
    } catch (...) {
        return 0.0;
    }
}

std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "name: " << user.get_name() << std::endl;
    // If the user has an RS, print all movies known to the RS
    if (user.rs) {
        std::vector<sp_movie> sorted_movies;
        sorted_movies.reserve(user.rs->get_movies().size());

        for (auto const & [movie, _] : user.rs->get_movies()) {
            sorted_movies.push_back(movie);
        }

        // Sort them
        std::sort(sorted_movies.begin(), sorted_movies.end(),
                  [](const sp_movie& a, const sp_movie& b){
                      return *a < *b;
                  });

        // Print them
        for (const auto & mv : sorted_movies) {
            os << mv->get_name() << " (" << mv->get_year() << ")" << std::endl;
        }
    }
    return os;
}