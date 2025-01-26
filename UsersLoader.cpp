#include "UsersLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

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

    // Read header
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty users file");
    }

    // Parse header (movie1-year, movie2-year, etc.)
    std::istringstream header(line);
    std::string skip;
    header >> skip; // e.g. "USER"

    std::vector<sp_movie> header_movies;
    std::string movie_info;
    while (header >> movie_info) {
        size_t pos = movie_info.rfind('-');
        if (pos == std::string::npos || pos == 0 || pos == movie_info.length()-1) {
            throw std::runtime_error("Invalid movie format: " + movie_info);
        }
        std::string name = movie_info.substr(0, pos);
        int year = std::stoi(movie_info.substr(pos + 1));

        sp_movie movie = rs->get_movie(name, year);
        if (!movie) {
            throw std::runtime_error("Movie not found in RS: " + movie_info);
        }
        header_movies.push_back(movie);
    }

    if (header_movies.empty()) {
        throw std::runtime_error("No movies in header");
    }

    // Read each user line
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string username;
        if (!(iss >> username)) {
            // skip if no username
            continue;
        }

        // build rating map
        rank_map ratings(0, sp_movie_hash, sp_movie_equal);
        std::string rating_str;
        size_t movie_idx = 0;

        while (iss >> rating_str && movie_idx < header_movies.size()) {
            if (rating_str != "NA") {
                double rate = std::stod(rating_str);
                if (rate < 1 || rate > 10) {
                    throw std::runtime_error("Rating must be between 1 and 10");
                }
                ratings[header_movies[movie_idx]] = rate;
            }
            movie_idx++;
        }

        // **Do NOT skip if ratings.empty()**; create the user anyway
        // old: if (!ratings.empty()) {
        //        users.emplace_back(username, ratings, rs);
        //      }
        // new:
        users.emplace_back(username, ratings, rs);
    }

    if (users.empty()) {
        throw std::runtime_error("No valid users found");
    }

    return users;
}