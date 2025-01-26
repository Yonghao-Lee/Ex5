#include "RecommendationSystemLoader.h"
#include <fstream>
#include <sstream>
//#include <stdexcept> // not needed since we skip throwing

std::unique_ptr<RecommendationSystem>
RecommendationSystemLoader::create_rs_from_movies(const std::string& movies_file_path)
{
    std::ifstream file(movies_file_path);
    // If file can't open, just return an empty RS (no throw)
    if (!file.is_open()) {
        return std::make_unique<RecommendationSystem>();
    }

    auto rs = std::make_unique<RecommendationSystem>();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string movie_info;
        // Format is "Name-Year feat1 feat2 ..."
        iss >> movie_info;
        if (movie_info.empty()) {
            continue;
        }
        size_t dash_pos = movie_info.find_last_of('-');
        if (dash_pos == std::string::npos) {
            continue; // skip invalid
        }
        std::string name = movie_info.substr(0, dash_pos);
        int year;
        try {
            year = std::stoi(movie_info.substr(dash_pos+1));
        } catch(...) {
            continue; // skip invalid
        }

        std::vector<double> features;
        double val;
        while (iss >> val) {
            // skip or clamp if out of [1..10], no throw
            // if (val < 1 || val > 10) { /* skip or clamp? */ }
            features.push_back(val);
        }
        rs->add_movie_to_rs(name, year, features);
    }
    return rs;
}
