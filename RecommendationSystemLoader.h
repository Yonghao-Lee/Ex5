/***************************************
 *  RecommendationSystemLoader.h
 ***************************************/
#ifndef RECOMMENDATIONSYSTEMLOADER_H
#define RECOMMENDATIONSYSTEMLOADER_H

#include "RecommendationSystem.h"
#include <memory>
#include <string>

class RecommendationSystemLoader {
private:
    RecommendationSystemLoader() = default;

public:
    /**
     * Create an RS from a file whose lines look like:
     *  "MovieName-Year feat1 feat2 ..."
     *  We throw if we see any feature outside [1..10].
     */
    static std::unique_ptr<RecommendationSystem> create_rs_from_movies(
        const std::string& movies_file_path);
};

#endif // RECOMMENDATIONSYSTEMLOADER_H
