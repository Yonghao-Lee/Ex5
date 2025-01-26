#ifndef USERSLOADER_H
#define USERSLOADER_H

#include <memory>
#include <vector>
#include <string>
#include "User.h"
#include "RecommendationSystem.h"

/**
 * Creates User objects from a file with:
 *  1) First line = list of "MovieName-Year" tokens
 *  2) Subsequent lines = "username rating rating rating..."
 */
class UsersLoader {
private:
    UsersLoader() = default;

public:
    static std::vector<User> create_users(const std::string& users_file_path,
                                          std::shared_ptr<RecommendationSystem> rs);
};

#endif