#include "UsersLoader.h"
#include <fstream>
#include <sstream>

std::vector<User> UsersLoader::create_users(
    const std::string& users_file_path,
    std::shared_ptr<RecommendationSystem> rs)
{
    std::ifstream file(users_file_path);
    // If file can't open, return empty vector, no throw
    if (!file.is_open()) {
        return {};
    }

    std::vector<User> users;
    std::string line;

    // First line: "User <movie1> <movie2> ..."
    if (!std::getline(file, line)) {
        return users; // no header => no users
    }
    std::istringstream header(line);
    std::string skip_word;
    header >> skip_word; // maybe "User"

    std::vector<sp_movie> movies;
    // read each "Name-Year" from header
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

    // Now each subsequent line is "username rating1 rating2 ..."
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
            if (!iss.fail() && !rating_str.empty()) {
                if (rating_str != "NA") {
                    try {
                        double val = std::stod(rating_str);
                        // if (val < 1 || val > 10) { /* clamp or skip */}
                        ranks[movies[idx]] = val;
                    } catch(...) {
                        // skip invalid
                    }
                }
            }
            ++idx;
        }

        // Only add user if they rated at least one movie
        if (!ranks.empty()) {
            users.emplace_back(username, ranks, rs);
        }
    }
    return users;
}
