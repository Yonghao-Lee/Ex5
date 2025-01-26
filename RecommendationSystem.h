/***************************************
 *  RecommendationSystem.h
 ***************************************/
#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H

#include "Movie.h"
#include <vector>
#include <map>
#include <memory>
#include <cmath>

class User;

/** Compare sp_movie by underlying (year,name). */
struct sp_movie_compare {
    bool operator()(const sp_movie &lhs, const sp_movie &rhs) const {
        if (!lhs || !rhs) {
            // fallback pointer comparison if either is null
            return lhs < rhs; 
        }
        return *lhs < *rhs; // calls Movie::operator<
    }
};

class RecommendationSystem {
private:
    /** map from sp_movie -> features, sorted by (year,name). */
    std::map<sp_movie, std::vector<double>, sp_movie_compare> movies_features;

    double cosine_similarity(const std::vector<double>& v1,
                             const std::vector<double>& v2) const;

    /** For content-based: build user’s preference vector. */
    std::vector<double> get_preference_vector(const User& user) const;

public:
    RecommendationSystem() = default;

    /** Add a new movie with given features.  May throw if features <1 or >10. */
    sp_movie add_movie_to_rs(const std::string& name, int year,
                             const std::vector<double>& features);

    /** Retrieve an sp_movie from this system if it exists, else nullptr. */
    sp_movie get_movie(const std::string& name, int year) const;

    /** Content-based recommendation, per the usual "preference vector" approach. */
    sp_movie recommend_by_content(const User& user) const;

    /**
     * CF rating prediction for a movie:
     *   "Raw Weighted Average" approach, per your Hebrew snippet:
     *      score_k(u) = sum_{i=1..k}( sim(m,i)* rating(u,i) ) / sum_{i=1..k}( sim(m,i) ).
     */
    double predict_movie_score(const User& user, const sp_movie& movie, int k);

    /** recommend_by_cf = pick the un-rated movie with highest predicted rating. */
    sp_movie recommend_by_cf(const User& user, int k);

    friend std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs);
};

#endif // RECOMMENDATIONSYSTEM_H
