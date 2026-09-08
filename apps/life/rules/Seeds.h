//
// Created by robert.archambault on 9/7/2026.
//

#ifndef MOBAGEN_SEEDS_H
#define MOBAGEN_SEEDS_H

#include "../RuleBase.h"
#include "../fsm/State.h"
#include "../fsm/StateMachine.h"
#include <memory>
#include <string>
//Bonus Seeds (B2/S - empty survival ruleset). Every live cell dies
//every generation no matter its neighbor count; a dead cell is born with
//exactly 2 neighbors. This is architecturally interesting: since there is NO
//survival condition, "Alive" has a single not conditional transition to "Dead"
//instead of the usual pair of Underpopulation/Overpopulation guards. It demonstrates that the framework
// doesn't require states to have any stay behavior at all a state can be
// purely transient.

class Seeds : public RuleBase {
public:
  Seeds();
  ~Seeds() override = default;
  std::string GetName() override { return "Seeds (B2/S"; }
  void Step(World& world) override;
  int CountNeighbors(World& world, Point2D point);
  GameOfLifeTileSetEnum GetTileSet() override { return GameOfLifeTileSetEnum::Square;};

private:
  std::shared_ptr<State> alive;
  std::shared_ptr<State> dead;
  StateMachine machine;
};


#endif  // MOBAGEN_SEEDS_H
