#include "BoundedAreaRule.h"
#include "imgui.h"
#include <glm/glm.hpp>
#include <algorithm>

glm::vec2 BoundedAreaRule::computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid) {
  glm::vec2 force(0.f);
  ImVec2 displaySize = ImGui::GetIO().DisplaySize;
  // desiredDistance is the distance from the borders that the boids should try to maintain.

  // begin solution
  float w = displaySize.x > 0.f ? displaySize.x : 1280.f;
  float h = displaySize.y > 0.f ? displaySize.y : 800.f;
  float d = static_cast<float>(desiredDistance);

  //the closer the boid is to a border, the stronger the pushback toawrds it and its center should be and
  //it should be proportional to the pentration depth and that said a boid barely inside the margin gets a tiny
  // nudge, and one that's basically touching the wall gets shoved back hard. each axis is handled
  // independently since a boid could be close to a horizontal border, a vertical one, or both at once
  if (boid.position.x < d)
  {
    force.x += (d - boid.position.x);
  }
  else if (boid.position.x > w - d)
  {
    force.x -= (boid.position.x - (w - d));
  }

  if (boid.position.y < d)
  {
    force.y += (d - boid.position.y);
  }
  else if (boid.position.y > h - d)
  {
    force.y -= (boid.position.y - (h - d));
  }

  // end solution

  return force;
}

bool BoundedAreaRule::drawImguiRuleExtra() {
  ImVec2 displaySize = ImGui::GetIO().DisplaySize;
  float widthWindows = displaySize.x > 0.f ? displaySize.x : 1280.f;
  float heightWindows = displaySize.y > 0.f ? displaySize.y : 800.f;
  bool valueHasChanged = false;
  int minHeightWidth = static_cast<int>(std::min(widthWindows, heightWindows));

  if (ImGui::SliderInt("Desired Distance From Borders", &desiredDistance, 0, minHeightWidth / 3, "%i")) {
    valueHasChanged = true;
  }
  return valueHasChanged;
}

void BoundedAreaRule::drawWorldOverlay(ImDrawList* dl) const {
  ImVec2 displaySize = ImGui::GetIO().DisplaySize;
  float w = displaySize.x > 0.f ? displaySize.x : 1280.f;
  float h = displaySize.y > 0.f ? displaySize.y : 800.f;
  float d = static_cast<float>(desiredDistance);
  ImU32 col = IM_COL32(128, 128, 128, 200);

  dl->AddLine({d, d}, {w - d, d}, col);
  dl->AddLine({w - d, d}, {w - d, h - d}, col);
  dl->AddLine({w - d, h - d}, {d, h - d}, col);
  dl->AddLine({d, h - d}, {d, d}, col);
}
