#include "User.h"

void User::add_movie_to_user(const std::string &name, int year,
                           const std::vector<double> &features,
                           double rate) {
    sp_movie movie = rs->add_movie_to_rs(name, year, features);
    ratings[movie] = rate;
}

sp_movie User::get_rs_recommendation_by_content() const {
    return rs->recommend_by_content(*this);
}

sp_movie User::get_rs_recommendation_by_cf(int k) const {
    if (!rs) {
        throw std::runtime_error("Recommendation system is null");
    }
    try {
        return rs->recommend_by_cf(*this, k);
    } catch (const std::exception& e) {
        // If recommendation fails, return nullptr or rethrow based on requirements
        return nullptr;
    }
}

double User::get_rs_prediction_score_for_movie(const std::string& name,
                                             int year, int k) const {
    if (!rs) {
        throw std::runtime_error("Recommendation system is null");
    }

    sp_movie movie = rs->get_movie(name, year);
    if (!movie) {
        return 0;
    }

    try {
        return rs->predict_movie_score(*this, movie, k);
    } catch (const std::exception& e) {
        return 0;  // Return default score on error
    }
}