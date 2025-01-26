#ifndef USERSLOADER_H
#define USERSLOADER_H

#include <memory>
#include <vector>
#include <string>
#include "User.h"
#include "RecommendationSystem.h"

class UsersLoader {
private:
    UsersLoader() = default;

    // Helper to parse "Name-Year" from header and find the movie in the RS
    static sp_movie parseMovieFromHeader(const std::string& movie_info,
                                         std::shared_ptr<RecommendationSystem> rs);

public:
    static std::vector<User> create_users(const std::string& users_file_path,
                                          std::shared_ptr<RecommendationSystem> rs);
};

#endif