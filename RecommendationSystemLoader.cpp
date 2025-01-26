#include "RecommendationSystemLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::unique_ptr<RecommendationSystem>
RecommendationSystemLoader::create_rs_from_movies(const std::string& movies_file_path) {
    auto rs = std::make_unique<RecommendationSystem>();
    std::ifstream file(movies_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open movies file");
    }

    std::string line;
    while (std::getline(file, line)) {
        try {
            std::istringstream iss(line);
            std::string name_year;
            if (!(iss >> name_year)) continue;

            size_t dash = name_year.find_last_of('-');
            if (dash == std::string::npos) {
                throw std::runtime_error("Invalid movie name-year format");
            }

            std::string name = name_year.substr(0, dash);
            int year;
            try {
                year = std::stoi(name_year.substr(dash + 1));
            } catch (...) {
                throw std::runtime_error("Invalid year format");
            }

            std::vector<double> features;
            double val;
            while (iss >> val) {
                if (val < 1.0 || val > 10.0) {
                    throw std::runtime_error("Feature value out of range");
                }
                features.push_back(val);
            }

            if (features.empty()) {
                throw std::runtime_error("No features found for movie");
            }

            rs->add_movie_to_rs(name, year, features);
        } catch (const std::runtime_error& e) {
            // Propagate feature range errors
            if (std::string(e.what()) == "Feature value out of range") {
                throw;
            }
            // Skip other parsing errors
            continue;
        }
    }
    return rs;
}