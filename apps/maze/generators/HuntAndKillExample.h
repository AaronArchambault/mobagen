#ifndef HUNTANDKILLEXAMPLE_H
#define HUNTANDKILLEXAMPLE_H

#include "../MazeGeneratorBase.h"
#include <string>
#include "math/Point2D.h"
#include <vector>

class HuntAndKillExample : public MazeGeneratorBase {
private:
  std::vector<Point2D> stack;
  //flat bool per cell indexed by y times width plus x instead of a map of maps, this matters more
  //here than the other generators since the hunt scan walks nearly the whole grid over and over
  std::vector<bool> visited;
  Point2D randomStartPoint(World* world);
  std::vector<Point2D> getVisitables(World* w, const Point2D& p);
  std::vector<Point2D> getVisitedNeighbors(World* w, const Point2D& p);

public:
  HuntAndKillExample() = default;
  std::string GetName() override { return "HuntAndKill"; };
  bool Step(World* world) override;
  void Clear(World* world) override;
};

#endif  //HUNTANDKILLEXAMPLE_H
