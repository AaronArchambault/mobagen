#include "Manager.h"
#include "rules/JohnConway.h"
#include "rules/HexagonGameOfLife.h"
#include "rules/HighLife.h"
#include "rules/Seeds.h"
#include "rules/BriansBrain.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace {
//it is the bonus that maps a neighbor count (0..8) to a blue -> yellow -> red gradient
ImU32 HeatColor(int neighborCount, int maxExpected) {
  float t = maxExpected > 0 ? std::clamp((float)neighborCount / (float)maxExpected, 0.0f, 1.0f) : 0.0f;
  float r, g, b;
  if (t < 0.5f) {
    // blue -> yellow
    float k = t / 0.5f;
    r = k;
    g = k;
    b = 1.0f - k;
  } else {
    // yellow -> red
    float k = (t - 0.5f) / 0.5f;
    r = 1.0f;
    g = 1.0f - k;
    b = 0.0f;
  }
  return IM_COL32((int)(r * 255), (int)(g * 255), (int)(b * 255), 255);
}
}  // namespace

Manager::Manager() {
  world.Resize(sideSize);
  rules.push_back(new HexagonGameOfLife());
  rules.push_back(new JohnConway());
  //it is the bonus extra rule variants
  rules.push_back(new HighLife());
  rules.push_back(new Seeds());
  rules.push_back(new BriansBrain());
}

void Manager::Start() {}

void Manager::OnGui() {
  ImGui::Begin("Settings", nullptr);
  ImGui::Text("%.1fms %.0fFPS | AVG: %.2fms %.1fFPS", ImGui::GetIO().DeltaTime * 1000, 1.0f / ImGui::GetIO().DeltaTime,
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  static auto newSize = sideSize;
  if (ImGui::SliderInt("Side Size", &newSize, 5, 256)) {
    newSize = (newSize / 4) * 4 + 1;
    if (newSize != sideSize) {
      sideSize = newSize;
      world.Resize(newSize);
      generation = 0;
      history.clear();  //it is the bonus and is for/so that old snapshots don't match the new grid size
    }
  }

  ImGui::Text("Generator: %s", rules[ruleId]->GetName().c_str());
  if (ImGui::BeginCombo("##combo", rules[ruleId]->GetName().c_str())) {
    for (int n = 0; n < (int)rules.size(); n++) {
      bool is_selected = (rules[ruleId]->GetName() == rules[n]->GetName());
      if (ImGui::Selectable(rules[n]->GetName().c_str(), is_selected)) {
        ruleId = n;
        clear();
      }
      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  ImGui::Text("Simulation");
  if (ImGui::Button("Step")) {
    isSimulating = false;
    accumulatedTime += ImGui::GetIO().DeltaTime;
    step();
  }
  ImGui::SameLine();
  if (ImGui::Button("Start")) {
    isSimulating = true;
  }

  ImGui::SameLine();
  if (ImGui::Button("Pause")) {
    isSimulating = false;
  }

  //it is the bonus undo or step-back. Disabled (grayed out) when there's nothing to
  //undo, rather than silently doing nothing on click.
  ImGui::SameLine();
  ImGui::BeginDisabled(history.empty());
  if (ImGui::Button("Back")) {
    isSimulating = false;
    popHistory();
  }
  ImGui::EndDisabled();

  ImGui::Text("TimeToNextStep: %.3f", (timeBetweenSteps - accumulatedTime));
  static auto newTime = timeBetweenSteps;
  if (ImGui::SliderFloat("Time Between Steps", &newTime, 0.0001f, 1.0f)) {
    if (newTime != timeBetweenSteps) timeBetweenSteps = newTime;
  }

  if (ImGui::Button("Randomize")) {
    isSimulating = false;
    world.Randomize();
    generation = 0;
    history.clear();  //it is the bonus and is a fresh board invalidates prior undo history
  }
  ImGui::SameLine();
  //it is the bonus reliable, repeatable pattern for demoing/recording
  if (ImGui::Button("Load Glider")) {
    isSimulating = false;
    loadGliderPreset();
    generation = 0;
    history.clear();
  }

  //it is the bonus debug interface
  ImGui::Separator();
  ImGui::Text("Generation: %d", generation);
  ImGui::Text("Population: %d / %d", countAlivePopulation(), sideSize * sideSize);
  ImGui::Checkbox("Debug: Neighbor Heatmap", &showHeatmap);
  if (showHeatmap) {
    ImGui::TextWrapped(
        "Tints every cell by its live-neighbor count (blue = 0, yellow = "
        "mid, red = max) so a wrong CountNeighbors is easy to spot visually. "
        "Alive cells (On, for Brian's Brain) get a white border.");
  }

  ImGui::End();  // end settings

  static glm::ivec2 lastIndexClicked = {INT32_MAX, INT32_MAX};
  
  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) {
    isDraggingOnCanvas = true;
  }

  if (isDraggingOnCanvas && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    auto mousePos = ImGui::GetMousePos();
    glm::ivec2 index;
    if (rules[ruleId]->GetTileSet() == GameOfLifeTileSetEnum::Square) {
      index = mousePositionToIndex(mousePos);
    } else if (rules[ruleId]->GetTileSet() == GameOfLifeTileSetEnum::Hexagon) {
      index = hexPositionToIndex(mousePos);
    }

    //std::cout << "(" << index.x << "," << index.y << ")" << std::endl;

    if (lastIndexClicked != index) {
      lastIndexClicked = index;
      //std::cout << "MatrixPos: (" << index.x << "," << index.y << ")" << std::endl;
      if (index.x >= 0 && index.x < sideSize && index.y >= 0 && index.y < sideSize) {
        world.SetCurrent(index, !world.Get(index));  //to be visible
        world.SetNext(index, !world.Get(index));     //to be used next time
      }
    }
  }
  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    isDraggingOnCanvas = false;
    lastIndexClicked = {INT32_MAX, INT32_MAX};
  }
}

void Manager::OnDraw() {
  if (rules[ruleId]->GetTileSet() == GameOfLifeTileSetEnum::None) {
    std::cout << "your rule should explicitly say which board you want to use";
    return;
  }

  auto* dl = ImGui::GetBackgroundDrawList();
  ImVec2 winSize = ImGui::GetIO().DisplaySize;
  float cx = winSize.x * 0.5f;
  float cy = winSize.y * 0.5f;
  float minDimension = std::min(winSize.x, winSize.y) * 0.99f;
  float squareSide = minDimension / sideSize;
  float sideSideOver2 = sideSize / 2.0f;

  //High-contrast palette vivid live cells, dark dead ones, no borders
  //cells are exactly sized so they tile edge to edge.
  const ImU32 liveFill = IM_COL32(120, 215, 60, 255);
  const ImU32 deadFill = IM_COL32(28, 28, 34, 255);
  const ImU32 aliveBorder = IM_COL32(255, 255, 255, 255);
  //Square/most-rules neighborhoods max out at 8; hex maxes at 6 but that
  //just means the heatmap never reaches the very top of the gradient there.
  const int heatmapMax = 8;

  if (rules[ruleId]->GetTileSet() == GameOfLifeTileSetEnum::Square) {
    for (int l = 0; l < sideSize; l++) {
      for (int c = 0; c < sideSize; c++) {
        Point2D p{c, l};
        bool alive = world.Get(p);
        float rx = std::ceil(cx + (c - sideSideOver2) * squareSide);
        float ry = std::ceil(cy + (l - sideSideOver2) * squareSide);
        ImVec2 topLeft(rx, ry), bottomRight(rx + squareSide, ry + squareSide);
        if (showHeatmap) {
          ImU32 heat = HeatColor(rules[ruleId]->CountNeighbors(world, p), heatmapMax);
          dl->AddRectFilled(topLeft, bottomRight, heat);
          if (alive) dl->AddRect(topLeft, bottomRight, aliveBorder, 0.0f, 0, 2.0f);
        } else {
          dl->AddRectFilled(topLeft, bottomRight, alive ? liveFill : deadFill);
        }
      }
    }
  } else if (rules[ruleId]->GetTileSet() == GameOfLifeTileSetEnum::Hexagon) {
    //True pointy-top hex tiling  vertices at top and bottom, flat edges left and right. Row parity keeps the same odd-row
    //shift direction the mouse picking uses. The circumradius solves so the grid fits the viewport on both axes
    //(computed once in computeHexGeometry() and shared with hexPositionToIndex so draw and click-picking can't drift apart)
    HexGeometry hexGeom = computeHexGeometry();
    const float radius = hexGeom.radius;
    const float width = hexGeom.width;      //flat-to-flat, in-row pitch
    const float rowPitch = hexGeom.rowPitch;  //distance between row centers
    const float startX = hexGeom.startX;
    const float startY = hexGeom.startY;
    for (int l = 0; l < sideSize; l++) {
      float displacement = std::abs(l - (int)sideSideOver2) % 2 == 1 ? width * 0.5f : 0.0f;
      for (int c = 0; c < sideSize; c++) {
        Point2D p{c, l};
        bool alive = world.Get(p);
        float centerX = startX + c * width + displacement;
        float centerY = startY + l * rowPitch;
        ImVec2 points[6];
        for (int v = 0; v < 6; v++) {
          float angle = v * (3.14159265f / 3.0f) + 3.14159265f / 6.0f;  //pointy-top: vertices at 30,90,...,330 degrees
          points[v] = ImVec2(centerX + radius * std::cos(angle), centerY + radius * std::sin(angle));
        }
        if (showHeatmap) {
          ImU32 heat = HeatColor(rules[ruleId]->CountNeighbors(world, p), heatmapMax);
          dl->AddConvexPolyFilled(points, 6, heat);
          if (alive) dl->AddPolyline(points, 6, aliveBorder, ImDrawFlags_Closed, 2.0f);
        } else {
          dl->AddConvexPolyFilled(points, 6, alive ? liveFill : deadFill);
        }
      }
    }
  }
}

void Manager::Update(float deltaTime) {
  if (isSimulating) {
    accumulatedTime += deltaTime;
    if (accumulatedTime > timeBetweenSteps) {
      step();
      accumulatedTime = 0;
    }
  }
}

void Manager::step() {
  pushHistory();  //bonus snapshot before stepping
  rules[ruleId]->Step(world);
  world.SwapBuffers();
  ++generation;  //bonus debug counter
}

Manager::~Manager() {
  for (auto x : rules) delete x;
  rules.clear();
}

void Manager::clear() {
  isSimulating = false;
  world.Resize(sideSize);
  generation = 0;
  history.clear();  //it is the bonus and it switching rules invalidates prior undo history
}

//it is a bonus that is the debug interface helper and it counts live cells in the curent buffer by
//reading through the public World::Get
int Manager::countAlivePopulation() {
  int count = 0;
  for (int y = 0; y < sideSize; ++y)
    for (int x = 0; x < sideSize; ++x)
      if (world.Get({x, y})) ++count;
  return count;
}

//it is the bonuse that loads a Conway Glider near the top-left of the grid
void Manager::loadGliderPreset() {
  world.Resize(sideSize);
  static const glm::ivec2 offsets[5] = {
      {1, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2},
  };
  for (const auto& offset : offsets) {
    world.SetCurrent(offset, true);
    world.SetNext(offset, true);
  }
}

//bonus it is an undo ot step-back and it snapshotting both World channels (main + extra)
//and then it keeps this correct even for the other bonus thing, whose Dying state lives in the
//other placres an undo that only restored the main bit
void Manager::pushHistory() {
  history.emplace_back(world.Snapshot(), world.SnapshotExtra());
  if ((int)history.size() > kMaxHistorySteps) history.erase(history.begin());
}

void Manager::popHistory() {
  if (history.empty()) return;
  auto [mainSnapshot, extraSnapshot] = history.back();
  history.pop_back();
  world.LoadSnapshot(mainSnapshot, extraSnapshot);
  if (generation > 0) --generation;
}

glm::ivec2 Manager::mousePositionToIndex(ImVec2& mousePos) {
  ImVec2 winSize = ImGui::GetIO().DisplaySize;
  float cx = winSize.x * 0.5f;
  float cy = winSize.y * 0.5f;
  float minDimension = std::min(winSize.x, winSize.y) * 0.99f;
  float squareSide = minDimension / sideSize;

  glm::vec2 rel(mousePos.x - cx, mousePos.y - cy);
  rel += glm::vec2(minDimension / 2.0f, minDimension / 2.0f);
  rel /= squareSide;

  return glm::ivec2((int)rel.x, (int)rel.y);
}

Manager::HexGeometry Manager::computeHexGeometry() {
  ImVec2 winSize = ImGui::GetIO().DisplaySize;
  float cx = winSize.x * 0.5f;
  float cy = winSize.y * 0.5f;
  float minDimension = std::min(winSize.x, winSize.y) * 0.99f;
  const float sqrt3 = 1.7320508f;

  HexGeometry g{};
  g.radius = std::min(minDimension / (sqrt3 * (sideSize + 0.5f)), minDimension / (1.5f * (sideSize - 1) + 2.0f));
  g.width = sqrt3 * g.radius;
  g.rowPitch = 1.5f * g.radius;
  g.startX = cx - (g.width * (sideSize + 0.5f)) * 0.5f + g.width * 0.5f;
  g.startY = cy - (g.rowPitch * (sideSize - 1) + 2.0f * g.radius) * 0.5f + g.radius;
  g.sideSideOver2 = sideSize / 2.0f;
  return g;
}

glm::ivec2 Manager::hexPositionToIndex(ImVec2& mousePos) {
  HexGeometry g = computeHexGeometry();
  int l = (int)std::round((mousePos.y - g.startY) / g.rowPitch);
  float displacement = std::abs(l - (int)g.sideSideOver2) % 2 == 1 ? g.width * 0.5f : 0.0f;
  int c = (int)std::round((mousePos.x - displacement - g.startX) / g.width);
  return {c, l};
}
