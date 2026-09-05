//
// Created by robert.archambault on 9/2/2026.
//

#include "ObstacleAvoidanceRule.h"
#include "imgui.h"
#include <glm/glm.hpp>
#include<algorithm>

// this does not get called from computeForce it is called once per frame by FlockingManager
// whenever a right-click happens, so it is not adding/removing an obstacle once per boid
void ObstacleAvoidanceRule::handleRightClick(glm::vec2 mousePos)
{
  //if the click landed on a obstacle, it removes it instade of staking
  auto it = std::find_if(obstacles.begin(), obstacles.end(), [&](const glm::vec2& o) {return glm::length(o -mousePos) <= obstacleRadius;});

  if (it != obstacles.end())
  {
    obstacles.erase(it);
  }
  else
  {
  obstacles.push_back(mousePos);
  }
}

glm::vec2 ObstacleAvoidanceRule::computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid)
{
  glm::vec2 force(0.0f);

  //it checks the boid against every obstacle that is currently placed, and not just nearby ones
  for (const auto& obstacle : obstacles)
  {
    glm::vec2 away =  boid.position - obstacle;
    float distance = glm::length(away);
    float dangerZone = obstacleRadius + avoidanceRange;

    //dangerZone is the obstacle's solid radius plus its avoidance range around that
    // and outside of dangerZone the obstacle has zero effect on this boid at all
    if (distance > 0.0001f && distance < dangerZone)
    {
      glm::vec2 direction = away / distance;
      //it is more powerful the closer it is to the obstcle
      float penetration = dangerZone - distance;
      force += direction * (penetration / avoidanceRange) * dangerZone;
    }
  }
  return force;
}

bool ObstacleAvoidanceRule::drawImguiRuleExtra()
{
  bool valueHasChanged = false;
  if (ImGui::SliderFloat("Obstacle Radius", &obstacleRadius, 5.f, 100.f, "%.f")) valueHasChanged = true;
  if (ImGui::SliderFloat("Avoidance Range", &avoidanceRange, 5.f, 200.f, "%.f")) valueHasChanged = true;

  ImGui::TextDisabled("Right-click empty space to add an obstacle,");
  ImGui::TextDisabled("right-click an obstacle to remove it.");

  if (ImGui::Button("Clear All Obstacles")) {
    obstacles.clear();
    valueHasChanged = true;
  }

  return valueHasChanged;
}

//it draws each obstacle as a solid circle for its actual body, plus a fainter outer ring
// showing the avoidance range, so you can visually see exactly where the force kicks in
void ObstacleAvoidanceRule::drawWorldOverlay(ImDrawList* dl) const
{
  ImU32 fillCol = IM_COL32(static_cast<int>(debugColor.r * 255), static_cast<int>(debugColor.g * 255), static_cast<int>(debugColor.b * 255), 90);
  ImU32 outlineCol = IM_COL32(static_cast<int>(debugColor.r * 255), static_cast<int>(debugColor.g * 255), static_cast<int>(debugColor.b * 255), 220);
  ImU32 rangeCol = IM_COL32(static_cast<int>(debugColor.r * 255), static_cast<int>(debugColor.g * 255), static_cast<int>(debugColor.b * 255), 60);

  for (const auto& obstacle : obstacles)
  {
    dl->AddCircleFilled({obstacle.x, obstacle.y}, obstacleRadius, fillCol, 24);
    dl->AddCircle({obstacle.x, obstacle.y}, obstacleRadius, outlineCol, 24, 2.f);
    dl->AddCircle({obstacle.x, obstacle.y}, obstacleRadius + avoidanceRange, rangeCol, 24, 1.f);
  }
}
