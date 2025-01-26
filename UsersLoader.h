#ifndef USERSLOADER_H
#define USERSLOADER_H

#include <memory>
#include <vector>
#include <string>
#include "User.h"
#include "RecommendationSystem.h"

/**
 * Creates User objects from a file with:
 *  1) First line = list of "MovieName-Year" tokens (optionally prefixed with "USER")
 *  2) Subsequent lines = "username rating rating rating..."
 */
class UsersLoader {
private:
    UsersLoader() = default;

    // Helper to parse "MovieName-Year" and retrieve the movie from RS
    static sp_movie parseMovieFromHeader(const std::string& movie_info,
                                         std::shared_ptr<RecommendationSystem> rs);

public:
    static std::vector<User> create_users(const std::string& users_file_path,
                                          std::shared_ptr<RecommendationSystem> rs);
};

#endif // USERSLOADER_H