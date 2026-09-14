//
// Created by robert.archambault on 9/10/2026.
//

#include "HexagonB34GameOfLife.h"
#include "../fsm/Action.h"
#include "../fsm/AgentContext.h"
#include "../fsm/Condition.h"

#include <SDL3/SDL_log.h>

#include <stdexcept>

// Hexagonal variant: each cell has 6 neighbors instead of 8. This is the
// "up to you" variant originally used before switching the default Hexagon
// rule to match the reference site's B2/S2:
//   underpopulation (<3) / overpopulation (>4) -> conditions that leave Alive
//   reproduction (==2)                          -> condition that leaves Dead
//   survival is implicit: no transition firing means the stay actions run.
//
// hint: the app draws odd rows displaced by half a cell, so the neighbors
// above and below shift by one column depending on the row parity.
// Reference: https://arunarjunakani.github.io/HexagonalGameOfLife/

namespace {

class Underpopulation : public Condition {
public:
  bool Test(const AgentContext& context) override {
    //on the hex grid a live cell is underpopulated below 3 neighbors
    return context.isAlive && context.aliveNeighbors < 3;
  }
};

class Overpopulation : public Condition {
public:
  bool Test(const AgentContext& context) override {
    //on the hex grid a live cell is overpopulated above 4 neighbors
    return context.isAlive && context.aliveNeighbors > 4;
  }
};

class Reproduction : public Condition {
public:
  bool Test(const AgentContext& context) override {
    //on the hex grid a dead cell is born with exactly 2 neighbors
    return !context.isAlive && context.aliveNeighbors == 2;
  }
};

class DieAction : public Action {
public:
  void Execute(const AgentContext& context) override { context.world.SetNext(context.position, false); }
};

class BornAction : public Action {
public:
  void Execute(const AgentContext& context) override { context.world.SetNext(context.position, true); }
};

class StayAliveAction : public Action {
public:
  void Execute(const AgentContext& context) override { context.world.SetNext(context.position, true); }
};

class StayDeadAction : public Action {
public:
  void Execute(const AgentContext& context) override { context.world.SetNext(context.position, false); }
};

}  // namespace

HexagonB34GameOfLife::HexagonB34GameOfLife() {
  alive = std::make_shared<State>("Alive");
  dead = std::make_shared<State>("Dead");

  const auto die = std::make_shared<DieAction>();
  const auto born = std::make_shared<BornAction>();

  alive->AddTransition(std::make_shared<Underpopulation>(), dead, {die});
  alive->AddTransition(std::make_shared<Overpopulation>(), dead, {die});
  alive->AddAction(std::make_shared<StayAliveAction>());

  dead->AddTransition(std::make_shared<Reproduction>(), alive, {born});
  dead->AddAction(std::make_shared<StayDeadAction>());
}

void HexagonB34GameOfLife::Step(World& world) {
  for (int y = 0; y < world.Height(); ++y) {
    for (int x = 0; x < world.Width(); ++x) {
      AgentContext context{world, {x, y}, world.Get({x, y}), CountNeighbors(world, {x, y})};
      machine.SetCurrent(context.isAlive ? alive : dead);
      machine.Update(context);
    }
  }
}

int HexagonB34GameOfLife::CountNeighbors(World& world, Point2D point) {
  const bool oddRow = (point.y % 2 != 0);

  static const int evenRowOffsets[6][2] = {
      {-1, -1}, {0, -1},  //upper-left and upper-right
      {-1, 0},  {1, 0},   //left and right
      {-1, 1},  {0, 1}    //lower-left and lower-right
  };

  static const int oddRowOffsets[6][2] = {
      {0, -1}, {1, -1},  //upper-left and upper-right
      {-1, 0}, {1, 0},   //left and right
      {0, 1},  {1, 1}    //lower-left and lower-right
  };

  const int (*offsets)[2] = oddRow ? oddRowOffsets : evenRowOffsets;

  int count = 0;
  for (int i = 0; i < 6; ++i) {
    if (world.Get({point.x + offsets[i][0], point.y + offsets[i][1]})) ++count;
  }
  return count;
}
