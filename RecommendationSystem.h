#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H

#include <map>
#include "User.h"

struct compare_movies {
    bool operator()(const sp_movie& movie1, const sp_movie& movie2) const {
        return (*movie1) < (*movie2);
    }
};

class RecommendationSystem {
private:
    std::map<sp_movie, std::vector<double>, compare_movies> movies_;
    void make_pref(const rank_map& ranks, std::vector<double>& pref_vec);
    sp_movie find_rec(const rank_map& ranks, const std::vector<double>& pref_vec);
    double check_similarity(const sp_movie& movie1, const sp_movie& movie2);

public:
    sp_movie add_movie_to_rs(const std::string& name, int year, const std::vector<double>& features);
    sp_movie recommend_by_content(const User& user);
    sp_movie recommend_by_cf(const User& user, int k);
    double predict_movie_score(const User& user, const sp_movie& movie, int k);
    sp_movie get_movie(const std::string& name, int year) const;
    friend std::ostream& operator<<(std::ostream& s, const RecommendationSystem& rec);
};

#endif