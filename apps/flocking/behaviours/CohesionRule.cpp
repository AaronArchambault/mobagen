#include "CohesionRule.h"
#include <glm/glm.hpp>

glm::vec2 CohesionRule::computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid) {
  glm::vec2 cohesionForce(0.f);

  // glm::length(vec) returns the length of a vector,
  // glm::normalize(vec) returns the normalized vector (length 1) in the same direction as vec.

  // begin solution
  if (neighborhood.empty())
  {
    return cohesionForce;
  }

  glm::vec2 centerOfMass(0.f);
  for (const auto& other : neighborhood)
  {
    centerOfMass += other.position;
  }

  centerOfMass /= static_cast<float>(neighborhood.size());

  //the vector pointing form the boid to the center of mass its own magnitude grows straight with around the distance to it center of mass
  cohesionForce = centerOfMass - boid.position;


  // end solution

  return cohesionForce;
}
