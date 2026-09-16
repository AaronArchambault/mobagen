#include "../World.h"
#include "../SeededRandom.h"
#include "RecursiveBacktrackerExample.h"
#include <climits>

// Recursive backtracker, in FORMAL units: (0, 0) is the top-left cell, x grows
// right, y grows down. The caller seeds SeededRandom before the first Step;
// every decision consumes the seed in order, so the maze is deterministic.
//
// Procedure per Step, on the cell at the top of the path stack:
//   1. mark it visited;
//   2. list its visitable (unvisited) neighbors in clockwise order starting
//      from the top: UP, RIGHT, DOWN, LEFT (getVisitables does this);
//   3. none        -> dead end: pop the stack (backtrack). Empty stack = done;
//   4. exactly one -> move to it, do not consume a random number;
//   5. two or more -> consume SeededRandom::next() and pick
//      next() % visitableCount;
//   6. moving opens the wall between the two cells
//      (World::SetNorth/SetEast/SetSouth/SetWest with false).

namespace {
const Color32 kPathColor = {1.0f, 1.0f, 1.0f, 1.0f};     //white: settled into the carved path
const Color32 kCursorColor = {1.0f, 0.6f, 0.0f, 1.0f};   //orange: current active cell
}  // namespace

void RecursiveBacktrackerExample::Clear(World* world) {
  visited.clear();
  stack.clear();
  stack.push_back({0, 0});
}

bool RecursiveBacktrackerExample::Step(World* w) {
  if (stack.empty()) return false;

  Point2D current = stack.back();
  visited[current.y][current.x] = true;
  w->SetNodeColor(w->ToWorldCoords(current), kPathColor);  //is is for the settle into the maze

  std::vector<Point2D> visitables = getVisitables(w, current);

  if (visitables.empty()) {
    // dead end: backtrack
    stack.pop_back();
  } else {
    Point2D next = visitables[0];
    if (visitables.size() > 1) {
      next = visitables[SeededRandom::next() % visitables.size()];
    }

    Point2D worldCurrent = w->ToWorldCoords(current);
    if (next.x == current.x && next.y == current.y - 1) {
      w->SetNorth(worldCurrent, false);
    } else if (next.x == current.x + 1 && next.y == current.y) {
      w->SetEast(worldCurrent, false);
    } else if (next.x == current.x && next.y == current.y + 1) {
      w->SetSouth(worldCurrent, false);
    } else if (next.x == current.x - 1 && next.y == current.y) {
      w->SetWest(worldCurrent, false);
    }

    stack.push_back(next);
  }

  if (!stack.empty()) {
    w->SetNodeColor(w->ToWorldCoords(stack.back()), kCursorColor);  //highlight the new active cell
  }

  return !stack.empty();
}

std::vector<Point2D> RecursiveBacktrackerExample::getVisitables(World* w, const Point2D& formalPoint) {
  //it is for the candidates in clockwise order starting from the top
  std::vector<Point2D> deltas = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};  // UP, RIGHT, DOWN, LEFT
  std::vector<Point2D> visitables;

  for (const auto& delta : deltas) {
    Point2D candidate = {formalPoint.x + delta.x, formalPoint.y + delta.y};
    if (candidate.x < 0 || candidate.x >= w->GetWidth() || candidate.y < 0 || candidate.y >= w->GetHeight()) continue;
    if (visited[candidate.y][candidate.x]) continue;
    visitables.push_back(candidate);
  }

  return visitables;
}
