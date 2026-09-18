#include "HuntAndKillExample.h"
#include "../World.h"
#include "Random.h"
#include <climits>

namespace {
void OpenWallsBetween(World* w, const Point2D& a, const Point2D& b) {
  Point2D worldA = w->ToWorldCoords(a);
  if (b.x == a.x && b.y == a.y - 1)
  {
    w->SetNorth(worldA, false);
  }
  else if (b.x == a.x + 1 && b.y == a.y)
  {
    w->SetEast(worldA, false);
  }
  else if (b.x == a.x && b.y == a.y + 1)
  {
    w->SetSouth(worldA, false);
  }
  else if (b.x == a.x - 1 && b.y == a.y)
  {
    w->SetWest(worldA, false);
  }
}

const Color32 kPathColor = {1.0f, 1.0f, 1.0f, 1.0f};        //white, it is a cell that settled into the path
const Color32 kWalkCursorColor = {1.0f, 0.6f, 0.0f, 1.0f};  //orange, it is the normal walk cursor
const Color32 kHuntCursorColor = {1.0f, 0.0f, 1.0f, 1.0f};  //magenta, it means a hunt jump just happened here
}  //namespace

bool HuntAndKillExample::Step(World* w) {
  //it uses stack to hold just one cursor cell, that being where we currently are in the walk
  if (stack.empty()) {
    Point2D start = randomStartPoint(w);
    if (start.x == INT_MAX) return false;  //it means there are no unvisited cells left so the maze is done
    visited[start.y][start.x] = true;
    stack.push_back(start);
    w->SetNodeColor(w->ToWorldCoords(start), kWalkCursorColor);
    return true;
  }

  Point2D current = stack.back();
  std::vector<Point2D> visitables = getVisitables(w, current);

  if (!visitables.empty())
  {
    //it does a random walk, it keeps carving forward while there is still somewhere unvisited to go
    Point2D next = visitables[Random::Range(0, (int)visitables.size() - 1)];
    OpenWallsBetween(w, current, next);
    visited[next.y][next.x] = true;
    w->SetNodeColor(w->ToWorldCoords(current), kPathColor);  //it settles the cell we are leaving
    stack.back() = next;
    w->SetNodeColor(w->ToWorldCoords(next), kWalkCursorColor);  //it highlights the new cursor
    return true;
  }

  //it is stuck so it settles this cell, then it hunts for the first unvisited cell that borders a visited one
  w->SetNodeColor(w->ToWorldCoords(current), kPathColor);

  Point2D huntCell = randomStartPoint(w);
  if (huntCell.x == INT_MAX)
  {
    stack.clear();
    return false;  //it means the maze is done
  }

  std::vector<Point2D> visitedNeighbors = getVisitedNeighbors(w, huntCell);
  //this is never empty, since it scans top left to bottom right, any unvisited cell it lands on
  //has to border an already visited cell that came earlier in the scan
  Point2D linkTo = visitedNeighbors[Random::Range(0, (int)visitedNeighbors.size() - 1)];
  OpenWallsBetween(w, huntCell, linkTo);
  visited[huntCell.y][huntCell.x] = true;
  stack.back() = huntCell;
  w->SetNodeColor(w->ToWorldCoords(huntCell), kHuntCursorColor);  //it uses a different color here since a jump just happened

  return true;
}
void HuntAndKillExample::Clear(World* world) {
  visited.clear();
  stack.clear();

  for (int i = 0; i < world->GetHeight(); i++)
  {
    for (int j = 0; j < world->GetWidth(); j++)
    {
      visited[i][j] = false;
    }
  }
}
Point2D HuntAndKillExample::randomStartPoint(World* world) {
  for (int y = 0; y < world->GetHeight(); y++)
    for (int x = 0; x < world->GetWidth(); x++)
      if (!visited[y][x]) return {x, y};
  return {INT_MAX, INT_MAX};
}

std::vector<Point2D> HuntAndKillExample::getVisitables(World* w, const Point2D& p) {
  std::vector<Point2D> deltas = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};  //it goes north east south west
  std::vector<Point2D> visitables;

  for (const auto& d : deltas)
  {
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

  for (const auto& d : deltas)
  {
    Point2D c = {p.x + d.x, p.y + d.y};
    if (c.x < 0 || c.x >= w->GetWidth() || c.y < 0 || c.y >= w->GetHeight()) continue;
    if (visited[c.y][c.x]) neighbors.push_back(c);
  }

  return neighbors;
}
