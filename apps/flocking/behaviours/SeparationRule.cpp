#include "SeparationRule.h"
#include "imgui.h"
#include <glm/glm.hpp>

glm::vec2 SeparationRule::computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid) {
  glm::vec2 separatingForce(0.f);

  // the header have the desiredMinimalDistance member variable, which is the distance that the boids should try to maintain from each other.
  // glm::length(vec) returns the length of a vector,
  // glm::normalize(vec) returns the normalized vector (length 1) in the same direction as vec.
  // multiply by (desiredMinimalDistance / distance) is the proportionality factor that makes the force stronger when the boids are closer together, and weaker when they are farther apart.

  // begin solution
  //it goes through every neighbor and push away from each one and then add them all up
  //that way a boid boxed in on multiple sides gets a combined push out, not just a reaction to one
for (const auto& other : neighborhood)
{
  glm::vec2 awayFromOther = boid.position - other.position;
  float distance = glm::length(awayFromOther);


  if (distance > 0.0001f)
  {
    //it is dividing by distance here and it normalizes the direction and is the
    //"1/d" half of the inverse-square falloff, since it is multiply by (desiredMinimalDistance/distance)
    //again right after and that is so the closer a neighbor is, the stronger this whole thing gets
    glm::vec2 direction = awayFromOther / distance;
    separatingForce += direction * (desiredMinimalDistance / distance);
  }
}

  // end solution

  return separatingForce;
}

bool SeparationRule::drawImguiRuleExtra() {
  bool valueHasChanged = false;
  if (ImGui::DragFloat("Desired Separation", &desiredMinimalDistance, 0.05f)) {
    valueHasChanged = true;
  }
  return valueHasChanged;
}
