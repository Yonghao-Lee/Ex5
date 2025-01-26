#ifndef USER_H
#define USER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include "Movie.h"
#include "RecommendationSystem.h"

typedef std::unordered_map<sp_movie, double, hash_func, equal_func> rank_map;

class User {
private:
    std::string username;
    rank_map ratings;
    std::shared_ptr<RecommendationSystem> rs;

public:
    User(const std::string& username, const rank_map& ranks,
         std::shared_ptr<RecommendationSystem> rs)
         : username(username), ratings(ranks), rs(std::move(rs)) {}

    std::string get_name() const { return username; }
    const rank_map& get_ranks() const { return ratings; }

    void add_movie_to_user(const std::string& name, int year,
                          const std::vector<double>& features,
                          double rate);

    sp_movie get_rs_recommendation_by_content() const;
    sp_movie get_rs_recommendation_by_cf(int k) const;
    double get_rs_prediction_score_for_movie(const std::string& name,
                                           int year, int k) const;

    friend std::ostream& operator<<(std::ostream& os, const User& user);
};

#endif // USER_H