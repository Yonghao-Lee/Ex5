#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cmath>
#include "Movie.h"
#include "User.h"

class User;

struct sp_movie_compare {
    bool operator()(const sp_movie &lhs, const sp_movie &rhs) const {
        if (!lhs || !rhs) return lhs < rhs;
        return *lhs < *rhs;
    }
};

class RecommendationSystem {
private:
    std::map<sp_movie, std::vector<double>, sp_movie_compare> movies_;

    void make_pref(const rank_map& ranks, std::vector<double>& pref_vec);
    sp_movie find_rec(const rank_map& ranks, const std::vector<double>& pref_vec);
    double check_similarity(const sp_movie& movie1, const sp_movie& movie2);

public:
    RecommendationSystem() = default;
    sp_movie add_movie_to_rs(const std::string& name, int year,
                            const std::vector<double>& features);
    sp_movie get_movie(const std::string& name, int year) const;
    sp_movie recommend_by_content(const User& user);
    double predict_movie_score(const User& user, const sp_movie& movie, int k);
    sp_movie recommend_by_cf(const User& user, int k);
    friend std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs);
};

#endif