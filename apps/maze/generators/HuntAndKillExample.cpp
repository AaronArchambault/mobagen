#include "HuntAndKillExample.h"
#include "../World.h"
#include "Random.h"
#include <climits>

namespace {
void OpenWallsBetween(World* w, const Point2D& a, const Point2D& b) {
  Point2D worldA = w->ToWorldCoords(a);
  if (b.x == a.x && b.y == a.y - 1) {
    w->SetNorth(worldA, false);
  } else if (b.x == a.x + 1 && b.y == a.y) {
    w->SetEast(worldA, false);
  } else if (b.x == a.x && b.y == a.y + 1) {
    w->SetSouth(worldA, false);
  } else if (b.x == a.x - 1 && b.y == a.y) {
    w->SetWest(worldA, false);
  }
}

const Color32 kPathColor = {1.0f, 1.0f, 1.0f, 1.0f};        //white: settled into the path
const Color32 kWalkCursorColor = {1.0f, 0.6f, 0.0f, 1.0f};  //orange: normal random-walk cursor
const Color32 kHuntCursorColor = {1.0f, 0.0f, 1.0f, 1.0f};  //magenta: a hunt jump just happened here
}  // namespace

bool HuntAndKillExample::Step(World* w) {
  //stack holds a single "cursor" cell: our current position in the walk.
  if (stack.empty()) {
    Point2D start = randomStartPoint(w);
    if (start.x == INT_MAX) return false;  //no unvisited cells left: maze complete
    visited[start.y][start.x] = true;
    stack.push_back(start);
    w->SetNodeColor(w->ToWorldCoords(start), kWalkCursorColor);
    return true;
  }

  Point2D current = stack.back();
  std::vector<Point2D> visitables = getVisitables(w, current);

  if (!visitables.empty()) {
    //it is the random walk and it keep carving forward while there's somewhere unvisited to go
    Point2D next = visitables[Random::Range(0, (int)visitables.size() - 1)];
    OpenWallsBetween(w, current, next);
    visited[next.y][next.x] = true;
    w->SetNodeColor(w->ToWorldCoords(current), kPathColor);  //it settle the cell we're leaving
    stack.back() = next;
    w->SetNodeColor(w->ToWorldCoords(next), kWalkCursorColor);  //it highlight the new cursor
    return true;
  }

  // settle this cell, then hunt for the first unvisited cell that borders a visited one
  w->SetNodeColor(w->ToWorldCoords(current), kPathColor);

  Point2D huntCell = randomStartPoint(w);
  if (huntCell.x == INT_MAX) {
    stack.clear();
    return false;  // maze complete
  }

  std::vector<Point2D> visitedNeighbors = getVisitedNeighbors(w, huntCell);
  //it is so that it is a guaranteed non-empty and it scanning top-left to bottom-right, and any unvisited
  //cell it land on must border an already-visited cell earlier in the scan
  Point2D linkTo = visitedNeighbors[Random::Range(0, (int)visitedNeighbors.size() - 1)];
  OpenWallsBetween(w, huntCell, linkTo);
  visited[huntCell.y][huntCell.x] = true;
  stack.back() = huntCell;
  w->SetNodeColor(w->ToWorldCoords(huntCell), kHuntCursorColor);  //distinct color: a discontinuous jump happened

  return true;
}
void HuntAndKillExample::Clear(World* world) {
  visited.clear();
  stack.clear();

  for (int i = 0; i < world->GetHeight(); i++) {
    for (int j = 0; j < world->GetWidth(); j++) {
      visited[i][j] = false;
    }
  }
}
Point2D HuntAndKillExample::randomStartPoint(World* world) {
  // Todo: improve this if you want
  for (int y = 0; y < world->GetHeight(); y++)
    for (int x = 0; x < world->GetWidth(); x++)
      if (!visited[y][x]) return {x, y};
  return {INT_MAX, INT_MAX};
}

std::vector<Point2D> HuntAndKillExample::getVisitables(World* w, const Point2D& p) {
  std::vector<Point2D> deltas = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};  // N, E, S, W
  std::vector<Point2D> visitables;

  for (const auto& d : deltas) {
    Point2D c = {p.x + d.x, p.y + d.y};
    if (c.x < 0 || c.x >= w->GetWidth() || c.y < 0 || c.y >= w->GetHeight()) continue;
    if (visited[c.y][c.x]) continue;
    visitables.push_back(c);
  }

  return visitables;
}
std::vector<Point2D> HuntAndKillExample::getVisitedNeighbors(World* w, const Point2D& p) {
  std::vector<Point2D> deltas = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};
  std::vector<Point2D> neighbors;

  for (const auto& d : deltas) {
    Point2D c = {p.x + d.x, p.y + d.y};
    if (c.x < 0 || c.x >= w->GetWidth() || c.y < 0 || c.y >= w->GetHeight()) continue;
    if (visited[c.y][c.x]) neighbors.push_back(c);
  }

  return neighbors;
}
