#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "Movie.h"

// Forward-declare User to avoid circular includes
class User;

/**
 * RecommendationSystem manages a collection of movies with their feature vectors,
 * and provides methods for recommending movies by content-based or CF methods.
 */
class RecommendationSystem {
private:
    // Maps movies to their feature vectors
    std::map<sp_movie, std::vector<double>> movies_features;

    /**
     * Validates that feature values are within acceptable range (1-10)
     * @throws std::invalid_argument if features are invalid or size mismatch
     */
    void validate_feature_vector(const std::vector<double>& features) const;

    /**
     * Computes cosine similarity between two feature vectors
     * @throws std::invalid_argument if v1,v2 have different sizes
     */
    double cosine_similarity(const std::vector<double>& v1,
                            const std::vector<double>& v2) const;

    /**
     * Generates user preference vector from the user's rated movies
     * @throws std::runtime_error if user has no valid ratings in the system
     */
    std::vector<double> get_preference_vector(const User& user) const;

public:
    RecommendationSystem() = default;

    /**
     * Access all movies stored in this system
     */
    const std::map<sp_movie, std::vector<double>>& get_movies() const {
        return movies_features;
    }

    /**
     * Finds a movie by (name, year) or returns nullptr if not found
     */
    sp_movie get_movie(const std::string& name, int year) const;

    /**
     * Adds a new movie to the recommendation system or returns the existing one
     * if it already exists
     */
    sp_movie add_movie_to_rs(const std::string& name, int year,
                             const std::vector<double>& features);

    /**
     * Recommends a movie by content-based filtering
     */
    sp_movie recommend_by_content(const User& user) const;

    /**
     * Predicts a user rating for a given movie using CF
     * @throws std::invalid_argument if k <= 0 or movie is null
     */
    double predict_movie_score(const User& user, const sp_movie& movie, int k);

    /**
     * Recommends a movie by collaborative filtering
     */
    sp_movie recommend_by_cf(const User& user, int k);

    // Give RecommendationSystemLoader friend access (to allow direct access).
    friend class RecommendationSystemLoader;
};

/**
 * Prints all movies in ascending order (year then name),
 * each movie on its own line as "<name> (<year>)".
 */
std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs);

#endif