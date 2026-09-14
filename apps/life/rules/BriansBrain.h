//
// Created by robert.archambault on 9/7/2026.
//

#ifndef MOBAGEN_BRIANSBRAIN_H
#define MOBAGEN_BRIANSBRAIN_H
#include "../RuleBase.h"
#include "../fsm/State.h"
#include "../fsm/StateMachine.h"
#include <memory>
#include <string>
//bonus that is a celluar automaton with three states rather than the 2
//Neighbor counting only counts "On" cells - a Dying neighbor does not
//count towards a dead cell's rebirth, exactly like the classic rule
class BriansBrain : public RuleBase {
public:
  BriansBrain();
  ~BriansBrain() override = default;
  std::string GetName() override { return "Brian's Brain (3-state)"; }
  void Step(World& world) override;
  int CountNeighbors(World& world, Point2D point) override;
  GameOfLifeTileSetEnum GetTileSet() override { return GameOfLifeTileSetEnum::Square; };

private:
  std::shared_ptr<State> on;
  std::shared_ptr<State> dying;
  std::shared_ptr<State> off;
  StateMachine machine;
};


#endif  // MOBAGEN_BRIANSBRAIN_H
