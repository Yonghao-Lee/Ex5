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

    // 1) Read the entire header line:
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty users file (no header line)");
    }
    std::istringstream header_line(line);

    // 2) Check the FIRST token:
    std::string maybe_user;
    if (!(header_line >> maybe_user)) {
        throw std::runtime_error("Invalid header line: no tokens");
    }

    std::vector<sp_movie> header_movies;
    std::string movie_info;

    // 3) If the first token is literally "USER", skip it:
    if (maybe_user == "USER")
    {
        // Then parse the rest of the header line as "MovieName-Year"
        while (header_line >> movie_info) {
            header_movies.push_back( parseMovieFromHeader(movie_info, rs) );
        }
    }
    else
    {
        // The first token is already a "MovieName-Year"
        header_movies.push_back( parseMovieFromHeader(maybe_user, rs) );

        // Now parse any remaining tokens on that line as well
        while (header_line >> movie_info) {
            header_movies.push_back( parseMovieFromHeader(movie_info, rs) );
        }
    }

    if (header_movies.empty()) {
        throw std::runtime_error("No movies found in header line");
    }

    // 4) Now read each subsequent line => parse username + numeric ratings
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;  // skip blank lines
        }

        std::istringstream iss(line);
        std::string username;
        if (!(iss >> username)) {
            // If we fail to read a username from this line, skip it
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

        // Always create a User object, even if they gave 0 numeric ratings
        users.emplace_back(username, ratings, rs);
    }

    if (users.empty()) {
        throw std::runtime_error("No valid users found after parsing all lines");
    }

    return users;
}

// A small helper function that parses "MovieName-Year" from the header
// and retrieves it from the RecommendationSystem.
sp_movie UsersLoader::parseMovieFromHeader(const std::string& movie_info,
                                           std::shared_ptr<RecommendationSystem> rs)
{
    // e.g. "BrokebackMountain-1966"
    size_t pos = movie_info.rfind('-');
    if (pos == std::string::npos || pos == 0 || pos == movie_info.size() - 1) {
        throw std::runtime_error("Invalid movie format in header: " + movie_info);
    }
    std::string name = movie_info.substr(0, pos);
    int year = std::stoi(movie_info.substr(pos + 1));

    sp_movie mov = rs->get_movie(name, year);
    if (!mov) {
        // The test .in_m file presumably has this movie. If not found => fail.
        throw std::runtime_error("Movie not found in RS: " + movie_info);
    }
    return mov;
}