#include "UsersLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<User> UsersLoader::create_users(
    const std::string& users_file_path,
    std::shared_ptr<RecommendationSystem> rs)
{
    std::ifstream file(users_file_path);
    if (!file.is_open()) {
        // throw std::runtime_error("Could not open users file");
        // or skip:
        return {};
    }

    std::vector<User> users;
    std::string line;

    // First line is the header: "User <movie1> <movie2> ..."
    if (!std::getline(file, line)) {
        return users; // empty
    }
    std::istringstream header(line);

    // The first token might be "User" or something similar
    std::string dummy;
    header >> dummy;

    // Then the rest are "Name-Year" for each movie
    std::vector<sp_movie> movies;
    std::string movie_info;
    while (header >> movie_info) {
        size_t dash = movie_info.find_last_of('-');
        if (dash != std::string::npos) {
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
    }

    // Now read each user line
    while (std::getline(file, line)) {
        if (line.empty()) { continue; }
        std::istringstream iss(line);
        std::string username;
        iss >> username;
        if (username.empty()) { continue; }

        rank_map ranks(0, sp_movie_hash, sp_movie_equal);
        size_t idx = 0;
        while (idx < movies.size() && !iss.eof()) {
            std::string rating_str;
            iss >> rating_str;
            if (!iss.fail()) {
                if (rating_str != "NA") {
                    try {
                        double rt = std::stod(rating_str);
                        // if (rt < 1 || rt > 10)
                        // {
                        //     // throw std::runtime_error("Rating must be in [1..10]");
                        //     // or skip
                        // }
                        ranks[movies[idx]] = rt;
                    }
                    catch(...) {
                        // skip invalid
                    }
                }
            }
            ++idx;
        }

        if (!ranks.empty()) {
            users.emplace_back(username, ranks, rs);
        }
    }

    return users;
}
