#include "RecommendationSystemLoader.h"
#include <fstream>
#include <sstream>
std::unique_ptr<RecommendationSystem> RecommendationSystemLoader::create_rs_from_movies(
    const std::string& movies_file_path) {

    std::ifstream file(movies_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open movies file: " + movies_file_path);
    }

    auto rs = std::make_unique<RecommendationSystem>();
    std::string line;
    int line_number = 0;

    while (std::getline(file, line)) {
        line_number++;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string movie_info;
        std::vector<double> features;

        if (!std::getline(iss, movie_info, ' ')) {
            throw std::runtime_error("Invalid format at line " +
                                   std::to_string(line_number));
        }

        // Look for the last occurrence of "-" followed by digits
        size_t pos = movie_info.length() - 1;
        while (pos > 0 && std::isdigit(movie_info[pos])) pos--;
        if (pos == 0 || movie_info[pos] != '-') {
            throw std::runtime_error("Invalid movie format at line " +
                                   std::to_string(line_number));
        }

        std::string name = movie_info.substr(0, pos);
        int year;
        try {
            year = std::stoi(movie_info.substr(pos + 1));
        } catch (const std::exception& e) {
            throw std::runtime_error("Invalid year format at line " +
                                   std::to_string(line_number));
        }

        double feature;
        while (iss >> feature) {
            if (feature < 1 || feature > 10) {
                throw std::runtime_error("Feature value out of range at line " +
                                       std::to_string(line_number));
            }
            features.push_back(feature);
        }

        if (features.empty()) {
            throw std::runtime_error("No features found at line " +
                                   std::to_string(line_number));
        }

        try {
            rs->add_movie_to_rs(name, year, features);
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to add movie at line " +
                                   std::to_string(line_number) + ": " + e.what());
        }
    }

    if (rs->movies_features.empty()) {
        throw std::runtime_error("No valid movies found in file");
    }

    return rs;
}