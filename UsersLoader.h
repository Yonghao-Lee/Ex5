#ifndef USERSLOADER_H
#define USERSLOADER_H

#include <memory>
#include <vector>
#include <string>
#include "User.h"
#include "RecommendationSystem.h"

/**
 * Responsible for creating User objects from a file that has a header line
 * of movies, followed by lines of "username rating1 rating2 ..."
 */
class UsersLoader {
private:
    UsersLoader() = default;

public:
    /**
     * Creates a vector of User objects from the given file, using the
     * provided RecommendationSystem for retrieving/validating movies.
     */
    static std::vector<User> create_users(
        const std::string& users_file_path,
        std::shared_ptr<RecommendationSystem> rs);
};

#endif