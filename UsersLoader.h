#ifndef USERLOADER_H
#define USERLOADER_H
#include <sstream>
#include <fstream>
#include <vector>
#include "User.h"
#include "RecommendationSystem.h"

#define YEAR_SEPARATOR '-'


class UsersLoader
{
private:


public:
    UsersLoader() = delete;
    static std::vector<User> create_users(const std::string& users_file_path,
                     std::unique_ptr<RecommendationSystem> rs) noexcept(false);

};


#endif //USERLOADER_H
