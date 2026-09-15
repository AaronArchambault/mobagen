#include "HuntAndKillExample.h"
#include "../World.h"
#include "Random.h"
#include <climits>

namespace {
  //it opens the walls between two prevoisly togerther cells of a and b
  void OpenWallsBetween(World* w, const Point2D& a, const Point2D& b)
  {
    Point2D worldA = w->ToWorldCoords(a);
    if (b.x == a.x && b.y == a.y -1)
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
}


bool HuntAndKillExample::Step(World* w) {
  // todo: code this
  //it/the stack holds a single "cursor"/cell with its current postion in the walk/path
  if (stack.empty())
  {
    Point2D start = randomStartPoint(w);
    if (start.x == INT_MAX) return false; //it does this when no unvisited cells are left so the maze is done

    visited[start.y][start.x] = true;
    stack.push_back(start);
    return true;
  }

  Point2D current = stack.back();
  std::vector<Point2D> visitables = getVisitables(w, current);
  if (!visitables.empty())
  {
    //it picks a random walk/ it keeps moving forward while there is something that is unvisited
    Point2D next = visitables[Random::Range(0, int(visitables.size()))];
    OpenWallsBetween(w, current, next);
    visited[next.y][next.x] = true;
    stack.back() = next;
    return true;
  }
  //it is for the stuck/ for the hunt for the first unvisited cell that borders a visited one
  Point2D huntCell = randomStartPoint(w);
  if (huntCell.x == INT_MAX)
  {
    stack.clear();
    return false; //it means that the maze is done
  }

  std::vector<Point2D> visitedNeighbors = getVisitedNeighbors(w, huntCell);
  //it makes it so that there are no non-empty one by scaning top-left to bottom right and noting any unvisited cells that are laned on and that must border an
  //alrady visited cell that was in the eilier porcess
  Point2D linkTo = visitedNeighbors[Random::Range(0, int(visitedNeighbors.size()))];
  OpenWallsBetween(w, huntCell, linkTo);
  visited[huntCell.y][huntCell.x] = true;
  stack.back() = huntCell;
  return true;
  //return false;
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
  // todo: code this
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

  // todo: code this
  for (const auto& d : deltas)
  {
    Point2D c = {p.x + d.x, p.y + d.y};
    if (c.x < 0 || c.x >= w->GetWidth() || c.y < 0 || c.y >= w->GetHeight()) continue;

    if (visited[c.y][c.x]) neighbors.push_back(c);
  }


  return neighbors;
}
