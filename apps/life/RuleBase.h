#ifndef LIFE_RULEBASE_H
#define LIFE_RULEBASE_H

// reference https://playgameoflife.com/info

#include "World.h"
#include <string>
#include "GameOfLifeTileSetEnum.h"
class RuleBase {
public:
  virtual ~RuleBase() = default;
  virtual void Step(World& world) = 0;
  virtual std::string GetName() = 0;
  virtual GameOfLifeTileSetEnum GetTileSet() = 0;
  //for the bonus that is for the heamap and it can aks any rule pretty much how many neighbors does this cell have without fully knowing the rule activityl
  virtual int CountNeighbors(World& world, Point2D point) = 0;
};

#endif  // MOBAGEN_RULEBASE_H
