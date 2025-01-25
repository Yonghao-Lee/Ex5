#include "UsersLoader.h"
#include <fstream>
#include <sstream>
std::vector<User> UsersLoader::create_users(
    const std::string& users_file_path,
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

    std::istringstream header(line);
    std::vector<sp_movie> movies;
    std::string movie_info;

    // Skip the first column (username)
    header >> movie_info;

    // Parse movie headers
    while (header >> movie_info) {
        size_t pos = movie_info.length() - 1;
        while (pos > 0 && std::isdigit(movie_info[pos])) pos--;

        if (pos == 0 || movie_info[pos] != '-') {
            throw std::runtime_error("Invalid movie format in header: " + movie_info);
        }

        std::string name = movie_info.substr(0, pos);
        int year = std::stoi(movie_info.substr(pos + 1));

        sp_movie movie = rs->get_movie(name, year);
        if (!movie) {
            throw std::runtime_error("Movie not found in RS: " + movie_info);
        }
        movies.push_back(movie);
    }

    if (movies.empty()) {
        throw std::runtime_error("No valid movies found in header");
    }

    // Read user ratings
    int line_number = 1;
    while (std::getline(file, line)) {
        line_number++;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string username;
        if (!(iss >> username)) {
            throw std::runtime_error("Invalid username at line " +
                                   std::to_string(line_number));
        }

        rank_map rankings(0, sp_movie_hash, sp_movie_equal);
        std::string rating;
        size_t movie_idx = 0;

        while (iss >> rating && movie_idx < movies.size()) {
            if (rating != "NA") {
                try {
                    double rate = std::stod(rating);
                    if (rate < 1 || rate > 10) {
                        throw std::runtime_error("Rating must be between 1 and 10");
                    }
                    rankings[movies[movie_idx]] = rate;
                } catch (const std::exception& e) {
                    throw std::runtime_error("Invalid rating format at line " +
                                           std::to_string(line_number));
                }
            }
            movie_idx++;
        }

        // Only create user if they have rated at least one movie
        if (!rankings.empty()) {
            users.emplace_back(username, rankings, rs);
        }
    }

    if (users.empty()) {
        throw std::runtime_error("No valid users found in file");
    }

    return users;
}