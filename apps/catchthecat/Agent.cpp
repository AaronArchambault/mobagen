#include "Agent.h"
#include <climits>
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "World.h"

//https://www.redblobgames.com/pathfinding/a-star/introduction.html
//https://www.redblobgames.com/pathfinding/tower-defense/
//https://www.redblobgames.com/grids/hexagons/
//https://www.baeldung.com/cs/graph-number-of-shortest-paths
//https://benzene.sourceforge.net/benzene-doc/html/classTwoDistance.html
//https://download.racket-lang.org/docs/5.0/html/games/chat-noir.html
//https://discuss.codechef.com/problems/BEARTRAP


using namespace std;

//it gets the neighbors of p that the search can still go to
//it skips the ones that are off the board or are the cat or are blocked or were already visited or are already waiting in the queue
static vector<Point2D> getVisitableNeighbors(CatWorld* w, const Point2D& p, const unordered_map<Point2D, bool>& visited, const unordered_set<Point2D>& frontierSet) {

  vector<Point2D> result;
  const Point2D catPos = w->getCat();
  for (const Point2D& n : CatWorld::neighbors(p)) {
    if (!w->isValidPosition(n)) continue; //it is the outside of the board
    if (n == catPos) continue; //it is the cat itself
    if (w->getContent(n)) continue; //it is blocked
    if (visited.find(n) != visited.end()) continue; //it was already visited
    if (frontierSet.count(n)) continue; //it is already in the queue
    result.push_back(n);
  }
  return result;
}

//it finds the shortest path from the cat to the closest open border cell with a breadth first search
//it gives back the path going from the border to the cat so the front is the border cell and the back is the cell right next to the cat
//it gives back an empty path if the cat can not get to the border anymore
std::vector<Point2D> Agent::generatePath(CatWorld* w) {

  unordered_map<Point2D, Point2D> cameFrom; //it remembers where each cell came from so it can build the flowfield and the path
  queue<Point2D> frontier; //it stores the next cells to visit
  unordered_set<Point2D> frontierSet; //it is for optimization so it can check faster if a point is in the queue
  unordered_map<Point2D, bool> visited; //it uses find to check it because using [] on a missing element would add it and give the wrong results

  //it is the bootstrap state so the search starts on the cat
  auto catPos = w->getCat();
  frontier.push(catPos);
  frontierSet.insert(catPos);
  Point2D borderExit = {INT32_MAX, INT32_MAX}; //it is the sentinel so it means no border was found yet

  while (!frontier.empty()) {
    //it takes the next cell out of the queue and marks it as visited
    Point2D current = frontier.front();
    frontier.pop();
    frontierSet.erase(current);
    visited[current] = true;

    for (const Point2D& neighbor : getVisitableNeighbors(w, current, visited, frontierSet)) {
      //it remembers where the neighbor came from and adds it to the queue
      cameFrom[neighbor] = current;
      frontier.push(neighbor);
      frontierSet.insert(neighbor);

      //it is a bfs on a grid where every step costs the same so the first border it finds is one of the closest ones
      if (w->catWinsOnSpace(neighbor)) {
        borderExit = neighbor;
        break;
      }
    }
    if (borderExit.x != INT32_MAX) break; //it stops the whole search once a border was found
  }

  //it is if there is no reachable border so the cat is trapped
  if (borderExit.x == INT32_MAX) return {};

  //it walks back from the border to the cat using camefrom and it does not add the cat itself
  //it makes it so the front is the border cell for the catcher and the back is the cell next to the cat for the cat
  vector<Point2D> path;
  Point2D current = borderExit;
  while (current != catPos) {
    path.push_back(current);
    current = cameFrom.at(current);
  }
  return path;
}

//it builds the neighbor table for a board size and keeps it around so it only gets built once
//it is a big speed up because the old way made a new vector every time it asked for the neighbors of a cell
const Agent::NeighborTable& Agent::neighborTable(int size) {
  static NeighborTable table;
  if (table.size == size) return table;

  const int half = size / 2;
  table.size = size;
  table.neighbors.assign(size * size, {});
  table.border.clear();
  for (int y = -half; y <= half; ++y)
    for (int x = -half; x <= half; ++x) {
      Point2D p = {x, y};
      int i = index(size, p);
      std::vector<Point2D> around = CatWorld::neighbors(p);
      for (int k = 0; k < 6; ++k) table.neighbors[i][k] = inside(size, around[k]) ? index(size, around[k]) : -1;
      if (std::abs(x) == half || std::abs(y) == half) table.border.push_back(i);
    }
  return table;
}

//it builds the escape map for the whole board with one bfs that starts from every open border cell at the same time
//it saves how many steps each cell is from the closest exit and how many different shortest ways there are to get out
//it is so the cat and the catcher can look at the whole board without running a new search from every cell
Agent::EscapeField Agent::computeEscapeField(int size, const std::vector<bool>& blocked) {
  const NeighborTable& table = neighborTable(size);
  EscapeField f;
  f.dist.assign(size * size, kUnreachable);
  f.paths.assign(size * size, 0.0);

  //it uses a plain vector as the queue with a read spot so it does not have to make and free memory all the time
  vector<int> q;
  q.reserve(size * size);

  //it starts every open border cell as an exit with a distance of 0 and 1 way out
  for (int i : table.border) {
    if (blocked[i]) continue; //it is a blocked border cell so it is not an exit
    f.dist[i] = 0;
    f.paths[i] = 1.0;
    q.push_back(i);
  }

  //it is the normal bfs but it also counts the shortest paths
  for (size_t head = 0; head < q.size(); ++head) {
    int ci = q[head];
    for (int ni : table.neighbors[ci]) {
      if (ni < 0 || blocked[ni]) continue; //it is off the board or blocked
      if (f.dist[ni] == kUnreachable) {
        //it is the first time the cell is reached so it gets its distance and copies the path count
        f.dist[ni] = f.dist[ci] + 1;
        f.paths[ni] = f.paths[ci];
        q.push_back(ni);
      } else if (f.dist[ni] == f.dist[ci] + 1) {
        //it is another shortest way into the same cell so it adds the path count on top
        f.paths[ni] += f.paths[ci];
      }
    }
  }
  return f;
}

//it counts how many open cells can still be reached from start
//it is used when there is no way out so the cat and the catcher can tell which area is the biggest
int Agent::floodFillSize(int size, const std::vector<bool>& blocked, const Point2D& start) {
  const NeighborTable& table = neighborTable(size);
  vector<bool> seen(size * size, false);
  vector<int> q;
  q.reserve(size * size);
  q.push_back(index(size, start));
  seen[q[0]] = true;

  for (size_t head = 0; head < q.size(); ++head) {
    for (int ni : table.neighbors[q[head]]) {
      if (ni < 0 || blocked[ni] || seen[ni]) continue;
      seen[ni] = true;
      q.push_back(ni);
    }
  }
  return static_cast<int>(q.size()); //it is how many cells got reached
}

//it is the two distance idea that comes from hex game ais
//it works because the catcher will always block the best next step so a cell is only as good as its second best neighbor
//it gives border cells a 0 and every other cell gets 1 plus the second smallest value of its open neighbors
//it means the smaller the number the faster the cat can force its way out even when the catcher blocks well
std::vector<int> Agent::computeTwoDistance(int size, const std::vector<bool>& blocked) {
  const NeighborTable& table = neighborTable(size);
  vector<int> value(size * size, kUnreachable);
  vector<int> settledNeighbors(size * size, 0);
  vector<int> q;
  q.reserve(size * size);

  //it starts every open border cell with a value of 0
  for (int i : table.border) {
    if (blocked[i]) continue;
    value[i] = 0;
    q.push_back(i);
  }

  //it settles the cells in order from smallest to biggest just like a bfs
  //it gives a cell its value the moment its second neighbor gets settled because that neighbor has the second smallest value
  for (size_t head = 0; head < q.size(); ++head) {
    int ci = q[head];
    for (int ni : table.neighbors[ci]) {
      if (ni < 0 || blocked[ni] || value[ni] != kUnreachable) continue;
      if (++settledNeighbors[ni] == 2) {
        value[ni] = value[ci] + 1;
        q.push_back(ni);
      }
    }
  }
  return value;
}







//it is the old/original code/solution:
/*using namespace std;

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
}*/
