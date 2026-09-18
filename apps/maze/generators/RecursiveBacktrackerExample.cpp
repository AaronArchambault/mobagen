#include "../World.h"
#include "../SeededRandom.h"
#include "RecursiveBacktrackerExample.h"
#include <climits>

//this is the recursive backtracker, it works in formal units so 0,0 is the top left cell, x grows
//right and y grows down, the caller seeds SeededRandom before the first Step and every decision
//consumes the seed in order so that the maze always comes out the same way for the same seed
//
//what it does each Step, on the cell at the top of the path stack
//1 it marks it visited
//2 it lists its visitable unvisited neighbors going clockwise starting from the top, that part is
//getVisitables
//3 if there are none that is a dead end so it backtracks by popping the stack, empty stack means done
//4 if there is exactly one it just moves there, no random number spent
//5 if there is two or more it consumes one SeededRandom::next() and picks next() % visitableCount
//6 moving opens the wall between the two cells using SetNorth/SetEast/SetSouth/SetWest with false

namespace {
const Color32 kPathColor = {1.0f, 1.0f, 1.0f, 1.0f};     //white, it is a cell that settled into the path
const Color32 kCursorColor = {1.0f, 0.6f, 0.0f, 1.0f};   //orange, it is the current active cell
}  //namespace

void RecursiveBacktrackerExample::Clear(World* world) {
  visited.clear();
  stack.clear();
  stack.push_back({0, 0});
}

bool RecursiveBacktrackerExample::Step(World* w) {
  if (stack.empty()) return false;

  Point2D current = stack.back();
  visited[current.y][current.x] = true;
  w->SetNodeColor(w->ToWorldCoords(current), kPathColor);  //it settles this cell into the maze

  std::vector<Point2D> visitables = getVisitables(w, current);

  if (visitables.empty())
  {
    //it hit a dead end so it backtracks
    stack.pop_back();
  }
  else
  {
    Point2D next = visitables[0];
    if (visitables.size() > 1)
    {
      next = visitables[SeededRandom::next() % visitables.size()];
    }

    Point2D worldCurrent = w->ToWorldCoords(current);
    if (next.x == current.x && next.y == current.y - 1)
    {
      w->SetNorth(worldCurrent, false);
    }
    else if (next.x == current.x + 1 && next.y == current.y)
    {
      w->SetEast(worldCurrent, false);
    }
    else if (next.x == current.x && next.y == current.y + 1)
    {
      w->SetSouth(worldCurrent, false);
    }
    else if (next.x == current.x - 1 && next.y == current.y)
    {
      w->SetWest(worldCurrent, false);
    }

    stack.push_back(next);
  }

  if (!stack.empty())
  {
    w->SetNodeColor(w->ToWorldCoords(stack.back()), kCursorColor);  //it highlights the new active cell
  }

  return !stack.empty();
}

std::vector<Point2D> RecursiveBacktrackerExample::getVisitables(World* w, const Point2D& formalPoint) {
  //the candidates go clockwise starting from the top, up right down left
  std::vector<Point2D> deltas = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
  std::vector<Point2D> visitables;

  for (const auto& delta : deltas)
  {
    Point2D candidate = {formalPoint.x + delta.x, formalPoint.y + delta.y};
    if (candidate.x < 0 || candidate.x >= w->GetWidth() || candidate.y < 0 || candidate.y >= w->GetHeight()) continue;
    if (visited[candidate.y][candidate.x]) continue;
    visitables.push_back(candidate);
  }

  return visitables;
}
