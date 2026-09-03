//
// Created by robert.archambault on 9/2/2026.
//

#ifndef MOBAGEN_LEADERFOLLOWRULE_H
#define MOBAGEN_LEADERFOLLOWRULE_H
#include "FlockingRule.h"

//it makes the flock fallow a single leader boid and it is steered with arow keys and instead of just the local center of mass
//every one of the boid is pulled toward a point trailing behind the leader, and based on the leaders current postion whihc makes a kind of
//trail like formation

class LeaderFollowRule : public FlockingRule {
private:
  glm::vec2 leaderPosition{0.f};
  glm::vec2 leaderVelocity{0.f};
  bool hasLeader = false;
  float followDistance = 40.f;

public:
  explicit LeaderFollowRule(float weight = 1.f, bool isEnabled = true) : FlockingRule(Color32(0.6f, 0.2f, 1.0f,1.0f), weight, isEnabled) {}

  LeaderFollowRule(const LeaderFollowRule& toCopy) : FlockingRule(toCopy), leaderPosition(toCopy.leaderPosition), leaderVelocity(toCopy.leaderVelocity), hasLeader(toCopy.hasLeader), followDistance(toCopy.followDistance) {}

  std::unique_ptr<FlockingRule> clone() override { return std::make_unique<LeaderFollowRule>(*this); }

  const char* getRuleName() override { return "Leader Following"; }
  const char* getRuleExplanation() override {
    return "Steer towrd a point traling behind the leader boid";
  }

  float getBaseWeightMultiplier() override {return 1.f; }
  //called once per frame by flockingManager with the current leaders state
  void setLeaderState(glm::vec2 position, glm::vec2 velocity);

  glm::vec2 computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid) override;
  bool drawImguiRuleExtra() override;
};

#endif  // MOBAGEN_LEADERFOLLOWRULE_H
