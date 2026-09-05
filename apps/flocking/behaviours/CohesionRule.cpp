#include "CohesionRule.h"
#include <glm/glm.hpp>

glm::vec2 CohesionRule::computeForce(const std::vector<BoidView>& neighborhood, const BoidView& boid) {
  glm::vec2 cohesionForce(0.f);

  // glm::length(vec) returns the length of a vector,
  // glm::normalize(vec) returns the normalized vector (length 1) in the same direction as vec.

  // begin solution
  //no neighbors means that their is no group to be pulled towards, so nothing should really be done
  if (neighborhood.empty())
  {
    return cohesionForce;
  }

  //average every neighbor's position together to find the "middle" of the local group
  glm::vec2 centerOfMass(0.f);
  for (const auto& other : neighborhood)
  {
    centerOfMass += other.position;
  }

  centerOfMass /= static_cast<float>(neighborhood.size());

  //it goes towards the center of mass with the same magnitude and does nto grow with distance
  glm::vec2 toCenter = centerOfMass - boid.position;
  float distance = glm::length(toCenter);

  if (distance > 0.0001f)
  {
    cohesionForce = toCenter / distance;
  }

  // end solution

  return cohesionForce;
}
