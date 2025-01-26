#ifndef USER_H
#define USER_H

#include <unordered_map>
#include <string>
#include <memory>   // <-- IMPORTANT: so we can use std::shared_ptr
#include "Movie.h"  // for sp_movie, hash_func, etc.

// 1) Forward-declare the RecommendationSystem class
class RecommendationSystem;

typedef std::unordered_map<sp_movie, double, hash_func, equal_func> rank_map;

class User {
private:
    std::string username;
    rank_map ratings;

    // 2) Now the compiler knows "RecommendationSystem is a class"
    std::shared_ptr<RecommendationSystem> rs;

public:
    /**
     * This constructor must match the type of the third argument:
     * 'std::shared_ptr<RecommendationSystem>' not 'int'
     */
    User(const std::string& username,
         const rank_map& rankings,
         std::shared_ptr<RecommendationSystem> rs)
        : username(username), ratings(rankings), rs(rs)
    {
        if (username.empty()) {
            throw std::invalid_argument("Empty username");
        }
        if (!rs) {
            throw std::invalid_argument("Null recommendation system");
        }
    }

    std::string get_name() const { return username; }
    const rank_map& get_rank() const { return ratings; }

    void add_movie_to_user(const std::string& name, int year,
                          const std::vector<double>& features, double rate);
    sp_movie get_rs_recommendation_by_content() const;
    sp_movie get_rs_recommendation_by_cf(int k) const;
    double get_rs_prediction_score_for_movie(const std::string& name,
                                           int year, int k) const;

    friend std::ostream& operator<<(std::ostream& os, const User& user);
};

#endif