#include "RecommendationSystemLoader.h"
#include <fstream>
#include <sstream>

std::unique_ptr<RecommendationSystem>
RecommendationSystemLoader::create_rs_from_movies(const std::string& movies_file_path)
{
    std::ifstream file(movies_file_path);
    // if file can't open, just return an empty RS (no throw)
    if (!file.is_open()) {
        return std::make_unique<RecommendationSystem>();
    }

    auto rs = std::make_unique<RecommendationSystem>();
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) { continue; }
        std::istringstream iss(line);
        std::string movie_info;
        iss >> movie_info;
        if (movie_info.empty()) { continue; }

        // "Name-Year"
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

        std::vector<double> feats;
        double val;
        while (iss >> val) {
            // skip/clamp if out of range, no throw
            feats.push_back(val);
        }
        rs->add_movie_to_rs(name, year, feats);
    }
    return rs;
}
