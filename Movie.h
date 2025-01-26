#ifndef EX5_MOVIE_H
#define EX5_MOVIE_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>

#define HASH_START 17
#define RES_MULT 31

class Movie;
// Define smart pointer type for Movie
typedef std::shared_ptr<Movie> sp_movie;

typedef std::size_t (*hash_func)(const sp_movie& movie);
typedef bool (*equal_func)(const sp_movie& m1, const sp_movie& m2);

/**
 * Computes hash value for a movie based on its name and year
 * @param movie Shared pointer to the movie
 * @return Hash value for the movie
 */
std::size_t sp_movie_hash(const sp_movie& movie);

/**
 * Checks if two movies are equal based on name and year
 * @param m1 First movie to compare
 * @param m2 Second movie to compare
 * @return true if movies are equal, false otherwise
 */
bool sp_movie_equal(const sp_movie& m1, const sp_movie& m2);

class Movie {
private:
    std::string name;
    int year;

public:
    /**
     * Constructor for Movie class
     * @param name Name of the movie
     * @param year Release year of the movie
     * @throws std::invalid_argument if name is empty or year is invalid
     */
    Movie(const std::string& name, int year)
        : name(name), year(year)
    {
        if (name.empty()) {
            throw std::invalid_argument("Movie name cannot be empty");
        }
        if (year < 1888) { // First known movie was in 1888
            throw std::invalid_argument("Invalid movie year");
        }
    }

    /**
     * Gets the movie's name
     * @return const reference to movie's name
     */
    const std::string& get_name() const { return name; }

    /**
     * Gets the movie's release year
     * @return movie's release year
     */
    int get_year() const { return year; }

    /**
     * Compares movies based on year and then name
     * @param rhs Movie to compare with
     * @return true if this movie should be ordered before rhs
     */
    bool operator<(const Movie& rhs) const {
        if (year != rhs.year) {
            return year < rhs.year;
        }
        return name < rhs.name;
    }

    /**
     * Outputs movie information in format: name (year)
     * @param os Output stream
     * @param movie Movie to output
     * @return Reference to output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const Movie& movie) {
        os << movie.name << " (" << movie.year << ")";
        return os;
    }
};

#endif