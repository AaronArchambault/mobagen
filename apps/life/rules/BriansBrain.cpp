//
// Created by robert.archambault on 9/7/2026.
//

#include "BriansBrain.h"
#include "../fsm/Action.h"
#include "../fsm/AgentContext.h"
#include "../fsm/Condition.h"

//Brian's Brain: On -> Dying -> Off are both unconditional (same
//AlwaysTrue-guarded-transition trick used in Seeds), because "always move
//on to the next state" isn't a function of the neighborhood at all. Off ->
//On is the only conditioned transition, firing on exactly 2 "On" neighbors.

class AlwaysTrue : public Condition {
public:
  bool Test(const AgentContext&) override { return true; }
};

class Reproduction : public Condition {
public:
  //Off -> On when exactly 2 of the 8 neighbors are "On".
  bool Test(const AgentContext& context) override { return context.aliveNeighbors == 2; }
};

class BecomeDyingAction : public Action {
public:
  //On -> Dying: clear the "on" bit, raise the "dying" bit.
  void Execute(const AgentContext& context) override {
    context.world.SetNext(context.position, false);
    context.world.SetNextExtra(context.position, true);
  }
};

class BecomeOffAction : public Action {
public:
  //Dying -> Off: both bits clear.
  void Execute(const AgentContext& context) override {
    context.world.SetNext(context.position, false);
    context.world.SetNextExtra(context.position, false);
  }
};

class BecomeOnAction : public Action {
public:
  // ff -> On: raise the "on" bit, "dying" bit stays clear.
  void Execute(const AgentContext& context) override {
    context.world.SetNext(context.position, true);
    context.world.SetNextExtra(context.position, false);
  }
};

class StayOffAction : public Action {
public:
  //Off stays Off: keep writing both bits false explicitly

  void Execute(const AgentContext& context) override {
    context.world.SetNext(context.position, false);
    context.world.SetNextExtra(context.position, false);
  }
};

BriansBrain::BriansBrain() {
  on = std::make_shared<State>("On");
  dying = std::make_shared<State>("Dying");
  off = std::make_shared<State>("Off");

  //On is purely transient, exactly like Alive in Seeds: it always
  //transitions out, so it has no stay action.
  on->AddTransition(std::make_shared<AlwaysTrue>(), dying, {std::make_shared<BecomeDyingAction>()});

  //Dying is also purely transient.
  dying->AddTransition(std::make_shared<AlwaysTrue>(), off, {std::make_shared<BecomeOffAction>()});

  //Off is the only state that can actually stay put.
  off->AddTransition(std::make_shared<Reproduction>(), on, {std::make_shared<BecomeOnAction>()});
  off->AddAction(std::make_shared<StayOffAction>());
}

void BriansBrain::Step(World& world) {
  for (int y = 0; y < world.Height(); ++y) {
    for (int x = 0; x < world.Width(); ++x) {
      const Point2D position{x, y};
      const bool isOn = world.Get(position);
      const bool isDying = world.GetExtra(position);

      //it reconstructs which of the three shared state nodes this cell is
      //currently in from the two bits, then let the machine take it from
      //there exactly like every other rule in this project.
      std::shared_ptr<State> current = isOn ? on : (isDying ? dying : off);

      AgentContext context{world, position, isOn, CountNeighbors(world, position)};
      machine.SetCurrent(current);
      machine.Update(context);
    }
  }
}

int BriansBrain::CountNeighbors(World& world, Point2D point) {
  //only "On" cells count dying cells are deliberately invisible here, which is what makes Brian's Brain behave differently from a simple
  //2-state automaton with an added delay.
  int count = 0;
  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      if (dx == 0 && dy == 0) continue;
      if (world.Get({point.x + dx, point.y + dy})) ++count;
    }
  }
  return count;
}
