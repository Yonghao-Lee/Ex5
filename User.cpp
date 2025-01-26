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
    return rs->recommend_by_cf(*this, k);
}

double User::get_rs_prediction_score_for_movie(const std::string& name,
                                             int year, int k) const {
    sp_movie movie = rs->get_movie(name, year);
    if (movie == nullptr) {
        return 0;
    }
    return rs->predict_movie_score(*this, movie, k);
}

std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "name: " << user.username << std::endl;
    os << *user.rs;
    return os;
}
