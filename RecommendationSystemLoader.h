#ifndef RECOMMENDATIONSYSTEMLOADER_H
#define RECOMMENDATIONSYSTEMLOADER_H

#include "RecommendationSystem.h"
#include <memory>
#include <string>
#include <tuple>
#include <vector>

class RecommendationSystemLoader {
private:
    // Private constructor prevents instantiation
    RecommendationSystemLoader() = default;

    /**
     * Parses a movie line from input file
     * @param line Input line to parse
     * @param line_number Current line number for error reporting
     * @return Tuple of movie name, year, and features
     * @throws std::runtime_error if line format is invalid
     */
    static std::tuple<std::string, int, std::vector<double>> 
    parse_movie_line(const std::string& line, int line_number);

public:
    /**
     * Creates recommendation system from movie data file
     * @param movies_file_path Path to movie data file
     * @return Unique pointer to created recommendation system
     * @throws std::runtime_error if file cannot be read or has invalid format
     */
    static std::unique_ptr<RecommendationSystem> create_rs_from_movies(
        const std::string& movies_file_path);
};

#endif