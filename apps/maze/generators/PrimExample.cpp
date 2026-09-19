#include "PrimExample.h"
#include "../World.h"
#include "Random.h"

namespace {
void OpenWallBetween(World* w, const Point2D& a, const Point2D& b) {
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

const Color32 kPathColor = {1.0f, 1.0f, 1.0f, 1.0f};       //white, it means carved into the maze
const Color32 kFrontierColor = {0.3f, 0.6f, 1.0f, 1.0f};   //light blue, it is on the frontier but not carved in yet

//it turns an x y into a single index into the flat visited vector, y times width plus x
int VisitedIndex(World* w, int x, int y) { return y * w->GetWidth() + x; }
}  //namespace

bool PrimExample::Step(World* w) {
  if (!initialized)
  {
    Point2D start = {0, 0};
    visited[VisitedIndex(w, start.x, start.y)] = true;
    w->SetNodeColor(w->ToWorldCoords(start), kPathColor);
    for (const auto& n : getVisitables(w, start))
    {
      toBeVisited.push_back(n);
      w->SetNodeColor(w->ToWorldCoords(n), kFrontierColor);
    }
    initialized = true;
    return true;
  }

  if (toBeVisited.empty()) return false;  //it means the maze is done

  int idx = Random::Range(0, (int)toBeVisited.size() - 1);
  Point2D chosen = toBeVisited[idx];
  //it swaps the chosen cell with the last one and pops the back instead of erasing from the
  //middle, order in this list never mattered anyway since it was picked randomly, so this turns
  //an O(n) shift of the whole rest of the vector into an O(1) removal
  toBeVisited[idx] = toBeVisited.back();
  toBeVisited.pop_back();

  //the same cell can get pushed onto the frontier more than once, since it can get reached from two
  //different visited neighbors, so it skips it if it is already carved in
  if (visited[VisitedIndex(w, chosen.x, chosen.y)]) return true;

  std::vector<Point2D> visitedNeighbors = getVisitedNeighbors(w, chosen);
  if (!visitedNeighbors.empty()) {
    Point2D linkFrom = visitedNeighbors[Random::Range(0, (int)visitedNeighbors.size() - 1)];
    OpenWallBetween(w, chosen, linkFrom);
  }
  visited[VisitedIndex(w, chosen.x, chosen.y)] = true;
  w->SetNodeColor(w->ToWorldCoords(chosen), kPathColor);  //it moves this cell from frontier to settled

  for (const auto& n : getVisitables(w, chosen))
  {
    toBeVisited.push_back(n);
    w->SetNodeColor(w->ToWorldCoords(n), kFrontierColor);
  }

  return true;
}
void PrimExample::Clear(World* world) {
  toBeVisited.clear();
  //it resizes to the current maze size and resets every cell back to unvisited, one allocation
  //here instead of a bunch of tiny node allocations every time a cell gets touched
  visited.assign((size_t)world->GetWidth() * world->GetHeight(), false);
  initialized = false;
}

std::vector<Point2D> PrimExample::getVisitables(World* w, const Point2D& p) {
  std::vector<Point2D> deltas = {Point2D(0, -1), Point2D(0, 1), Point2D(-1, 0), Point2D(1, 0)};  //it goes north south west east
  std::vector<Point2D> visitables;
  visitables.reserve(4);  //it never holds more than 4, so this skips the vector's internal regrows

  for (const auto& d : deltas)
  {
    Point2D c(p.x + d.x, p.y + d.y);
    if (c.x < 0 || c.x >= w->GetWidth() || c.y < 0 || c.y >= w->GetHeight()) continue;
    if (visited[VisitedIndex(w, c.x, c.y)]) continue;
    visitables.push_back(c);
  }

  return visitables;
}

std::vector<Point2D> PrimExample::getVisitedNeighbors(World* w, const Point2D& p) {
  std::vector<Point2D> deltas = {Point2D(0, -1), Point2D(0, 1), Point2D(-1, 0), Point2D(1, 0)};  //it goes north south west east
  std::vector<Point2D> neighbors;
  neighbors.reserve(4);  //it never holds more than 4, so this skips the vector's internal regrows

  for (const auto& d : deltas)
  {
    Point2D c(p.x + d.x, p.y + d.y);
    if (c.x < 0 || c.x >= w->GetWidth() || c.y < 0 || c.y >= w->GetHeight()) continue;
    if (visited[VisitedIndex(w, c.x, c.y)]) neighbors.push_back(c);
  }

  return neighbors;
}
