#include "WindRule.h"
#include "imgui.h"
#include <cmath>

glm::vec2 WindRule::computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid) {
  // std::cos and std::sin return the cosine and sine of an angle in radians, respectively.
  // windAngle is the angle of the wind direction in degrees, so we need to convert it to radians by multiplying it by (pi / 180).
  
  // begin solution
  //this one completely ignores neighborhood and boid wind is the same constant push
  // in the same direction for every single boid, every frame, no matter where they are
  // or what's around them
  return glm::vec2(std::cos(windAngle), std::sin(windAngle));
 // return glm::vec2(0,0);
  // end solution
}

bool WindRule::drawImguiRuleExtra() {
  bool valueHasChanged = false;
  if (ImGui::SliderAngle("Wind Direction", &windAngle, 0)) {
    valueHasChanged = true;
  }
  return valueHasChanged;
}
