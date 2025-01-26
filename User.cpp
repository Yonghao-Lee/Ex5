#include "User.h"

void User::add_movie_to_user(const std::string &name, int year,
                             const std::vector<double> &features,
                             double rate)
{
    // if (rate < 1 || rate > 10)
    // {
    //     throw std::runtime_error("Rating must be in [1..10]");
    //     // or skip/clamp
    // }
    sp_movie movie = rs->add_movie_to_rs(name, year, features);
    ratings[movie] = rate; // Overwrites if already existed
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
    if (!movie) {
        return 0.0;
    }
    return rs->predict_movie_score(*this, movie, k);
}

std::ostream& operator<<(std::ostream& os, const User& user) {
    // The tests want:
    // name: <username>
    // then *all* movies in the RS (sorted by year, name)
    // because operator<<(std::ostream&, RecommendationSystem&) prints them
    os << "name: " << user.username << "\n";
    os << *user.rs;
    return os;
}
