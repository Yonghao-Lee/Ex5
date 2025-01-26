/***************************************
 *  UsersLoader.h
 ***************************************/
#ifndef USERSLOADER_H
#define USERSLOADER_H

#include "User.h"
#include <vector>
#include <string>
#include <memory>

class UsersLoader {
private:
    UsersLoader() = default;

public:
    /**
     * Create users from a file whose first line is:
     *   "User <Movie1-Year> <Movie2-Year> ..."
     * Then each subsequent line is:
     *   "<username> rating1 rating2 ..."
     * We throw if we see an out-of-range rating.
     */
    static std::vector<User> create_users(const std::string &users_file_path,
                                         std::shared_ptr<RecommendationSystem> rs);
};

#endif // USERSLOADER_H
