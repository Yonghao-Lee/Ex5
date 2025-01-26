#ifndef USER_H
#define USER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>  // for std::shared_ptr
#include "Movie.h"

// Forward declare RecommendationSystem to avoid circular include
class RecommendationSystem;

typedef std::unordered_map<sp_movie, double,
                           hash_func, equal_func> rank_map;

/**
 * The User class manages the user's name, their rated movies,
 * and a pointer to a RecommendationSystem.
 */
class User {
private:
    std::string username;
    rank_map ratings;
    std::shared_ptr<RecommendationSystem> rs;

public:
    /**
     * Constructor for User class
     * @throws std::invalid_argument if username empty or rs == nullptr
     */
    User(const std::string& username,
         const rank_map& rankings,
         std::shared_ptr<RecommendationSystem> rs)
        : username(username), ratings(rankings), rs(rs)
    {
        if (username.empty()) {
            throw std::invalid_argument("Username cannot be empty");
        }
        if (!rs) {
            throw std::invalid_argument("RecommendationSystem cannot be null");
        }
    }

    std::string get_name() const { return username; }
    const rank_map& get_rank() const { return ratings; }

    /**
     * Add or update a rating for a movie in the system
     */
    void add_movie_to_user(const std::string& name, int year,
                           const std::vector<double>& features,
                           double rate);

    /**
     * Content-based recommendation
     */
    sp_movie get_rs_recommendation_by_content() const;

    /**
     * Collaborative filtering recommendation
     */
    sp_movie get_rs_recommendation_by_cf(int k) const;

    /**
     * Predicts the user's rating for a given (name, year)
     */
    double get_rs_prediction_score_for_movie(const std::string& name,
                                             int year, int k) const;

    /**
     * Prints:
     * name: <username>
     * <movieName1> (<year1>)
     * <movieName2> (<year2>)
     * ...
     * for all movies in the system, in ascending order (year then name).
     */
    friend std::ostream& operator<<(std::ostream& os, const User& user);
};

#endif