#include "User.h"

void User::add_movie_to_user(const std::string &name, int year,
                             const std::vector<double> &features,
                             double rate)
{
    // if (rate < 1 || rate > 10) { /* skip or clamp, no throw */}
    sp_movie mv = rs->add_movie_to_rs(name, year, features);
    ratings[mv] = rate;
}

sp_movie User::get_rs_recommendation_by_content() const {
    return rs->recommend_by_content(*this);
}

sp_movie User::get_rs_recommendation_by_cf(int k) const {
    return rs->recommend_by_cf(*this, k);
}

double User::get_rs_prediction_score_for_movie(const std::string& name,
                                               int year, int k) const
{
    sp_movie mv = rs->get_movie(name, year);
    if (!mv) {
        return 0.0;
    }
    return rs->predict_movie_score(*this, mv, k);
}

/**
 * Print the user's name and then all movies in the RS (sorted by year,name).
 * Some tests also want an extra blank line after each user, so we add one.
 */
std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "name: " << user.username << "\n";
    os << *user.rs;
    os << "\n"; // extra newline
    return os;
}
