//
// Created by robert.archambault on 9/2/2026.
//

#include "LeaderFollowRule.h"
#include <glm/glm.hpp>
#include <imgui.h>

//it is called once per frame from FlockingManager::Update, with the current position/velocity
//and that means and makes it so that of whichever boid is being steered by the arrow keys. this is how the rule finds out
//and that where the leader is without having to search for it itself
void LeaderFollowRule::setLeaderState(glm::vec2 position, glm::vec2 velocity)
{
  leaderPosition = position;
  leaderVelocity = velocity;
  hasLeader = true;
}

glm::vec2 LeaderFollowRule::computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid)
{

  //and it is so that if setLeaderState has not been called yet there is no leader to follow,
  //so it does not apply any force at all
  if (!hasLeader)
  {
    return glm::vec2(0.0f);
  }

  //it figures out which way the leader is currently facing, based on its velocity
  float speed = glm::length(leaderVelocity);
  glm::vec2 forward = speed > 0.0001f ? leaderVelocity / speed : glm::vec2(0.f, -1.f);

  //the actual target is not the leader's exact position and it is a point trailing behind it,
  //and that is so the flock forms up behind the leader instead of all trying to occupy the same spot
  glm::vec2 targetPoint = leaderPosition - forward * followDistance;

  //it is the constant-magnitude pull toward the trail point instead of one that grows with
  //the distance and without this, boids far from the leader get slammed toward it at huge
  //force the moment the rule is enabled, pile up right next to the leader, and then
  //get shoved right back out by SeparationRule
  glm::vec2 toTarget = targetPoint - boid.position;
  float distance = glm::length(toTarget);
  return distance > 0.0001f ? toTarget / distance : glm::vec2(0.0f);
}

bool LeaderFollowRule::drawImguiRuleExtra()
{
  bool valueHasChanged = false;
  if (ImGui::SliderFloat("Fallow Distance", &followDistance, 0.0f, 200.f, "%.f")) valueHasChanged = true;
  ImGui::TextDisabled("Steer the leader with the arrow keys");
  return valueHasChanged;
}
