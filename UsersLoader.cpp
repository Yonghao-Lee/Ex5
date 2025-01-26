#include "UsersLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

std::vector<User> UsersLoader::create_users(const std::string& path,
    std::shared_ptr<RecommendationSystem> rs)
{
    if (!rs) {
        throw std::invalid_argument("RecommendationSystem cannot be null");
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open users file: " + path);
    }

    std::vector<User> users;
    std::string line;

    // read header
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty users file");
    }
    std::istringstream header(line);
    std::string skip; // usually "USER"
    header >> skip;

    std::vector<sp_movie> header_movies;
    std::string movie_info;

    while (header >> movie_info) {
        size_t pos = movie_info.rfind('-');
        if (pos == std::string::npos || pos == 0 || pos == movie_info.size() - 1) {
            throw std::runtime_error("Invalid movie format in header: " + movie_info);
        }
        std::string name = movie_info.substr(0, pos);
        int year = std::stoi(movie_info.substr(pos+1));

        sp_movie mv = rs->get_movie(name, year);
        if (!mv) {
            throw std::runtime_error("Movie not found in RS: " + movie_info);
        }
        header_movies.push_back(mv);
    }

    if (header_movies.empty()) {
        throw std::runtime_error("No movies found in header");
    }

    // read user lines
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string username;
        if (!(iss >> username)) {
            // skip line if no username
            continue;
        }
        rank_map ratings(0, sp_movie_hash, sp_movie_equal);

        size_t idx = 0;
        std::string rating_str;
        while (iss >> rating_str && idx < header_movies.size()) {
            if (rating_str != "NA") {
                double val = std::stod(rating_str);
                if (val < 1.0 || val > 10.0) {
                    throw std::runtime_error("Rating out of range: " + rating_str);
                }
                ratings[header_movies[idx]] = val;
            }
            idx++;
        }
        // Only create a User if they have at least one rating
        if (!ratings.empty()) {
            users.emplace_back(username, ratings, rs);
        }
    }

    if (users.empty()) {
        throw std::runtime_error("No valid users found in file");
    }
    return users;
}