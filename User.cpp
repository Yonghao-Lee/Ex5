// User.cpp
#include "User.h"

void User::add_movie_to_user(const std::string& name, int year,
                           const std::vector<double>& features,
                           double rate) {
    // Validate input parameters
    if (!rs) {
        throw std::runtime_error("Recommendation system not initialized");
    }
    if (rate < 1 || rate > 10) {
        throw std::invalid_argument("Rating must be between 1 and 10");
    }

    try {
        // Add movie to recommendation system and get shared pointer
        sp_movie movie = rs->add_movie_to_rs(name, year, features);
        if (!movie) {
            throw std::runtime_error("Failed to create/get movie");
        }

        // Update or add the rating
        ratings[movie] = rate;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to add movie: " + std::string(e.what()));
    }
}

sp_movie User::get_rs_recommendation_by_content() const {
    if (!rs) {
        return nullptr;
    }
    try {
        if (ratings.empty()) {
            return nullptr;
        }
        return rs->recommend_by_content(*this);
    } catch (const std::exception&) {
        return nullptr;
    }
}

sp_movie User::get_rs_recommendation_by_cf(int k) const {
    if (!rs || k <= 0) {
        return nullptr;
    }
    try {
        if (ratings.empty()) {
            return nullptr;
        }
        return rs->recommend_by_cf(*this, k);
    } catch (const std::exception&) {
        return nullptr;
    }
}

double User::get_rs_prediction_score_for_movie(const std::string& name,
                                             int year, int k) const {
    if (!rs || k <= 0) {
        return 0;
    }
    try {
        sp_movie movie = rs->get_movie(name, year);
        if (!movie) {
            return 0;
        }
        return rs->predict_movie_score(*this, movie, k);
    } catch (const std::exception&) {
        return 0;
    }
}

std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "name: " << user.username << std::endl;
    if (user.rs) {
        std::vector<sp_movie> sorted_movies;
        // Use the public getter instead
        for (const auto& [movie, _] : user.rs->get_movies()) {
            sorted_movies.push_back(movie);
        }
        std::sort(sorted_movies.begin(), sorted_movies.end(),
                 [](const sp_movie& a, const sp_movie& b) {
                     return *a < *b;
                 });

        for (const auto& movie : sorted_movies) {
            os << *movie;
        }
    }
    return os;
}