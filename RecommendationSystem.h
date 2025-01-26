#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H

#include <map>
#include <vector>
#include <memory>
#include <stdexcept>
#include "Movie.h"

// Forward-declare to avoid circular include
class User;

/**
 * Manages a collection of movies + features and recommends based on user data.
 */
class RecommendationSystem {
private:
    // Movies mapped to their feature vectors
    std::map<sp_movie, std::vector<double>> movies_features;

    // Validate + compare
    void validate_feature_vector(const std::vector<double>& features) const;
    double cosine_similarity(const std::vector<double>& v1,
                             const std::vector<double>& v2) const;
    std::vector<double> get_preference_vector(const User& user) const;

public:
    RecommendationSystem() = default;

    // For debugging or other retrieval
    const std::map<sp_movie, std::vector<double>>& get_movies() const {
        return movies_features;
    }

    sp_movie get_movie(const std::string& name, int year) const;
    sp_movie add_movie_to_rs(const std::string& name, int year,
                             const std::vector<double>& features);

    /**
     * Returns best content-based recommendation or throws if none found
     * (e.g., user has no valid ratings).
     */
    sp_movie recommend_by_content(const User& user) const;

    /**
     * Predicts user rating for a movie using top-k CF neighbors or throws if no data.
     */
    double predict_movie_score(const User& user, const sp_movie& movie, int k) const;

    /**
     * Returns best CF recommendation or throws if none found
     * (or returns nullptr if you prefer, but that can cause segfault in tests).
     */
    sp_movie recommend_by_cf(const User& user, int k) const;

    friend class RecommendationSystemLoader;
};

/**
 * Allows:  std::cout << rs;
 * Prints all movies in ascending order, each on its own line.
 */
std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs);

#endif