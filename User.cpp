#include "User.h"
#include "RecommendationSystem.h"

User::User(std::string name, rank_map& ranks, std::shared_ptr<RecommendationSystem> rec)
    : _name(std::move(name)), _ranks(ranks), _rec(std::move(rec)) {}

const std::string& User::get_name() const {
    return _name;
}

const rank_map& User::get_ranks() const {
    return _ranks;
}

void User::add_movie_to_user(const std::string& name, int year,
                            const std::vector<double>& features, double rate) {
    _ranks[_rec->add_movie_to_rs(name, year, features)] = rate;
}

sp_movie User::get_rs_recommendation_by_content() const {
    return _rec->recommend_by_content(*this);
}

sp_movie User::get_rs_recommendation_by_cf(int k) const {
    return _rec->recommend_by_cf(*this, k);
}

double User::get_rs_prediction_score_for_movie(const std::string& name,
                                             int year, int k) const {
    sp_movie movie = _rec->get_movie(name, year);
    return _rec->predict_movie_score(*this, movie, k);
}

std::ostream& operator<<(std::ostream& s, const User& user) {
    s << "name: " << user._name << std::endl;
    s << *(user._rec);
    return s;
}
