#include "UsersLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

// Helper function to parse "MovieName-Year" and retrieve from RS
sp_movie UsersLoader::parseMovieFromHeader(const std::string& movie_info,
                                           std::shared_ptr<RecommendationSystem> rs)
{
    // e.g., "BrokebackMountain-1966"
    size_t pos = movie_info.rfind('-');
    if (pos == std::string::npos || pos == 0 || pos == movie_info.size() - 1) {
        throw std::runtime_error("Invalid movie format in header: " + movie_info);
    }
    std::string name = movie_info.substr(0, pos);
    int year = std::stoi(movie_info.substr(pos + 1));

    sp_movie mov = rs->get_movie(name, year);
    if (!mov) {
        // The test expects that the RS already has such a movie from its .in_m file
        throw std::runtime_error("Movie not found in RS: " + movie_info);
    }
    return mov;
}

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

    // -- 1) Read the full header line --
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty users file (no header line)");
    }
    std::istringstream header_line(line);

    // -- 2) Check the FIRST token --
    std::string first_token;
    if (!(header_line >> first_token)) {
        throw std::runtime_error("Invalid header format (no first token)");
    }

    std::vector<sp_movie> header_movies;
    std::string movie_info;

    if (first_token == "USER") {
        // Parse remaining tokens as "MovieName-Year"
        while (header_line >> movie_info) {
            sp_movie mov = parseMovieFromHeader(movie_info, rs);
            header_movies.push_back(mov);
        }
    }
    else {
        // The first token is already a "MovieName-Year"
        sp_movie mov = parseMovieFromHeader(first_token, rs);
        header_movies.push_back(mov);

        // Parse remaining tokens
        while (header_line >> movie_info) {
            mov = parseMovieFromHeader(movie_info, rs);
            header_movies.push_back(mov);
        }
    }

    if (header_movies.empty()) {
        throw std::runtime_error("No movies found in header line");
    }

    // -- 3) Parse each user line --
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string username;
        if (!(iss >> username)) {
            // If we can't read a username from this line, skip it
            continue;
        }

        rank_map ratings; // Default constructor uses sp_movie_hash and sp_movie_equal
        size_t movie_idx = 0;
        std::string rating_str;

        while (iss >> rating_str && movie_idx < header_movies.size()) {
            if (rating_str != "NA") {
                double val;
                try {
                    val = std::stod(rating_str);
                } catch (...) {
                    throw std::runtime_error("Invalid rating value: " + rating_str);
                }
                if (val < 1.0 || val > 10.0) {
                    throw std::runtime_error("Rating must be in [1..10], got: " + rating_str);
                }
                ratings[header_movies[movie_idx]] = val;
            }
            movie_idx++;
        }

        // Always create a User object, even if they gave 0 numeric ratings
        users.emplace_back(username, ratings, rs);
    }

    if (users.empty()) {
        throw std::runtime_error("No valid users found after header line");
    }

    return users;
}