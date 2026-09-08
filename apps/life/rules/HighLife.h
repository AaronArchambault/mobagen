//
// Created by robert.archambault on 9/7/2026.
//

#ifndef MOBAGEN_HIGHLIFE_H
#define MOBAGEN_HIGHLIFE_H

#include "../RuleBase.h"
#include "../fsm/State.h"
#include "../fsm/StateMachine.h"
#include <memory>
#include <string>
//Bonus HighLife (B36/S23). Identical machine shape to JohnConway
//same Underpopulation/Overpopulation conditions, same Die/StayAlive/StayDead
//actions the only change is Reproduction, which also fires at 6 neighbors.
//This demonstrates that the FSM design generalizes to rule variants by
//swapping out a single Condition, without touching the graph wiring shape.

class HighLife : public RuleBase {
public:
  HighLife();
  ~HighLife() override = default;
  std::string GetName() override { return "HighLife (B36/S23)"; }
  void Step(World& world) override;
  int CountNeighbors(World& world, Point2D point);
  GameOfLifeTileSetEnum GetTileSet() override { return GameOfLifeTileSetEnum::Square; };

private:
  std::shared_ptr<State> alive;
  std::shared_ptr<State> dead;
  StateMachine machine;
};

#endif  // MOBAGEN_HIGHLIFE_H
