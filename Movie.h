#ifndef EX5_MOVIE_H
#define EX5_MOVIE_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>

#define HASH_START 17
#define RES_MULT 31

class Movie;
typedef std::shared_ptr<Movie> sp_movie;
typedef std::size_t (*hash_func)(const sp_movie& movie);
typedef bool (*equal_func)(const sp_movie& m1, const sp_movie& m2);

std::size_t sp_movie_hash(const sp_movie& movie);
bool sp_movie_equal(const sp_movie& m1, const sp_movie& m2);

class Movie {
private:
    std::string name;
    int year;

public:
    Movie(const std::string& name, int year) : name(name), year(year) {
        if (name.empty()) {
            throw std::invalid_argument("Movie name cannot be empty");
        }
        if (year < 1888) {
            throw std::invalid_argument("Invalid movie year");
        }
    }

    const std::string& get_name() const { return name; }
    int get_year() const { return year; }

    bool operator<(const Movie& rhs) const {
        if (year != rhs.year) {
            return year < rhs.year;
        }
        return name < rhs.name;
    }

    friend std::ostream& operator<<(std::ostream& os, const Movie& movie) {
        os << movie.name << " (" << movie.year << ")";
        return os;
    }
};

#endif