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

void RecursiveBacktrackerExample::Clear(World* world) {
  // todo: reset the walk
  // hint:
  //   clear visited and the path stack, then start the walk at the
  //   top-left cell in formal units: stack.push_back({0, 0})
  // begin solution
  visited.clear();
  stack.clear();
  stack.push_back({0, 0});

  // end solution
}

bool RecursiveBacktrackerExample::Step(World* w) {
  // todo: implement one iteration of the recursive backtracker
  // hint:
  //   empty stack  -> the maze is done, return false
  //   otherwise, on the cell at the top of the stack (formal units):
  //   1. mark it visited;
  //   2. list its visitable neighbors with getVisitables
  //      (already in clockwise order: UP, RIGHT, DOWN, LEFT);
  //   3. none        -> dead end: pop the stack (backtrack);
  //   4. exactly one -> move to it, do not consume a random number;
  //   5. two or more -> consume SeededRandom::next() and pick
  //      next() % visitables.size();
  //   moving = opening the wall between the two cells, through the
  //   World coordinate translation:
  //     Point2D worldCurrent = w->ToWorldCoords(current);
  //     UP    -> w->SetNorth(worldCurrent, false)
  //     RIGHT -> w->SetEast(worldCurrent, false)
  //     DOWN  -> w->SetSouth(worldCurrent, false)
  //     LEFT  -> w->SetWest(worldCurrent, false)
  //   return true while there is still work (stack not empty after the move)
  // begin solution
  if (stack.empty()) return false;

  Point2D current = stack.back();
  visited[current.y][current.x] = true;

  std::vector<Point2D> visitables = getVisitables(w, current);

  if (visitables.empty())
  {
    //it is/found a dead end so backtrack
    stack.pop_back();
  }
  else
  {
    Point2D next = visitables[0];
    if (visitables.size() > 1) {
      next = visitables[SeededRandom::next() % visitables.size()];
    }
    Point2D worldCurrent = w->ToWorldCoords(current);
    if (next.x == current.x && next.y == current.y -1)
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
  return !stack.empty();
  // end solution
  //return false;
}

std::vector<Point2D> RecursiveBacktrackerExample::getVisitables(World* w, const Point2D& formalPoint) {
  // todo: list the unvisited neighbors of formalPoint, in clockwise order
  // hint:
  //   candidates in order: UP {x, y-1}, RIGHT {x+1, y}, DOWN {x, y+1}, LEFT {x-1, y}
  //   keep a candidate only if it is inside the grid
  //   (0 <= x < w->GetWidth(), 0 <= y < w->GetHeight()) and not visited
  // begin solution
  std::vector<Point2D> deltas = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};  // Up, Right, Down, Left
  std::vector<Point2D> visitables;

  for (const auto& delta : deltas)
  {
    Point2D candidate = {formalPoint.x + delta.x, formalPoint.y + delta.y};
    if (candidate.x < 0 || candidate.x >= w->GetWidth() || candidate.y < 0 || candidate.y >= w->GetHeight()) continue;

    if (visited[candidate.y][candidate.x]) continue;

    visitables.push_back(candidate);
  }
  return visitables;
  // end solution
 // return {};
}
