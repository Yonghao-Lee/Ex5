#ifndef EX5_MOVIE_H
#define EX5_MOVIE_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <functional>

#define HASH_START 17
#define RES_MULT 31

class Movie;
typedef std::shared_ptr<Movie> sp_movie;

// Hash functor for sp_movie
struct sp_movie_hash {
    std::size_t operator()(const sp_movie& movie) const {
        if (!movie) {
            throw std::invalid_argument("Null movie pointer");
        }
        std::size_t result = HASH_START;
        result = result * RES_MULT + std::hash<std::string>()(movie->get_name());
        result = result * RES_MULT + std::hash<int>()(movie->get_year());
        return result;
    }
};

// Equality functor for sp_movie
struct sp_movie_equal {
    bool operator()(const sp_movie& m1, const sp_movie& m2) const {
        if (!m1 || !m2) {
            throw std::invalid_argument("Null movie pointer");
        }
        // Two movies are equal if neither is strictly less than the other
        return !(*m1 < *m2) && !(*m2 < *m1);
    }
};

// Hash + equality for using sp_movie in std::unordered_map
// These are no longer needed as functors are defined above

class Movie {
private:
    std::string name;
    int year;

public:
    Movie(const std::string& name, int year)
        : name(name), year(year)
    {
        if (name.empty()) {
            throw std::invalid_argument("Movie name cannot be empty");
        }
        if (year < 1888) {
            throw std::invalid_argument("Invalid movie year");
        }
    }

    const std::string& get_name() const { return name; }
    int get_year() const { return year; }

    // Compare first by year, then by name
    bool operator<(const Movie& rhs) const {
        if (year != rhs.year) {
            return year < rhs.year;
        }
        return name < rhs.name;
    }

    friend std::ostream& operator<<(std::ostream& os, const Movie& movie) {
        // Print each movie: "Name (Year)\n"
        os << movie.name << " (" << movie.year << ")\n";
        return os;
    }
};

#endif // EX5_MOVIE_H