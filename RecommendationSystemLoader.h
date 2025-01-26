#ifndef RECOMMENDATIONSYSTEMLOADER_H
#define RECOMMENDATIONSYSTEMLOADER_H

#include "RecommendationSystem.h"
#include <memory>
#include <string>

/**
 * Loads a RecommendationSystem from a file.
 * Format: "MovieName-Year feat1 feat2 ..."
 */
class RecommendationSystemLoader {
private:
    RecommendationSystemLoader() = default;

public:
    static std::unique_ptr<RecommendationSystem>
    create_rs_from_movies(const std::string& movies_file_path);
};

#endif