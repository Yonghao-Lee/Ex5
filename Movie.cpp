/***************************************
 *  Movie.cpp
 ***************************************/
#include "Movie.h"

std::size_t sp_movie_hash(const sp_movie& movie)
{
    // combine the hash of name + year
    std::size_t res = HASH_START;
    res = res * RES_MULT + std::hash<std::string>()(movie->get_name());
    res = res * RES_MULT + std::hash<int>()(movie->get_year());
    return res;
}

bool sp_movie_equal(const sp_movie& m1, const sp_movie& m2)
{
    // equal if neither is < the other
    return !(*m1 < *m2) && !(*m2 < *m1);
}
