#ifndef USER_H
#define USER_H

#include <unordered_map>
#include <memory>
#include <string>
#include "Movie.h"

class RecommendationSystem;

typedef std::unordered_map<sp_movie,double,hash_func,equal_func> rank_map;

class User {
private:
    std::string username;
    rank_map ratings;
    std::shared_ptr<RecommendationSystem> rs;

public:
    User(const std::string& name,
         const rank_map& initial_ratings,
         std::shared_ptr<RecommendationSystem> system)
        : username(name), ratings(initial_ratings), rs(system)
    {
        if (username.empty()) {
            throw std::invalid_argument("Empty username");
        }
        if (!rs) {
            throw std::invalid_argument("Null RecommendationSystem");
        }
    }

    std::string get_name() const { return username; }
    const rank_map& get_rank() const { return ratings; }

    void add_movie_to_user(const std::string& name, int year,
                           const std::vector<double>& features, double rate);

    // Recommends by content-based, or throws if none found
    sp_movie get_rs_recommendation_by_content() const;
    // Recommends by CF, or throws if none found
    sp_movie get_rs_recommendation_by_cf(int k) const;

    // Predict user rating or returns 0 on error
    double get_rs_prediction_score_for_movie(const std::string& name,
                                             int year, int k) const;

    friend std::ostream& operator<<(std::ostream& os, const User& user);
};

#endif