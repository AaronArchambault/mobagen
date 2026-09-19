#include "MazeGame.h"
#include "World.h"
#include <queue>
#include <cstdlib>
#include <algorithm>
#include <climits>

#include "imgui.h"

//redblobgames.com/pathfinding/a-star/introduction.html
//github.com/ocornut/imgui
//en.cppreference.com/w/cpp/container/priority_queue
//redblobgames.com/pathfinding/a-star/introduction.html#heuristics
//en.wikipedia.org/wiki/A*_search_algorithm
//https://stackoverflow.com/questions/18815221/what-is-decltype-and-how-is-it-used
//https://en.wikipedia.org/wiki/Decltype
//https://www.geeksforgeeks.org/dsa/a-search-algorithm/
//https://theory.stanford.edu/~amitp/GameProgramming/AStarComparison.html
//https://stackoverflow.com/questions/1332466/how-does-dijkstras-algorithm-and-a-star-compare

namespace {
//walls always get drawn in pure white by World::OnDraw which i did not touch, so the floor needs
//to be dark for them to actually show up better
const Color32 kFloorColor = {0.13f, 0.13f, 0.16f, 1.0f};    //near black, it is the unvisited floor
const Color32 kVisitedColor = {0.30f, 0.30f, 0.36f, 1.0f};  //a lighter trail, it is cells the player already went through
const Color32 kPlayerColor = {0.25f, 0.55f, 1.0f, 1.0f};    //bright blue, this one is you
const Color32 kEnemyColor = {1.0f, 0.15f, 0.15f, 1.0f};     //vivid red, this one is the enemy
const Color32 kGoalColor = {1.0f, 0.82f, 0.0f, 1.0f};       //gold, this one is the goal

int ManhattanDistance(const Point2D& a, const Point2D& b) { return std::abs(a.x - b.x) + std::abs(a.y - b.y); }
}  //namespace

void MazeGame::PaintCell(World& world, const Point2D& formalCell, bool isGoalCell) {
  world.SetNodeColor(world.ToWorldCoords(formalCell), isGoalCell ? kGoalColor : kFloorColor);
}

void MazeGame::PaintPlayerTrail(World& world, const Point2D& formalCell) {
  bool isGoal = (formalCell.x == goal.x && formalCell.y == goal.y);
  world.SetNodeColor(world.ToWorldCoords(formalCell), isGoal ? kGoalColor : kVisitedColor);
}

void MazeGame::ResetBoardColors(World& world) {
  for (int y = 0; y < world.GetHeight(); y++)
  {
    for (int x = 0; x < world.GetWidth(); x++)
    {
      Point2D cell(x, y);
      bool isGoal = (cell.x == goal.x && cell.y == goal.y);
      PaintCell(world, cell, isGoal);
    }
  }
}

void MazeGame::StartGame(World& world) {
  lastWidth = world.GetWidth();
  lastHeight = world.GetHeight();

  player = Point2D(0, 0);
  goal = Point2D(lastWidth - 1, lastHeight - 1);
  enemy = Point2D(lastWidth - 1, 0);  //it is the opposite corner from the goal, same row as where the player starts

  hasPendingMove = false;
  pendingDx = pendingDy = 0;
  enemyMoveCooldown = 0.0f;
  state = State::Playing;

  ResetBoardColors(world);
  world.SetNodeColor(world.ToWorldCoords(player), kPlayerColor);
  world.SetNodeColor(world.ToWorldCoords(enemy), kEnemyColor);

  //this is just a check, if the goal is not reachable from where the player starts
  mazeLooksIncomplete = FindPathAStar(world, player, goal).empty();
}

void MazeGame::RequestMove(int dx, int dy) {
  hasPendingMove = true;
  pendingDx = dx;
  pendingDy = dy;
}

bool MazeGame::TryMovePlayer(World& world, int dx, int dy) {
  Point2D worldPlayer = world.ToWorldCoords(player);
  bool open = false;
  if (dx == 0 && dy == -1)
  {
    open = !world.GetNorth(worldPlayer);
  }
  else if (dx == 1 && dy == 0)
  {
    open = !world.GetEast(worldPlayer);
  }
  else if (dx == 0 && dy == 1)
  {
    open = !world.GetSouth(worldPlayer);
  }
  else if (dx == -1 && dy == 0)
  {
    open = !world.GetWest(worldPlayer);
  }
  if (!open) return false;

  int nx = player.x + dx, ny = player.y + dy;
  if (nx < 0 || nx >= world.GetWidth() || ny < 0 || ny >= world.GetHeight()) return false;

  PaintPlayerTrail(world, player);
  player = Point2D(nx, ny);
  return true;
}

std::vector<Point2D> MazeGame::FindPathAStar(World& world, const Point2D& start, const Point2D& target) {
  int width = world.GetWidth(), height = world.GetHeight();
  std::vector<std::vector<int>> gScore(height, std::vector<int>(width, INT_MAX));
  std::vector<std::vector<Point2D>> cameFrom(height, std::vector<Point2D>(width, Point2D(-1, -1)));
  std::vector<std::vector<bool>> closed(height, std::vector<bool>(width, false));

  using QueueItem = std::pair<int, Point2D>;
  auto cmp = [](const QueueItem& a, const QueueItem& b) { return a.first > b.first; };
  std::priority_queue<QueueItem, std::vector<QueueItem>, decltype(cmp)> open(cmp);

  gScore[start.y][start.x] = 0;
  open.push({ManhattanDistance(start, target), start});

  while (!open.empty())
  {
    Point2D current = open.top().second;
    open.pop();
    if (closed[current.y][current.x]) continue;
    closed[current.y][current.x] = true;
    if (current.x == target.x && current.y == target.y) break;

    Point2D worldCurrent = world.ToWorldCoords(current);
    struct {
      int dx, dy;
      bool open;
    } dirs[4] = {
        {0, -1, !world.GetNorth(worldCurrent)},
        {1, 0, !world.GetEast(worldCurrent)},
        {0, 1, !world.GetSouth(worldCurrent)},
        {-1, 0, !world.GetWest(worldCurrent)},
    };

    for (auto& d : dirs)
    {
      if (!d.open) continue;
      int nx = current.x + d.dx, ny = current.y + d.dy;
      if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
      int tentative = gScore[current.y][current.x] + 1;
      if (tentative < gScore[ny][nx])
      {
        gScore[ny][nx] = tentative;
        cameFrom[ny][nx] = current;
        open.push({tentative + ManhattanDistance(Point2D(nx, ny), target), Point2D(nx, ny)});
      }
    }
  }

  std::vector<Point2D> path;
  if (gScore[target.y][target.x] == INT_MAX) return path;  //it means the target is unreachable

  Point2D cur = target;
  path.push_back(cur);
  while (!(cur.x == start.x && cur.y == start.y))
  {
    cur = cameFrom[cur.y][cur.x];
    path.push_back(cur);
  }
  std::reverse(path.begin(), path.end());
  return path;
}

void MazeGame::StepEnemyTowardPlayer(World& world) {
  std::vector<Point2D> path = FindPathAStar(world, enemy, player);
  if (path.size() < 2) return;  //it means there is no path, or it is already on the players cell

  PaintCell(world, enemy, enemy.x == goal.x && enemy.y == goal.y);
  enemy = path[1];
  world.SetNodeColor(world.ToWorldCoords(enemy), kEnemyColor);
}

void MazeGame::Update(float dt, World& world) {
  if (!active || state != State::Playing) return;

  if (world.GetWidth() != lastWidth || world.GetHeight() != lastHeight)
  {
    StartGame(world);
    return;
  }

  //it applies the pending move right away, no cooldown, since RequestMove only ever fires once
  //per actual key press now, one tap always moves exactly one tile
  if (hasPendingMove)
  {
    if (TryMovePlayer(world, pendingDx, pendingDy))
    {
      world.SetNodeColor(world.ToWorldCoords(player), kPlayerColor);
    }
    hasPendingMove = false;
  }

  if (player.x == enemy.x && player.y == enemy.y)
  {
    state = State::Caught;
    return;
  }
  if (player.x == goal.x && player.y == goal.y)
  {
    state = State::Won;
    return;
  }

  enemyMoveCooldown -= dt;
  if (enemyMoveCooldown <= 0.0f)
  {
    StepEnemyTowardPlayer(world);
    enemyMoveCooldown = enemyMoveInterval;
  }

  if (player.x == enemy.x && player.y == enemy.y)
  {
    state = State::Caught;
  }
}

void MazeGame::OnGui(World& world) {
  ImGui::Begin("Play Maze", nullptr);

  bool wasActive = active;
  ImGui::Checkbox("Enable Play Mode", &active);
  if (active && !wasActive) StartGame(world);  //it just got turned on so it initializes right away

  if (!active)
  {
    ImGui::TextWrapped("Turn this on to play through the currently generated maze. Generator controls in Settings are paused while playing.");
    ImGui::End();
    return;
  }

  ImGui::Text("Blue = you. Red = enemy. Gold = goal. Faint trail = where you've been.");
  ImGui::Text("Move: WASD or Arrow Keys, one tap moves one tile");
  ImGui::SliderFloat("Enemy speed (s/move)", &enemyMoveInterval, 0.1f, 1.0f);

  if (mazeLooksIncomplete)
  {
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.2f, 1.0f), "warning, the goal isn't reachable yet, the maze\n"
                                                         "probably hasn't finished generating, use start or step\n"
                                                         "in settings to finish it, then click restart below");
  }

  switch (state)
  {
    case State::Playing:
      ImGui::Text("Status: Playing");
      break;
    case State::Won:
      ImGui::TextColored(ImVec4(kGoalColor.r, kGoalColor.g, kGoalColor.b, 1.0f), "You reached the goal! You win.");
      break;
    case State::Caught:
      ImGui::TextColored(ImVec4(kEnemyColor.r, kEnemyColor.g, kEnemyColor.b, 1.0f), "Caught! Game over.");
      break;
    case State::Idle:
      ImGui::Text("Status: Idle");
      break;
  }

  if (ImGui::Button("Restart")) StartGame(world);

  //it uses IsKeyPressed with repeat set to false instead of IsKeyDown, so each key only fires once
  //on the frame it actually gets pressed, holding it down does not send more moves, this is what
  //makes one tap equal exactly one tile, each direction gets checked on its own first so a press
  //never gets missed just because another direction happened to win the priority that frame
  if (state == State::Playing)
  {
    bool pressedUp = ImGui::IsKeyPressed(ImGuiKey_W, false) || ImGui::IsKeyPressed(ImGuiKey_UpArrow, false);
    bool pressedDown = ImGui::IsKeyPressed(ImGuiKey_S, false) || ImGui::IsKeyPressed(ImGuiKey_DownArrow, false);
    bool pressedLeft = ImGui::IsKeyPressed(ImGuiKey_A, false) || ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false);
    bool pressedRight = ImGui::IsKeyPressed(ImGuiKey_D, false) || ImGui::IsKeyPressed(ImGuiKey_RightArrow, false);

    if (pressedUp)
    {
      RequestMove(0, -1);
    }
    else if (pressedDown)
    {
      RequestMove(0, 1);
    }
    else if (pressedLeft)
    {
      RequestMove(-1, 0);
    }
    else if (pressedRight)
    {
      RequestMove(1, 0);
    }
  }

  ImGui::End();
}
