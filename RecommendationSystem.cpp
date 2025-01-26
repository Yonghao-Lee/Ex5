#include <valarray>
#include "RecommendationSystem.h"


sp_movie RecommendationSystem::recommend_by_content(const User& user){
    rank_map ranks = user.get_ranks();
    double average = 0;
    double num = 0;
    for(const auto& movie : ranks){
        average += movie.second;
        num++;
    }
    average /= num;
    for(auto& movie : ranks){
        movie.second -= average;
    }
    std::vector<double> pref(movies_.begin()->second.size(), 0.0);
    make_pref(ranks, pref);
    return find_rec(ranks, pref);
}


void RecommendationSystem::make_pref(const rank_map& ranks,
                                     std::vector<double>& pref_vec){
    // check that pref_vec is correct size and initialized to 0
    if (pref_vec.empty() && !movies_.empty()) {
        pref_vec.resize(movies_.begin()->second.size(), 0.0);
    }
    // make pref_vec
    for (const auto& pair : ranks) {
        const auto& movie = pair.first;
        const auto& normalized_rating = pair.second;
        auto it = movies_.find(movie);
        if (it != movies_.end()) {
            const auto& features = it->second;
            for (size_t i = 0; i < features.size(); ++i) {
                pref_vec[i] += normalized_rating * features[i];
            }
        }
    }
}

double RecommendationSystem::predict_movie_score(const User &user,
                                            const sp_movie &movie, int k){
    const rank_map& ranks = user.get_ranks();
    if (movies_.find(movie) == movies_.end()){
        return 0.0;
    }
    std::vector<std::pair<double, sp_movie>> similarities;
    for (const auto & user_rating : ranks) {
        const sp_movie& rated = user_rating.first;
        if (movies_.find(rated) == movies_.end()){
            continue;
        }
        double sim = check_similarity(rated, movie);
        similarities.emplace_back(sim, rated);
    }
    std::sort(similarities.begin(), similarities.end(),
              std::greater<>());
    double weighted_sum = 0.0, sum = 0.0;
    for(size_t i = 0; i < static_cast<size_t>(k) &&
                                    i < similarities.size(); ++i) {
        const std::pair<double, sp_movie>& item = similarities[i];
        double sim = item.first;
        const sp_movie& rated_movie = item.second;
        weighted_sum += sim * ranks.at(rated_movie);
        sum += sim;
    }
    return (sum > 0) ? (weighted_sum / sum) : 0.0;
}

sp_movie RecommendationSystem::find_rec(const rank_map& ranks,
                                        const std::vector<double>& pref_vec){
    sp_movie recommend = nullptr;
    double similar = -1.0;
    for(auto & feature : movies_) {
        const std::shared_ptr<Movie>& movie = feature.first;
        const std::vector<double>& features = feature.second;
        if (ranks.find(movie) != ranks.end()) {
            continue;
        }
        double dot_product = 0.0;
        double pref_norm = 0.0;
        double features_norm = 0.0;
        for (size_t i = 0; i < features.size(); ++i) {
            double feat = features[i];
            dot_product += pref_vec[i] * feat;
            pref_norm += pref_vec[i] * pref_vec[i];
            features_norm += feat * feat;
        }
        double similarity = (pref_norm > 0 && features_norm > 0) ?
                (dot_product / std::sqrt(pref_norm * features_norm)) : 0.0;
        if (similarity > similar) {
            similar = similarity;
            recommend = movie;
        }
    }
    return recommend;
}


sp_movie RecommendationSystem::recommend_by_cf(const User& user, int k) {
    const rank_map& ranks = user.get_ranks();
    sp_movie best_movie = nullptr;
    double best_rating = -1.0;
    for (const auto& feature : movies_) {
        const sp_movie& other_movie = feature.first;
        if (ranks.find(other_movie) != ranks.end()) {
            continue;
        }
        // checks similarity
        std::vector<std::pair<double, sp_movie>> similarities;
        for (const auto& rated_movie_pair : ranks) {
            const sp_movie& rated_movie = rated_movie_pair.first;
            if (movies_.find(rated_movie) == movies_.end() ||
                movies_.find(other_movie) == movies_.end()) {
                continue;
            }
            double similarity = check_similarity(rated_movie,
                                                 other_movie);
            similarities.emplace_back(similarity, rated_movie);
        }
        // sorts then selects k most similar movies
        std::sort(similarities.begin(), similarities.end(), std::greater<>());
        double weighted_sum = 0.0;
        double sum = 0.0;
        for(size_t i = 0; i < static_cast<size_t>(k) &&
                                    i < similarities.size(); ++i) {
            double similarity = similarities[i].first;
            sp_movie similar_movie = similarities[i].second;
            double user_rating = ranks.at(similar_movie);
            weighted_sum += similarity * user_rating;
            sum += similarity;
        }
        double predicted_rating = (sum > 0) ?
                (weighted_sum / sum) : 0.0;
        if (predicted_rating > best_rating) {
            best_rating = predicted_rating;
            best_movie = other_movie;
        }
    }
    return best_movie;
}

double RecommendationSystem::check_similarity(const sp_movie& movie1,
                                              const sp_movie& movie2) {
    const auto& features1 = movies_.at(movie1);
    const auto& features2 = movies_.at(movie2);
    double dot = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;
    for (size_t i = 0; i < features1.size(); ++i) {
        dot += features1[i] * features2[i];
        norm1 += features1[i] * features1[i];
        norm2 += features2[i] * features2[i];
    }
    double norm_product = std::sqrt(norm1) * std::sqrt(norm2);
    return (norm_product > 0) ? (dot / norm_product) : 0.0;
}

sp_movie RecommendationSystem::add_movie_to_rs(const std::string& name,
                                int year,const std::vector<double>& features){
    sp_movie new_movie = std::make_shared<Movie>(Movie(name, year));
    movies_[new_movie] = features;
    return new_movie;
}

sp_movie RecommendationSystem::get_movie(const std::string &name, int year)
const{
    for (const auto& entry : movies_) {
        const sp_movie& movie = entry.first;
        if (movie->get_name() == name && movie->get_year() == year) {
            return movie;
        }
    }
    return nullptr;
}

std::ostream& operator<<(std::ostream& s, const RecommendationSystem& rec){
    for (const auto& entry : rec.movies_) {
        s << *entry.first << std::endl;
    }
    return s;
}