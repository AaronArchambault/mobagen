#ifndef LIFE_MANAGER_H
#define LIFE_MANAGER_H

#include "imgui.h"
#include "RuleBase.h"
#include "World.h"

#include <glm/glm.hpp>
#include <vector>

class Manager {
private:
  int sideSize = 13;
  World world;
  bool isSimulating = false;
  float accumulatedTime = 0;
  float timeBetweenSteps = 0.2f;
  void step();
  void clear();
  std::vector<RuleBase*> rules;
  int ruleId = 0;
  glm::ivec2 mousePositionToIndex(ImVec2& pos);
  struct HexGeometry {
    float radius;
    float width;      //flat-to-flat, in-row pitch
    float rowPitch;   //distance between row centers
    float startX;
    float startY;
    float sideSideOver2;
  };
  HexGeometry computeHexGeometry();
  glm::ivec2 hexPositionToIndex(ImVec2& pos);

  bool isDraggingOnCanvas = false;

  //bonus thigns for gerneation count and live population and other things and onk-click for a well known patterns for reapaltade demos
  int generation = 0;
  int countAlivePopulation();
  void loadGliderPreset();
  bool showHeatmap = false;
  static constexpr int kMaxHistorySteps = 200;
  std::vector<std::pair<::std::vector<bool>, std::vector<bool>>> history;
  void pushHistory();
  void popHistory();

public:
  Manager();
  ~Manager();

  void Start();
  void OnGui();
  void OnDraw();
  void Update(float deltaTime);
};

#endif  // LIFE_MANAGER_H
