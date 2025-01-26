#include "User.h"

void User::add_movie_to_user(const std::string &name, int year,
                             const std::vector<double> &features,
                             double rate)
{
    // skip or clamp if out of [1..10], no throw
    // if (rate < 1 || rate > 10) { /* skip or clamp? */ }

    sp_movie mv = rs->add_movie_to_rs(name, year, features);
    ratings[mv] = rate; // Overwrite if existed
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
 * PRINTING THE USER: some tests want an extra blank line
 * after each user's block of movies. So we add one more newline.
 *
 * For example, the expected output for test #9 has:
 *   name: Amani
 *   <Movie lines>
 *
 *   name: Lauren
 *   <Movie lines>
 *
 * i.e. a blank line separating them.
 */
std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "name: " << user.username << "\n";
    os << *user.rs;         // prints all RS movies (sorted by year/name)
    os << "\n";             // add a blank line after printing
    return os;
}
