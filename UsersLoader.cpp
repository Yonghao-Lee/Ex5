// UsersLoader.cpp
#include "UsersLoader.h"
#include <fstream>
#include <sstream>

std::vector<User> UsersLoader::create_users(
    const std::string& users_file_path,
    std::shared_ptr<RecommendationSystem> rs) {

    // Validate input
    if (!rs) {
        throw std::invalid_argument("Recommendation system cannot be null");
    }

    // Open and validate file
    std::ifstream file(users_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open users file: " + users_file_path);
    }

    std::vector<User> users;
    users.reserve(10);
    std::string line;

    // Read and validate header
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty users file");
    }

    // Parse header to get movie information
    std::istringstream header(line);
    std::vector<sp_movie> movies;
    std::string movie_info;

    // Skip username column header
    header >> movie_info;

    // Parse movie headers and validate movies exist in RS
    while (header >> movie_info) {
        try {
            size_t pos = movie_info.rfind('-');
            if (pos == std::string::npos || pos == 0 || pos == movie_info.length() - 1) {
                throw std::runtime_error("Invalid movie format in header: " + movie_info);
            }

            std::string name = movie_info.substr(0, pos);
            int year = std::stoi(movie_info.substr(pos + 1));

            sp_movie movie = rs->get_movie(name, year);
            if (!movie) {
                throw std::runtime_error("Movie not found in RS: " + movie_info);
            }
            movies.push_back(movie);
        } catch (const std::exception& e) {
            throw std::runtime_error("Header parsing error: " + std::string(e.what()));
        }
    }

    if (movies.empty()) {
        throw std::runtime_error("No valid movies found in header");
    }

    // Read user data
    int line_number = 1;
    while (std::getline(file, line)) {
        line_number++;
        if (line.empty()) continue;

        try {
            std::istringstream iss(line);
            std::string username;
            if (!(iss >> username)) {
                throw std::runtime_error("Invalid username format");
            }

            rank_map rankings(0, sp_movie_hash, sp_movie_equal);
            std::string rating;
            size_t movie_idx = 0;

            while (iss >> rating && movie_idx < movies.size()) {
                if (rating != "NA") {
                    double rate = std::stod(rating);
                    if (rate < 1 || rate > 10) {
                        throw std::runtime_error("Rating must be between 1 and 10");
                    }
                    rankings[movies[movie_idx]] = rate;
                }
                movie_idx++;
            }

            // Only create user if they have valid ratings
            if (!rankings.empty()) {
                users.emplace_back(username, rankings, rs);
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Error at line " + std::to_string(line_number) +
                                   ": " + std::string(e.what()));
        }
    }

    if (users.empty()) {
        throw std::runtime_error("No valid users found in file");
    }

    return users;
}