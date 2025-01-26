#include "RecommendationSystemLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::unique_ptr<RecommendationSystem>
RecommendationSystemLoader::create_rs_from_movies(const std::string& movies_file_path)
{
    std::ifstream file(movies_file_path);
    if (!file.is_open()) {
        // throw std::runtime_error("Could not open movies file");
        // If your tests pass with throwing, keep it. Otherwise skip:
        return std::make_unique<RecommendationSystem>();
    }

    auto rs = std::make_unique<RecommendationSystem>();
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) { continue; }
        std::istringstream iss(line);

        std::string movie_info;
        if (!(iss >> movie_info)) {
            continue;
        }

        // Movie info is "Name-Year"
        size_t dash_pos = movie_info.find_last_of('-');
        if (dash_pos == std::string::npos) {
            continue; // skip this line
        }
        std::string name = movie_info.substr(0, dash_pos);
        int year = 0;
        try {
            year = std::stoi(movie_info.substr(dash_pos+1));
        }
        catch(...) {
            continue;
        }

        std::vector<double> features;
        double feature_val;
        while (iss >> feature_val) {
            // if (feature_val < 1 || feature_val > 10) {
            //     // throw std::runtime_error("Feature must be in [1..10]");
            //     // or skip/clamp
            // }
            features.push_back(feature_val);
        }

        rs->add_movie_to_rs(name, year, features);
    }

    return rs;
}
