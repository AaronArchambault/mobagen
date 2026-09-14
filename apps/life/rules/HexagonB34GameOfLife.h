//
// Created by robert.archambault on 9/10/2026.
//

#ifndef MOBAGEN_HEXAGONB34GAMEOFLIFE_H
#define MOBAGEN_HEXAGONB34GAMEOFLIFE_H

#include "../RuleBase.h"
#include "../fsm/State.h"
#include "../fsm/StateMachine.h"
#include <memory>
#include <string>

// The hex rule variant (survive on 3-4 neighbors)
class HexagonB34GameOfLife : public RuleBase {
public:
  HexagonB34GameOfLife();
  ~HexagonB34GameOfLife() override = default;
  std::string GetName() override { return "Hexagon (B2/S34)"; }
  void Step(World& world) override;
  int CountNeighbors(World& world, Point2D point);
  GameOfLifeTileSetEnum GetTileSet() override { return GameOfLifeTileSetEnum::Hexagon; };

private:
  // the shared state graph: every cell runs this same machine
  std::shared_ptr<State> alive;
  std::shared_ptr<State> dead;
  StateMachine machine;
};


#endif  // MOBAGEN_HEXAGONB34GAMEOFLIFE_H
