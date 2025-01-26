#include "Movie.h"

std::size_t sp_movie_hash(const sp_movie& movie) {
    if (!movie) throw std::invalid_argument("Null movie pointer");
    std::size_t result = HASH_START;
    result = result * RES_MULT + std::hash<std::string>()(movie->get_name());
    result = result * RES_MULT + std::hash<int>()(movie->get_year());
    return result;
}

bool sp_movie_equal(const sp_movie& m1, const sp_movie& m2) {
    if (!m1 || !m2) throw std::invalid_argument("Null movie pointer");
    return !(*m1 < *m2) && !(*m2 < *m1);
}