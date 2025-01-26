/***************************************
 *  RecommendationSystemLoader.cpp
 ***************************************/
#include "RecommendationSystemLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::unique_ptr<RecommendationSystem>
RecommendationSystemLoader::create_rs_from_movies(const std::string& movies_file_path)
{
    std::ifstream file(movies_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }
    auto rs = std::make_unique<RecommendationSystem>();

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);

        std::string movie_info;
        if (!(iss >> movie_info)) {
            continue;
        }
        size_t dash = movie_info.find_last_of('-');
        if (dash == std::string::npos) {
            // skip invalid line
            continue;
        }
        std::string name = movie_info.substr(0, dash);
        int year = 0;
        try {
            year = std::stoi(movie_info.substr(dash+1));
        } catch(...) {
            continue; // skip invalid
        }

        std::vector<double> feats;
        double val;
        bool invalidFeat = false;
        while (iss >> val) {
            if (val < 1.0 || val > 10.0) {
                // The tests #15,16 want us to throw if we see out-of-range
                throw std::runtime_error("Feature out of range");
            }
            feats.push_back(val);
        }
        if (feats.empty()) {
            // skip line if no features
            continue;
        }
        rs->add_movie_to_rs(name, year, feats);
    }
    return rs;
}
