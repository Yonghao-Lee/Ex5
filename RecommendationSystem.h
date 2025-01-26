#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <stdexcept>
#include "Movie.h"

class User;

typedef std::size_t (*hash_func)(const sp_movie& movie);
typedef bool (*equal_func)(const sp_movie& m1, const sp_movie& m2);

/**
 * Manages a collection of movies + features and provides recommendations.
 */
class RecommendationSystem {
private:
    // Use unordered_map with custom hash and equality functions
    std::unordered_map<sp_movie, std::vector<double>, hash_func, equal_func> movies_features;

    void validate_feature_vector(const std::vector<double>& features) const;
    double cosine_similarity(const std::vector<double>& v1,
                             const std::vector<double>& v2) const;
    std::vector<double> get_preference_vector(const User& user) const;

public:
    RecommendationSystem() = default;

    const std::unordered_map<sp_movie, std::vector<double>, hash_func, equal_func>& get_movies() const {
        return movies_features;
    }

    sp_movie get_movie(const std::string& name, int year) const;
    sp_movie add_movie_to_rs(const std::string& name, int year,
                             const std::vector<double>& features);

    /**
     * Returns best content-based recommendation or throws if none found
     */
    sp_movie recommend_by_content(const User& user) const;

    /**
     * Predicts user rating for a movie using top-k CF neighbors or throws if no data.
     */
    double predict_movie_score(const User& user, const sp_movie& movie, int k) const;

    /**
     * Returns best CF recommendation or throws if none found
     */
    sp_movie recommend_by_cf(const User& user, int k) const;

    friend class RecommendationSystemLoader;
};

/**
 * Overload operator<< so we can do:  std::cout << *recommendationSystem;
 */
std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs);

#endif