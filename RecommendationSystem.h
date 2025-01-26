#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H

#include <map>
#include <vector>
#include <memory>
#include "Movie.h"

class User;

class RecommendationSystem {
private:
    std::map<sp_movie, std::vector<double>> movies_features;

    void validate_feature_vector(const std::vector<double>& features) const;
    double cosine_similarity(const std::vector<double>& v1,
                           const std::vector<double>& v2) const;
    std::vector<double> get_preference_vector(const User& user) const;

public:
    const std::map<sp_movie, std::vector<double>>& get_movies() const {
        return movies_features;
    }

    sp_movie get_movie(const std::string& name, int year) const;
    sp_movie add_movie_to_rs(const std::string& name, int year,
                            const std::vector<double>& features);
    sp_movie recommend_by_content(const User& user) const;
    double predict_movie_score(const User& user, const sp_movie& movie, int k) const;
    sp_movie recommend_by_cf(const User& user, int k) const;

    friend class RecommendationSystemLoader;
};
std::ostream& operator<<(std::ostream& os, const RecommendationSystem& rs);

#endif