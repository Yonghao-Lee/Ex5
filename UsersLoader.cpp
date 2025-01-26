#include "UsersLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<User> UsersLoader::create_users(
    const std::string& users_file_path,
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

    // Read header line
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty users file");
    }

    // The header looks like: "USER mov1-year mov2-year mov3-year ..."
    std::istringstream header(line);
    std::string skip; // will hold the first token, "USER" presumably
    header >> skip;

    std::vector<sp_movie> movies_in_header;
    std::string movie_info;

    // For each "mov-name-year" in the header, parse it
    while (header >> movie_info) {
        size_t pos = movie_info.rfind('-');
        if (pos == std::string::npos || pos == 0 || pos == movie_info.size()-1) {
            throw std::runtime_error("Invalid movie format in header: " + movie_info);
        }
        std::string name = movie_info.substr(0, pos);
        int year = std::stoi(movie_info.substr(pos + 1));

        // Make sure the movie is in the RS
        sp_movie mv = rs->get_movie(name, year);
        if (!mv) {
            throw std::runtime_error("Movie not found in RS: " + movie_info);
        }
        movies_in_header.push_back(mv);
    }

    if (movies_in_header.empty()) {
        throw std::runtime_error("No valid movies found in header");
    }

    // Now read each subsequent line for a user
    int line_number = 1;
    while (std::getline(file, line)) {
        line_number++;
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);

        std::string username;
        if (!(iss >> username)) {
            throw std::runtime_error("Invalid username format on line "
                                     + std::to_string(line_number));
        }

        rank_map user_ratings(0, sp_movie_hash, sp_movie_equal);
        // Now read rating or "NA" for each movie in movies_in_header
        for (size_t i = 0; i < movies_in_header.size(); i++) {
            std::string rating_str;
            if (!(iss >> rating_str)) {
                // If there's no token, treat it like "NA" or break
                break;
            }
            if (rating_str != "NA") {
                double rate = std::stod(rating_str);
                if (rate < 1.0 || rate > 10.0) {
                    throw std::runtime_error("Rating must be between 1 and 10");
                }
                user_ratings[movies_in_header[i]] = rate;
            }
        }

        // Only create a User if they have at least one valid rating
        if (!user_ratings.empty()) {
            users.emplace_back(username, user_ratings, rs);
        }
    }

    if (users.empty()) {
        throw std::runtime_error("No valid users found in file");
    }

    return users;
}