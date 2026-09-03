//
// Created by robert.archambault on 9/2/2026.
//

#ifndef MOBAGEN_OBSTACLEAVOIDANCERULE_H
#define MOBAGEN_OBSTACLEAVOIDANCERULE_H

#include "FlockingRule.h"

//boids go away form circule obstacles placed by the user and the right click empty space to add an obstacle and right click and abstcle to get rid of it

class ObstacleAvoidanceRule : public FlockingRule {
private:
  std::vector<glm::vec2> obstacles;
  float obstacleRadius = 30.0f;
  float avoidanceRange = 60.0f;

public:
  explicit ObstacleAvoidanceRule(float weight = 1.0f, bool isEnabled = true) : FlockingRule(Color32(1.0f, 0.55f, 0.1f, 1.0f), weight, isEnabled) {}

  ObstacleAvoidanceRule(const ObstacleAvoidanceRule& toCopy) : FlockingRule(toCopy), obstacles(toCopy.obstacles), obstacleRadius(toCopy.obstacleRadius), avoidanceRange(toCopy.avoidanceRange) {}

  std::unique_ptr<FlockingRule> clone() override { return std::make_unique<ObstacleAvoidanceRule>(*this); }

  const char* getRuleName() override { return " Obstacle Avoidance"; }
  const char * getRuleExplanation() override { return "Steer away from placed obstacles. Right-click empty space to add one, right-click an obstacle to remove it.";}

  float getBaseWeightMultiplier() override { return 1.f; }
  void handleRightClick(glm::vec2 mousePos);

  glm::vec2 computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid) override;
  bool drawImguiRuleExtra() override;
  void drawWorldOverlay(ImDrawList* dl) const override;
};

#endif  // MOBAGEN_OBSTACLEAVOIDANCERULE_H
