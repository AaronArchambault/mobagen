//
// Created by robert.archambault on 9/7/2026.
//

#include "Seeds.h"
#include "../fsm/Action.h"
#include "../fsm/AgentContext.h"
#include "../fsm/Condition.h"
//Seeds (B2/S) there is no survival rule at all every live cell dies next
//generation regardless of its neighbor count. A dead cell is born with
//exactly 2 live neighbors. Because "always die" isn't conditioned on
//anything about the cell's neighborhood, it's modeled as a transition
//guarded by a Condition that's always true, rather than the usual
//Underpopulation/Overpopulation pair. "Alive" therefore has no stay action:
//it never survives an update, it only ever transitions out.

class AlwaysTrue : public Condition {
  public:
  bool Test(const AgentContext&) override {return true;}
};

class Reproduction : public Condition {
  public:
  bool Test(const AgentContext& context) override {return !context.isAlive && context.aliveNeighbors == 2; }

};

class DieAction : public Action {
  public:
    void Execute(const AgentContext& context) override {context.world.SetNext(context.position, false);}
};

class BornAction : public Action {
  public:
  void Execute(const AgentContext& context) override {context.world.SetNext(context.position, true);}
};

class StayDeadAction : public Action {
  public:
  void Execute(const AgentContext& context) override {context.world.SetNext(context.position, false);}
};

Seeds::Seeds() {
  alive = std::make_shared<State>("Alive");
  dead = std::make_shared<State>("Dead");

  const auto die = std::make_shared<DieAction>();
  const auto born = std::make_shared<BornAction>();

  //alive is only transient and it always transitons to dead so it has no stay action
  alive->AddTransition(std::make_shared<AlwaysTrue>(), dead, {die});

  dead->AddTransition(std::make_shared<Reproduction>(), alive, {born});
  dead->AddAction(std::make_shared<StayDeadAction>());
}

void Seeds::Step(World& world) {
  for (int y = 0; y < world.Height(); ++y)
    {
    for (int x = 0; x < world.Width(); ++x)
      {
        AgentContext context{world, {x, y}, world.Get({x, y}), CountNeighbors(world, {x, y})};
        machine.SetCurrent(context.isAlive ? alive : dead);
        machine.Update(context);
      }
    }
}

int Seeds::CountNeighbors(World& world, Point2D point) {
  int count = 0;
  for (int dy = -1; dy <= 1; ++dy)
    {
    for (int dx = -1; dx <= 1; ++dx)
      {
      if (dx == 0 && dy == 0) continue;
      if (world.Get({point.x + dx, point.y + dy})) ++count;
      }
    }
  return count;
}