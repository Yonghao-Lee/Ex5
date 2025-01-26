// RecommendationSystem.h
#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "Movie.h"

class User;

class RecommendationSystem {
private:
    // Maps movies to their feature vectors
    std::map<sp_movie, std::vector<double>> movies_features;

    /**
     * Validates that feature values are within acceptable range (1-10)
     * @param features Vector of feature values to validate
     * @throws std::invalid_argument if features are invalid
     */
    void validate_feature_vector(const std::vector<double>& features) const;

    /**
     * Computes cosine similarity between two feature vectors
     * @param v1 First feature vector
     * @param v2 Second feature vector
     * @return Similarity score between -1 and 1
     * @throws std::invalid_argument if vectors have different sizes
     */
    double cosine_similarity(const std::vector<double>& v1,
                           const std::vector<double>& v2) const;

    /**
     * Generates user preference vector based on their ratings
     * @param user User whose preferences to analyze
     * @return Vector representing user's preferences
     * @throws std::runtime_error if user has no valid ratings
     */
    std::vector<double> get_preference_vector(const User& user) const;

public:
    RecommendationSystem() = default;

    const std::map<sp_movie, std::vector<double>>& get_movies() const {
    return movies_features;
}

    sp_movie get_movie(const std::string& name, int year) const;

    /**
     * Adds a new movie to the recommendation system
     * @param name Movie name
     * @param year Release year
     * @param features Vector of movie features
     * @return Shared pointer to the created/existing movie
     * @throws std::invalid_argument if features are invalid
     */
    sp_movie add_movie_to_rs(const std::string& name, int year,
                            const std::vector<double>& features);

    sp_movie recommend_by_content(const User& user) const;

    double predict_movie_score(const User& user, const sp_movie& movie, int k);

    sp_movie recommend_by_cf(const User& user, int k);

}
    friend class RecommendationSystemLoader;
};

std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs);

#endif