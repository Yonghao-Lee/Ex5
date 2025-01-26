#include "UsersLoader.h"
#include <fstream>
#include <sstream>

std::vector<User> UsersLoader::create_users(
    const std::string& users_file_path,
    std::shared_ptr<RecommendationSystem> rs)
{
    std::ifstream file(users_file_path);
    if (!file.is_open()) return {};

    std::vector<User> users;
    std::string line;

    if (!std::getline(file, line)) return users;

    std::istringstream header(line);
    std::string skip_word;
    header >> skip_word;

    std::vector<sp_movie> movies;
    std::string movie_info;
    while (header >> movie_info) {
        size_t dash = movie_info.find_last_of('-');
        if (dash == std::string::npos) continue;

        std::string name = movie_info.substr(0, dash);
        int year;
        try {
            year = std::stoi(movie_info.substr(dash+1));
        } catch(...) {
            continue;
        }
        sp_movie mv = rs->get_movie(name, year);
        if (mv) movies.push_back(mv);
    }

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string uname;
        iss >> uname;
        if (uname.empty()) continue;

        rank_map ranks(0, sp_movie_hash, sp_movie_equal);
        size_t idx = 0;

        while (idx < movies.size() && !iss.eof()) {
            std::string rating_str;
            iss >> rating_str;

            if (rating_str != "NA") {
                try {
                    double val = std::stod(rating_str);
                    if (val < 1.0 || val > 10.0) {
                        throw std::runtime_error("Rating out of range");
                    }
                    ranks[movies[idx]] = val;
                } catch(const std::runtime_error& e) {
                    throw;
                } catch(...) {
                    continue;
                }
            }
            idx++;
        }

        if (!ranks.empty()) {
            users.emplace_back(uname, ranks, rs);
        }
    }
    return users;
}