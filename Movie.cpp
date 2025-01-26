#include "Movie.h"

std::size_t sp_movie_hash(const sp_movie& movie){
    // Hash by name + year
    std::size_t res = HASH_START;
    res = res * RES_MULT + std::hash<std::string>()(movie->get_name());
    res = res * RES_MULT + std::hash<int>()(movie->get_year());
    return res;
}

bool sp_movie_equal(const sp_movie& m1,const sp_movie& m2){
    // They are equal if neither is < the other
    // i.e. same (year, name)
    return !(*m1 < *m2) && !(*m2 < *m1);
}
