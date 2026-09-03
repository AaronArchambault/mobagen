//
// Created by robert.archambault on 9/2/2026.
//

#include "LeaderFollowRule.h"
#include <glm/glm.hpp>
#include <imgui.h>

void LeaderFollowRule::setLeaderState(glm::vec2 position, glm::vec2 velocity)
{
  leaderPosition = position;
  leaderVelocity = velocity;
  hasLeader = true;
}

glm::vec2 LeaderFollowRule::computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid)
{
  if (!hasLeader)
  {
    return glm::vec2(0.0f);
  }

  float speed = glm::length(leaderVelocity);
  glm::vec2 forward = speed > 0.0001f ? leaderVelocity / speed : glm::vec2(0.f, -1.f);
  glm::vec2 targetPoint = leaderPosition - forward * followDistance;
  return targetPoint - boid.position;
}

bool LeaderFollowRule::drawImguiRuleExtra()
{
  bool valueHasChanged = false;
  if (ImGui::SliderFloat("Fallow Distance", &followDistance, 0.0f, 200.f, "%.f")) valueHasChanged = true;
  ImGui::TextDisabled("Steer the leader with the arrow keys");
  return valueHasChanged;
}
