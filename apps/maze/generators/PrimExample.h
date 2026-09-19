#ifndef MOBAGEN_EXAMPLES_MAZE_GENERATORS_PRIMEXAMPLE_H_
#define MOBAGEN_EXAMPLES_MAZE_GENERATORS_PRIMEXAMPLE_H_

#include <vector>
#include <string>
#include "../MazeGeneratorBase.h"
#include "math/Point2D.h"

class PrimExample : public MazeGeneratorBase {
private:
  std::vector<Point2D> toBeVisited;
  //flat bool per cell indexed by y times width plus x, so it can tell what is in the maze from
  //what is just on the frontier without the overhead of a map of maps
  std::vector<bool> visited;
  bool initialized = false;
  std::vector<Point2D> getVisitables(World* w, const Point2D& p);
  std::vector<Point2D> getVisitedNeighbors(World* w, const Point2D& p);

public:
  PrimExample() = default;
  std::string GetName() override { return "Prim"; };
  bool Step(World* world) override;
  void Clear(World* world) override;
};

#endif  //MOBAGEN_EXAMPLES_MAZE_GENERATORS_PRIMEXAMPLE_H_
