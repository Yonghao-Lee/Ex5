#ifndef USER_H
#define USER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include "Movie.h"

class RecommendationSystem;
typedef std::unordered_map<sp_movie, double, hash_func, equal_func> rank_map;

class User {
private:
    std::string _name;
    rank_map _ranks;
    std::shared_ptr<RecommendationSystem> _rec;

public:
    User(std::string name, rank_map& ranks, std::shared_ptr<RecommendationSystem> rec);
    const std::string& get_name() const;
    const rank_map& get_rank() const;
    void add_movie_to_user(const std::string& name, int year,
                          const std::vector<double>& features, double rate);
    sp_movie get_rs_recommendation_by_content() const;
    sp_movie get_rs_recommendation_by_cf(int k) const;
    double get_rs_prediction_score_for_movie(const std::string& name,
                                           int year, int k) const;
    friend std::ostream& operator<<(std::ostream& s, const User& user);
};

#endif