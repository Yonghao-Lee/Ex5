#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cmath>
#include "Movie.h"

class User;

struct sp_movie_compare {
    bool operator()(const sp_movie &lhs, const sp_movie &rhs) const {
        if (!lhs || !rhs) {
            return lhs < rhs;
        }
        return *lhs < *rhs;
    }
};

class RecommendationSystem {
private:
    // Key = sp_movie, Value = feature vector
    // Sorted by (year,name) because of sp_movie_compare
    std::map<sp_movie, std::vector<double>, sp_movie_compare> movies_features;

    double cosine_similarity(const std::vector<double>& v1,
                             const std::vector<double>& v2) const;

    std::vector<double> get_preference_vector(const User& user) const;

public:
    RecommendationSystem() = default;

    sp_movie add_movie_to_rs(const std::string& name, int year,
                             const std::vector<double>& features);

    sp_movie get_movie(const std::string& name, int year) const;

    sp_movie recommend_by_content(const User& user) const;

    double predict_movie_score(const User& user, const sp_movie& movie, int k);

    sp_movie recommend_by_cf(const User& user, int k);

    friend std::ostream& operator<<(std::ostream& os,
                                    const RecommendationSystem& rs);
};

#endif // RECOMMENDATIONSYSTEM_H
