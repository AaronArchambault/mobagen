//
// Created by robert.archambault on 9/7/2026.
//

#include "HighLife.h"
#include "../fsm/Action.h"
#include "../fsm/AgentContext.h"
#include "../fsm/Condition.h"
//HighLife (B36/S23): a live cell survives with 2 or 3 neighbors (so it dies
//below 2 or above 3, exactly like Conway); a dead cell is born with 3 OR 6
//neighbors. Famous for supporting a naturally-occurring replicator pattern.
//Everything but Reproduction is copy-pasted from JohnConway on purposeit
//is the same two-state graph, we're just swapping one Condition.

class Underpopulation : public Condition {
public:
  bool Test(const AgentContext& context) override { return context.isAlive && context.aliveNeighbors < 2; }
};

class Overpopulation : public Condition {
public:
  bool Test(const AgentContext& context) override { return context.isAlive && context.aliveNeighbors > 3; }
};

class Reproduction : public Condition {
public:
  bool Test(const AgentContext& context) override {
    return !context.isAlive && (context.aliveNeighbors == 3 || context.aliveNeighbors == 6);
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

HighLife::HighLife() {
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

void HighLife::Step(World& world) {
  for (int y = 0; y < world.Height(); ++y) {
    for (int x = 0; x < world.Width(); ++x) {
      AgentContext context{world, {x, y}, world.Get({x, y}), CountNeighbors(world, {x, y})};
      machine.SetCurrent(context.isAlive ? alive : dead);
      machine.Update(context);
    }
  }
}

int HighLife::CountNeighbors(World& world, Point2D point) {
  int count = 0;
  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      if (dx == 0 && dy == 0) continue;
      if (world.Get({point.x + dx, point.y + dy})) ++count;
    }
  }
  return count;
}
