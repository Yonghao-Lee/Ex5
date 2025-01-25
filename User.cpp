#include "User.h"

void User::add_movie_to_user(const std::string &name, int year,
                           const std::vector<double> &features,
                           double rate) {
    if (!rs) {
        throw std::runtime_error("Recommendation system not initialized");
    }
    try {
        sp_movie movie = rs->add_movie_to_rs(name, year, features);
        if (movie) {
            ratings[movie] = rate;
        }
    } catch (const std::exception& e) {
        // Depending on requirements, either rethrow or handle silently
        throw std::runtime_error("Failed to add movie: " + std::string(e.what()));
    }
}
sp_movie User::get_rs_recommendation_by_content() const {
    if (!rs) {
        return nullptr;  // Safety check for null recommendation system
    }
    try {
        return rs->recommend_by_content(*this);
    } catch (const std::exception&) {
        return nullptr;  // Return nullptr instead of propagating the exception
    }
}

sp_movie User::get_rs_recommendation_by_cf(int k) const {
    if (!rs) {
        return nullptr;
    }
    try {
        return rs->recommend_by_cf(*this, k);
    } catch (const std::exception&) {
        return nullptr;
    }
}

double User::get_rs_prediction_score_for_movie(const std::string& name,
                                             int year, int k) const {
    if (!rs) {
        return 0;  // Handle null recommendation system
    }
    try {
        sp_movie movie = rs->get_movie(name, year);
        if (!movie) {
            return 0;
        }
        return rs->predict_movie_score(*this, movie, k);
    } catch (const std::exception&) {
        return 0;  // Handle any prediction errors gracefully
    }
}

std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "name: " << user.username << std::endl;
    if (user.rs) {  // Add null check before dereferencing
        os << *user.rs;
    }
    return os;
}
