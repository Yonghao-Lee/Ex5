#include "RecommendationSystemLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::unique_ptr<RecommendationSystem> RecommendationSystemLoader::create_rs_from_movies(
    const std::string& movies_file_path) {
    
    std::ifstream file(movies_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open movies file");
    }

    auto rs = std::make_unique<RecommendationSystem>();
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string movie_info;
        std::vector<double> features;
        
        if (!std::getline(iss, movie_info, ' ')) {
            continue;
        }
        
        size_t year_start = movie_info.find_last_of('-');
        if (year_start == std::string::npos) {
            continue;
        }
        
        std::string name = movie_info.substr(0, year_start);
        int year = std::stoi(movie_info.substr(year_start + 1));

        double feature;
        while (iss >> feature) {
            if (feature < 1 || feature > 10) {
                throw std::runtime_error("Feature values must be between 1 and 10");
            }
            features.push_back(feature);
        }

        rs->add_movie_to_rs(name, year, features);
    }

    return rs;
}