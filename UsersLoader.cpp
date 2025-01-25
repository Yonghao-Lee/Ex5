#include "UsersLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<User> UsersLoader::create_users(
    const std::string& users_file_path,
    std::shared_ptr<RecommendationSystem> rs) {
    
    std::ifstream file(users_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open users file");
    }

    std::vector<User> users;
    std::string line;
    
    std::getline(file, line);
    std::istringstream header(line);
    std::vector<sp_movie> movies;
    std::string movie_info;
    
    header >> movie_info;
    
    while (header >> movie_info) {
        size_t year_start = movie_info.find_last_of('-');
        if (year_start != std::string::npos) {
            std::string name = movie_info.substr(0, year_start);
            int year = std::stoi(movie_info.substr(year_start + 1));
            sp_movie movie = rs->get_movie(name, year);
            if (movie != nullptr) {

                movies.push_back(movie);
            }
        }
    }

    // Read user ratings
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username;
        iss >> username;

        rank_map rankings(0, sp_movie_hash, sp_movie_equal);
        std::string rating;
        size_t movie_idx = 0;

        while (iss >> rating) {
            if (rating != "NA") {
                double rate = std::stod(rating);
                if (rate < 1 || rate > 10) {
                    throw std::runtime_error("Rating must be between 1 and 10");
                }
                if (movie_idx < movies.size()) {
                    rankings[movies[movie_idx]] = rate;
                }
            }
            movie_idx++;
        }

        // Only create user if they have rated at least one movie
        if (!rankings.empty()) {
            users.emplace_back(username, rankings, rs);
        }
    }

    return users;
}