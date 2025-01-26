#include "UsersLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

std::vector<User> UsersLoader::create_users(const std::string& users_file_path,
                                            std::shared_ptr<RecommendationSystem> rs)
{
    if (!rs) {
        throw std::invalid_argument("Recommendation system cannot be null");
    }

    std::ifstream file(users_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open users file: " + users_file_path);
    }

    std::vector<User> users;
    std::string line;

    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty users file (no header line)");
    }
    std::istringstream header_line(line);

    std::string maybe_user;
    if (!(header_line >> maybe_user)) {
        throw std::runtime_error("Invalid header line: no tokens");
    }

    std::vector<sp_movie> header_movies;
    std::string movie_info;

    if (maybe_user == "USER") {
        while (header_line >> movie_info) {
            header_movies.push_back(parseMovieFromHeader(movie_info, rs));
        }
    } else {
        header_movies.push_back(parseMovieFromHeader(maybe_user, rs));
        while (header_line >> movie_info) {
            header_movies.push_back(parseMovieFromHeader(movie_info, rs));
        }
    }

    if (header_movies.empty()) {
        throw std::runtime_error("No movies found in header line");
    }

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string username;
        if (!(iss >> username)) {
            continue;
        }

        rank_map ratings(0, sp_movie_hash, sp_movie_equal);
        size_t movie_idx = 0;
        std::string rating_str;

        while (iss >> rating_str && movie_idx < header_movies.size()) {
            if (rating_str != "NA") {
                double val = std::stod(rating_str);
                if (val < 1.0 || val > 10.0) {
                    throw std::runtime_error("Rating must be in [1..10], got: " + rating_str);
                }
                ratings[header_movies[movie_idx]] = val;
            }
            movie_idx++;
        }

        users.emplace_back(username, ratings, rs);
    }

    if (users.empty()) {
        throw std::runtime_error("No valid users found after parsing all lines");
    }

    return users;
}

sp_movie UsersLoader::parseMovieFromHeader(const std::string& movie_info,
                                           std::shared_ptr<RecommendationSystem> rs)
{
    size_t pos = movie_info.rfind('-');
    if (pos == std::string::npos || pos == 0 || pos == movie_info.size() - 1) {
        throw std::runtime_error("Invalid movie format in header: " + movie_info);
    }
    std::string name = movie_info.substr(0, pos);
    int year = std::stoi(movie_info.substr(pos + 1));

    sp_movie mov = rs->get_movie(name, year);
    if (!mov) {
        throw std::runtime_error("Movie not found in RS: " + movie_info);
    }
    return mov;
}