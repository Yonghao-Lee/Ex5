#include "Movie.h"
#define RES_MULT 31

std::size_t sp_movie_hash(const sp_movie& movie) {
    std::size_t res = HASH_START;
    res = res * RES_MULT + std::hash<std::string>()(movie->get_name());
    res = res * RES_MULT + std::hash<int>()(movie->get_year());
    return res;
}

bool sp_movie_equal(const sp_movie& m1, const sp_movie& m2) {
    return !(*m1 < *m2) && !(*m2 < *m1);
}

Movie::Movie(const std::string& name, int year) : _name(name), _year(year) {}

const std::string& Movie::get_name() const {
    return _name;
}

int Movie::get_year() const {
    return _year;
}

bool operator<(const Movie& lhs, const Movie& rhs) {
    if (lhs._year < rhs._year) {
        return true;
    }
    if (lhs._year == rhs._year) {
        return lhs._name < rhs._name;
    }
    return false;
}

std::ostream& operator<<(std::ostream& s, const Movie& movie) {
    return s << movie._name << " (" << movie._year << ")" << std::endl;
}
