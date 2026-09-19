#ifndef RECURSIVEBACKTRACKER_H
#define RECURSIVEBACKTRACKER_H

#include "../MazeGeneratorBase.h"
#include <string>
#include "math/Point2D.h"
#include <vector>

class RecursiveBacktrackerExample : public MazeGeneratorBase {
private:
  //the path is tracked in FORMAL units: (0, 0) is the top-left cell,
  //x grows right and y grows down; World::ToWorldCoords/ToFormalCoords
  //translate to and from the world's centered units.
  std::vector<Point2D> stack;
  //flat bool per cell indexed by y times width plus x, instead of a map of maps, one allocation
  //and O(1) lookups instead of two tree walks and a bunch of node allocations per access
  std::vector<bool> visited;
  std::vector<Point2D> getVisitables(World* w, const Point2D& formalPoint);

public:
  RecursiveBacktrackerExample() = default;
  std::string GetName() override { return "Recursive Back-Tracker"; };
  bool Step(World* world) override;
  void Clear(World* world) override;
};

#endif  //RECURSIVEBACKTRACKER_H
