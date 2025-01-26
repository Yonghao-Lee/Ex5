
#ifndef RECOMMENDATIONSYSTEMLOADER_H
#define RECOMMENDATIONSYSTEMLOADER_H
#include "RecommendationSystem.h"

class RecommendationSystemLoader {

 private:

 public:
  RecommendationSystemLoader () = delete;

  static std::unique_ptr<RecommendationSystem> create_rs_from_movies
	  (const std::string &movies_file_path) noexcept (false);
};

#endif //RECOMMENDATIONSYSTEMLOADER_H
