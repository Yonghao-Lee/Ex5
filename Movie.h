#ifndef EX5_MOVIE_H
#define EX5_MOVIE_H

#include <iostream>
#include <vector>
#include <memory>
#include <functional>

// We keep these #defines for hashing:
#define HASH_START 17
#define RES_MULT 31

class Movie;

// Define smart pointer type for Movie
typedef std::shared_ptr<Movie> sp_movie;

// Custom hashing & equality for sp_movie
typedef std::size_t (*hash_func)(const sp_movie& movie);
typedef bool (*equal_func)(const sp_movie& m1,const sp_movie& m2);

/**
 * Hashing function for sp_movie based on (name, year).
 */
std::size_t sp_movie_hash(const sp_movie& movie);

/**
 * Equality function for sp_movie: compares (year, name).
 */
bool sp_movie_equal(const sp_movie& m1,const sp_movie& m2);

class Movie {
private:
    std::string name;
    int year;

public:
    Movie(const std::string& name, int year) : name(name), year(year) {}

    const std::string& get_name() const { return name; }
    int get_year() const { return year; }

    // Sort by year ascending; if tie, by name ascending
    bool operator<(const Movie& rhs) const {
        if (year != rhs.year) {
            return year < rhs.year;
        }
        return name < rhs.name;
    }

    // Print exactly "<name> (<year>)\n"
    friend std::ostream& operator<<(std::ostream& os, const Movie& movie) {
        os << movie.name << " (" << movie.year << ")\n";
        return os;
    }
};

#endif // EX5_MOVIE_H
