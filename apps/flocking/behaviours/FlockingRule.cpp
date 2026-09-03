#include "FlockingRule.h"
#include "../utils/ImGuiExtra.h"
#include "imgui.h"
#include <glm/glm.hpp>

//Possilbe fix the member initializers must be listed in declarations order (debugColor,weight, isEnabled), otherwise
//the compiler intializes them int hat order anyway and emits a warning for the mismatch. The orginal list
//was (weight, debugColor, isEnabled) harmless here since none of the iniliazesers depend on eachother, but might be worht cleaning


//FlockingRule::FlockingRule(const FlockingRule& toCopy) : weight(toCopy.weight), debugColor(toCopy.debugColor), isEnabled(toCopy.isEnabled) {}
FlockingRule::FlockingRule(const FlockingRule& toCopy) : debugColor(toCopy.debugColor), weight(toCopy.weight), isEnabled(toCopy.isEnabled) {}

glm::vec2 FlockingRule::computeWeightedForce(const std::vector<BoidView>& neighborhood, const BoidView& boid) {
  if (isEnabled) {
    return getBaseWeightMultiplier() * weight * computeForce(neighborhood, boid);
  }
  return glm::vec2(0.f);
}

bool FlockingRule::drawImguiRule() {
  bool valueHasChanged = false;

  ImGui::SetNextItemOpen(isEnabled, ImGuiCond_Once);
  if (ImGui::TreeNode(getRuleName())) {
    ImguiTooltip(getRuleExplanation());

    if (ImGui::Checkbox("Enabled", &isEnabled)) {
      valueHasChanged = true;
    }

    if (isEnabled) {
      if (ImGui::DragFloat("Weight##", &weight, 0.025f)) {
        valueHasChanged = true;
      }
      ImGui::SameLine();
      HelpMarker("Drag to change the weight's value or CTRL+Click to input a new value.");

      if (drawImguiRuleExtra()) {
        valueHasChanged = true;
      }
    }

    ImGui::TreePop();
  } else {
    ImguiTooltip(getRuleExplanation());
  }

  return valueHasChanged;
}

void FlockingRule::draw(const BoidView& boid, ImDrawList* dl, glm::vec2 cachedForce) const {
  glm::vec2 end = boid.position + cachedForce * 0.1f;
  ImU32 col = IM_COL32(static_cast<int>(debugColor.r * 255), static_cast<int>(debugColor.g * 255), static_cast<int>(debugColor.b * 255), 200);
  dl->AddLine({boid.position.x, boid.position.y}, {end.x, end.y}, col, 1.5f);
}
