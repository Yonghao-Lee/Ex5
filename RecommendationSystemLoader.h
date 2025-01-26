#ifndef RECOMMENDATIONSYSTEMLOADER_H
#define RECOMMENDATIONSYSTEMLOADER_H

#include "RecommendationSystem.h"
#include <memory>
#include <string>
#include <tuple>
#include <vector>

/**
 * Responsible for creating a RecommendationSystem from a file
 * that lists movies in the format "Name-Year feat1 feat2 ..."
 */
class RecommendationSystemLoader {
private:
    // Private constructor prevents instantiation
    RecommendationSystemLoader() = default;

    /**
     * Parses a movie line from input file
     */
    static std::tuple<std::string, int, std::vector<double>>
    parse_movie_line(const std::string& line, int line_number);

public:
    /**
     * Creates recommendation system from movie data file
     * @throws std::runtime_error if file cannot be read or has invalid format
     */
    static std::unique_ptr<RecommendationSystem> create_rs_from_movies(
        const std::string& movies_file_path);
};

#endif