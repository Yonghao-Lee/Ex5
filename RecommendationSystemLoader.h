#ifndef RECOMMENDATIONSYSTEMLOADER_H
#define RECOMMENDATIONSYSTEMLOADER_H

#include "RecommendationSystem.h"
#include <memory>
#include <string>
#include <iostream>
#include <fstream>

class RecommendationSystemLoader {
private:
    // Private constructor prevents instantiation since this is a utility class
    RecommendationSystemLoader() = default;

public:
    // Static method to create and load a recommendation system from a file
    static std::unique_ptr<RecommendationSystem> create_rs_from_movies(
        const std::string& movies_file_path);
};

#endif // RECOMMENDATIONSYSTEMLOADER_H