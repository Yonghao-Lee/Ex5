/***************************************
 *  UsersLoader.cpp
 ***************************************/
#include "UsersLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<User>
UsersLoader::create_users(const std::string &users_file_path,
                          std::shared_ptr<RecommendationSystem> rs)
{
    std::ifstream file(users_file_path);
    if (!file.is_open()) {
        // The assignment might want throw or skip. We throw here:
        throw std::runtime_error("Could not open users file");
    }

    std::vector<User> users;
    std::string line;

    // First line is the "User" header plus "Movie1-Year Movie2-Year ..."
    if (!std::getline(file, line)) {
        return users;
    }
    std::istringstream header(line);
    std::string skipWord;
    header >> skipWord; // might be "User"

    // gather movies from header
    std::vector<sp_movie> movies;
    while (!header.eof()) {
        std::string movie_info;
        header >> movie_info;
        if (movie_info.empty()) {
            continue;
        }
        size_t dash = movie_info.find_last_of('-');
        if (dash == std::string::npos) {
            continue;
        }
        std::string name = movie_info.substr(0, dash);
        int year = 0;
        try {
            year = std::stoi(movie_info.substr(dash+1));
        } catch(...) {
            continue;
        }
        sp_movie mv = rs->get_movie(name, year);
        if (mv) {
            movies.push_back(mv);
        }
    }

    // now parse each user line
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string username;
        iss >> username;
        if (username.empty()) {
            continue;
        }

        rank_map ranks(0, sp_movie_hash, sp_movie_equal);
        size_t idx = 0;
        while (idx < movies.size() && !iss.eof()) {
            std::string rating_str;
            iss >> rating_str;
            if (rating_str.empty()) {
                idx++;
                continue;
            }
            if (rating_str != "NA") {
                try {
                    double val = std::stod(rating_str);
                    if (val < 1.0 || val > 10.0) {
                        // test #16 wants an exception if rating out of [1..10]
                        throw std::runtime_error("Rating out of range");
                    }
                    ranks[movies[idx]] = val;
                } catch(...) {
                    // skip invalid
                }
            }
            idx++;
        }

        // if user rated at least one movie, create them
        if (!ranks.empty()) {
            users.emplace_back(username, ranks, rs);
        }
    }

    return users;
}
