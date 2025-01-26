/***************************************
*  User.cpp
 ***************************************/
#include "User.h"
#include <stdexcept>

void User::add_movie_to_user(const std::string &name, int year,
                             const std::vector<double> &features,
                             double rate)
{
    // If rating is out of range, we can throw (test #16 expects this):
    if (rate < 1.0 || rate > 10.0) {
        throw std::runtime_error("Rating out of range");
    }
    sp_movie mv = rs->add_movie_to_rs(name, year, features);
    ratings[mv] = rate; // Overwrites existing rating if any
}

sp_movie User::get_rs_recommendation_by_content() const
{
    return rs->recommend_by_content(*this);
}

sp_movie User::get_rs_recommendation_by_cf(int k) const
{
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

std::ostream& operator<<(std::ostream &os, const User &user)
{
    os << "name: " << user.username << "\n";
    // Print the entire RS (all movies) in sorted order:
    os << *user.rs;
    os << "\n"; // blank line after
    return os;
}
