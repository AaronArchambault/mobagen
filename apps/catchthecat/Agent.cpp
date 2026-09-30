#include "Agent.h"
#include <climits>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "World.h"

using namespace std;

// Returns neighbors of `p` that are inside the board, not blocked, not the cat,
// not already visited, and not already waiting in the frontier.
static vector<Point2D> getVisitableNeighbors(CatWorld* w, const Point2D& p, const unordered_map<Point2D, bool>& visited,
                                             const unordered_set<Point2D>& frontierSet) {
  vector<Point2D> result;
  const Point2D catPos = w->getCat();
  for (const Point2D& n : CatWorld::neighbors(p)) {
    if (!w->isValidPosition(n)) continue;             // outside the board
    if (n == catPos) continue;                         // the cat itself
    if (w->getContent(n)) continue;                    // blocked
    if (visited.find(n) != visited.end()) continue;    // already visited
    if (frontierSet.count(n)) continue;                // already queued
    result.push_back(n);
  }
  return result;
}

std::vector<Point2D> Agent::generatePath(CatWorld* w) {
  unordered_map<Point2D, Point2D> cameFrom;  // to build the flowfield and build the path
  queue<Point2D> frontier;                   // to store next ones to visit
  unordered_set<Point2D> frontierSet;        // check faster if a point is in the queue
  unordered_map<Point2D, bool> visited;      // use .at() to get data, if the element dont exist [] will give you wrong results

  // bootstrap state
  auto catPos = w->getCat();
  frontier.push(catPos);
  frontierSet.insert(catPos);
  Point2D borderExit = {INT32_MAX, INT32_MAX};  //if no border found yet

  while (!frontier.empty()) {
    Point2D current = frontier.front();
    frontier.pop();
    frontierSet.erase(current);
    visited[current] = true;

    for (const Point2D& neighbor : getVisitableNeighbors(w, current, visited, frontierSet)) {
      cameFrom[neighbor] = current;
      frontier.push(neighbor);
      frontierSet.insert(neighbor);

      // BFS on an unweighted grid: the first border we discover is a closest one.
      if (w->catWinsOnSpace(neighbor)) {
        borderExit = neighbor;
        break;
      }
    }
    if (borderExit.x != INT32_MAX) break;
  }

  // No reachable border: the cat is trapped.
  if (borderExit.x == INT32_MAX) return {};

  // Walk back from the border to the cat (cat itself is not included).
  // path.front() = border cell (catcher move), path.back() = cell next to the cat (cat move).
  vector<Point2D> path;
  Point2D current = borderExit;
  while (current != catPos) {
    path.push_back(current);
    current = cameFrom.at(current);
  }
  return path;
}
