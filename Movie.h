/***************************************
 *  Movie.h
 ***************************************/
#ifndef EX5_MOVIE_H
#define EX5_MOVIE_H

#include <iostream>
#include <memory>
#include <string>
#include <functional>

#define HASH_START 17
#define RES_MULT 31

class Movie;
typedef std::shared_ptr<Movie> sp_movie;

/** For use in std::unordered_map<sp_movie,...> */
typedef std::size_t (*hash_func)(const sp_movie&);
typedef bool       (*equal_func)(const sp_movie&, const sp_movie&);

/** Hash & equality for sp_movie. */
std::size_t sp_movie_hash(const sp_movie& movie);
bool sp_movie_equal(const sp_movie& m1, const sp_movie& m2);

class Movie {
private:
    std::string name;
    int year;

public:
    Movie(const std::string &name, int year) : name(name), year(year) {}

    const std::string& get_name() const { return name; }
    int get_year() const { return year; }

    /** Sort first by year ascending, then by name ascending. */
    bool operator<(const Movie &rhs) const {
        if (year != rhs.year) {
            return year < rhs.year;
        }
        return name < rhs.name;
    }

    /** Print as "Name (Year)\n". */
    friend std::ostream& operator<<(std::ostream &os, const Movie &movie) {
        os << movie.name << " (" << movie.year << ")\n";
        return os;
    }
};

#endif // EX5_MOVIE_H
