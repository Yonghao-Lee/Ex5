#include "UsersLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<User> UsersLoader::create_users(const std::string& users_file_path,
    std::shared_ptr<RecommendationSystem> rs) {
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

    // Parse header
    std::istringstream header(line);
    std::string skip;
    header >> skip;

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
            throw std::runtime_error("Movie not found: " + movie_info);
        }
        header_movies.push_back(movie);
    }

    if (header_movies.empty()) {
        throw std::runtime_error("No movies in header");
    }

    // Read users
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string username;
        if (!(iss >> username)) {
            continue;
        }

        rank_map ratings(0, sp_movie_hash, sp_movie_equal);
        std::string rating_str;
        size_t movie_idx = 0;

        while (iss >> rating_str && movie_idx < header_movies.size()) {
            if (rating_str != "NA") {
                double rating = std::stod(rating_str);
                if (rating < 1 || rating > 10) {
                    throw std::runtime_error("Invalid rating: " + rating_str);
                }
                ratings[header_movies[movie_idx]] = rating;
            }
            movie_idx++;
        }

        if (!ratings.empty()) {
            users.emplace_back(username, ratings, rs);
        }
    }

    if (users.empty()) {
        throw std::runtime_error("No valid users found");
    }

    return users;
}