#include "RecommendationSystemLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::unique_ptr<RecommendationSystem>
RecommendationSystemLoader::create_rs_from_movies(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open movies file: " + path);
    }

    auto rs = std::make_unique<RecommendationSystem>();
    std::string line;
    int line_number = 0;
    size_t expected_features = 0;

    while (std::getline(file, line)) {
        line_number++;
        if (line.empty()) {
            continue;
        }
        try {
            std::istringstream iss(line);
            std::string movie_info;
            if (!(iss >> movie_info)) {
                // no token => skip
                continue;
            }
            size_t pos = movie_info.rfind('-');
            if (pos == std::string::npos || pos == 0 ||
                pos == movie_info.size() - 1)
            {
                throw std::runtime_error("Invalid movie format");
            }
            std::string name = movie_info.substr(0, pos);
            int year = std::stoi(movie_info.substr(pos+1));

            std::vector<double> features;
            double feat;
            while (iss >> feat) {
                if (feat < 1.0 || feat > 10.0) {
                    throw std::runtime_error("Feature value out of range");
                }
                features.push_back(feat);
            }

            // check feature count consistency
            if (line_number == 1) {
                expected_features = features.size();
            } else {
                if (features.size() != expected_features) {
                    throw std::runtime_error("Inconsistent feature count");
                }
            }

            rs->add_movie_to_rs(name, year, features);

        } catch (std::exception &e) {
            throw std::runtime_error("Error at line " +
                                     std::to_string(line_number) + ": " + e.what());
        }
    }

    if (rs->get_movies().empty()) {
        throw std::runtime_error("No valid movies found in file");
    }

    return rs;
}