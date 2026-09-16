#include "PrimExample.h"
#include "../World.h"
#include "Random.h"

namespace {
  //it opens thw wall betwenn two cell that were next to eachother
  void OpenWallBetween(World* w, const Point2D& a, const Point2D& b) {
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
}


bool PrimExample::Step(World* w) {
  if (!initialized)
  {
    Point2D start = {0, 0};
    visited[start.y][start.x] = true;
    for (const auto& n : getVisitables(w, start)) toBeVisited.push_back(n);
    initialized = true;
    return true;
  }

  if (toBeVisited.empty()) return false;  //it is if the maze is complete

  int idx = Random::Range(0, (int)toBeVisited.size() -1);
  Point2D chosen = toBeVisited[idx];
  toBeVisited.erase(toBeVisited.begin() + idx);

  //the same cell can be pushed onto the frontier more than once skip it if it's already carved in
  if (visited[chosen.y][chosen.x]) return true;

  std::vector<Point2D> visitedNeighbors = getVisitedNeighbors(w, chosen);
  if (!visitedNeighbors.empty())
  {
    Point2D linkFrom = visitedNeighbors[Random::Range(0, (int)visitedNeighbors.size() - 1)];
    OpenWallBetween(w, chosen, linkFrom);
  }
  visited[chosen.y][chosen.x] = true;

  for (const auto& n : getVisitables(w, chosen)) toBeVisited.push_back(n);

  return true;

}
void PrimExample::Clear(World* world) {
  toBeVisited.clear();
  visited.clear();
  initialized = false;
}

std::vector<Point2D> PrimExample::getVisitables(World* w, const Point2D& p) {
  std::vector<Point2D> deltas = {Point2D(0, -1), Point2D(0, 1), Point2D(-1, 0), Point2D(1, 0)};  // N, S, W, E
  std::vector<Point2D> visitables;
  auto clearColor = Color32(169.0f / 255.0f, 169.0f / 255.0f, 169.0f / 255.0f, 1.0f);  // dark gray

  // todo: code this
  for (const auto& d : deltas)
  {
    Point2D c(p.x + d.x, p.y + d.y);
    if (c.x < 0 || c.x >= w->GetWidth() || c.y < 0 || c.y >= w->GetHeight()) continue;

    if (visited[c.y][c.x]) continue;

    visitables.push_back(c);
  }

  return visitables;
}

std::vector<Point2D> PrimExample::getVisitedNeighbors(World* w, const Point2D& p) {
  std::vector<Point2D> deltas = {Point2D(0, -1), Point2D(0, 1), Point2D(-1, 0), Point2D(1, 0)};  // N, S, W, E
  std::vector<Point2D> neighbors;

  // todo: code this
  for (const auto& d : deltas)
  {
    Point2D c(p.x + d.x, p.y + d.y);
    if (c.x < 0 || c.x >= w->GetWidth() || c.y < 0 || c.y >= w->GetHeight()) continue;

    if (visited[c.y][c.x]) neighbors.push_back(c);

  }

  return neighbors;
}
