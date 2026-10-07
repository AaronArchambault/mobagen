#include "Agent.h"
#include <climits>
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
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

//it turns the world's board into a grid with one byte for each cell
Agent::Grid Agent::toGrid(const std::vector<bool>& state) {
  Grid grid(state.size());
  for (size_t i = 0; i < state.size(); ++i) grid[i] = state[i] ? 1 : 0;
  return grid;
}

//it builds the neighbor table for a board size and keeps it around so it only gets built once
//it is a big speed up because the old way made a new vector every time it asked for the neighbors of a cell
const Agent::NeighborTable& Agent::neighborTable(int size) {
  static NeighborTable table;
  if (table.size == size) return table;

  const int half = size / 2;
  table.size = size;
  table.neighbors.assign(size * size * 6, -1);
  table.border.clear();
  for (int y = -half; y <= half; ++y)
    for (int x = -half; x <= half; ++x) {
      Point2D p = {x, y};
      int i = index(size, p);
      std::vector<Point2D> around = CatWorld::neighbors(p);
      for (int k = 0; k < 6; ++k) table.neighbors[i * 6 + k] = inside(size, around[k]) ? index(size, around[k]) : -1;
      if (std::abs(x) == half || std::abs(y) == half) table.border.push_back(i);
    }
  return table;
}

//it builds the escape map and gives it back as a new map
Agent::EscapeField Agent::computeEscapeField(int size, const Grid& blocked) {
  EscapeField f;
  computeEscapeField(size, blocked, f);
  return f;
}

//it builds the escape map for the whole board with one bfs that starts from every open border cell at the same time
//it saves how many steps each cell is from the closest exit and how many different shortest ways there are to get out
//it is so the cat and the catcher can look at the whole board without running a new search from every cell
//it reads plain arrays through pointers because that is a lot faster than going through the vector every time in an unoptimized build
void Agent::computeEscapeField(int size, const Grid& blockedGrid, EscapeField& out) {
  const NeighborTable& table = neighborTable(size);
  const int cells = size * size;
  static std::vector<int> queue; //it is reused memory for the queue so it does not get made again every call
  if (static_cast<int>(queue.size()) < cells) queue.resize(cells);
  out.dist.resize(cells);
  out.paths.resize(cells);

  int* dist = out.dist.data();
  double* paths = out.paths.data();
  int* q = queue.data();
  const unsigned char* blocked = blockedGrid.data();
  const int* neighbors = table.neighbors.data();
  const int* border = table.border.data();
  const int borderCount = static_cast<int>(table.border.size());

  for (int i = 0; i < cells; ++i) {
    dist[i] = kUnreachable;
    paths[i] = 0.0;
  }

  //it starts every open border cell as an exit with a distance of 0 and 1 way out
  int tail = 0;
  for (int k = 0; k < borderCount; ++k) {
    int i = border[k];
    if (blocked[i]) continue; //it is a blocked border cell so it is not an exit
    dist[i] = 0;
    paths[i] = 1.0;
    q[tail++] = i;
  }

  //it is the normal bfs but it also counts the shortest paths
  for (int head = 0; head < tail; ++head) {
    int ci = q[head];
    const int* around = neighbors + ci * 6;
    for (int k = 0; k < 6; ++k) {
      int ni = around[k];
      if (ni < 0 || blocked[ni]) continue; //it is off the board or blocked
      if (dist[ni] == kUnreachable) {
        //it is the first time the cell is reached so it gets its distance and copies the path count
        dist[ni] = dist[ci] + 1;
        paths[ni] = paths[ci];
        q[tail++] = ni;
      } else if (dist[ni] == dist[ci] + 1) {
        //it is another shortest way into the same cell so it adds the path count on top
        paths[ni] += paths[ci];
      }
    }
  }
}

//it counts how many open cells can still be reached from start
//it is used when there is no way out so the cat and the catcher can tell which area is the biggest
int Agent::floodFillSize(int size, const Grid& blockedGrid, const Point2D& start) {
  const NeighborTable& table = neighborTable(size);
  const int cells = size * size;
  static std::vector<int> queue; //it is reused memory so it does not get made again every call
  static std::vector<unsigned char> seenGrid;
  if (static_cast<int>(queue.size()) < cells) queue.resize(cells);
  seenGrid.assign(cells, 0);

  int* q = queue.data();
  unsigned char* seen = seenGrid.data();
  const unsigned char* blocked = blockedGrid.data();
  const int* neighbors = table.neighbors.data();

  int tail = 0;
  q[tail++] = index(size, start);
  seen[q[0]] = 1;
  for (int head = 0; head < tail; ++head) {
    const int* around = neighbors + q[head] * 6;
    for (int k = 0; k < 6; ++k) {
      int ni = around[k];
      if (ni < 0 || blocked[ni] || seen[ni]) continue;
      seen[ni] = 1;
      q[tail++] = ni;
    }
  }
  return tail; //it is how many cells got reached
}

//it builds the two distance map and gives it back as a new map
std::vector<int> Agent::computeTwoDistance(int size, const Grid& blocked) {
  std::vector<int> value;
  computeTwoDistance(size, blocked, value);
  return value;
}

//it is the two distance idea that comes from hex game ais
//it works because the catcher will always block the best next step so a cell is only as good as its second best neighbor
//it gives border cells a 0 and every other cell gets 1 plus the second smallest value of its open neighbors
//it means the smaller the number the faster the cat can force its way out even when the catcher blocks well
void Agent::computeTwoDistance(int size, const Grid& blockedGrid, std::vector<int>& out) {
  const NeighborTable& table = neighborTable(size);
  const int cells = size * size;
  static std::vector<int> queue; //it is reused memory so it does not get made again every call
  static std::vector<int> settledGrid;
  if (static_cast<int>(queue.size()) < cells) queue.resize(cells);
  if (static_cast<int>(settledGrid.size()) < cells) settledGrid.resize(cells);
  out.resize(cells);

  int* value = out.data();
  int* settledNeighbors = settledGrid.data();
  int* q = queue.data();
  const unsigned char* blocked = blockedGrid.data();
  const int* neighbors = table.neighbors.data();
  const int* border = table.border.data();
  const int borderCount = static_cast<int>(table.border.size());

  for (int i = 0; i < cells; ++i) {
    value[i] = kUnreachable;
    settledNeighbors[i] = 0;
  }

  //it starts every open border cell with a value of 0
  int tail = 0;
  for (int k = 0; k < borderCount; ++k) {
    int i = border[k];
    if (blocked[i]) continue;
    value[i] = 0;
    q[tail++] = i;
  }

  //it settles the cells in order from smallest to biggest just like a bfs
  //it gives a cell its value the moment its second neighbor gets settled because that neighbor has the second smallest value
  for (int head = 0; head < tail; ++head) {
    int ci = q[head];
    const int* around = neighbors + ci * 6;
    for (int k = 0; k < 6; ++k) {
      int ni = around[k];
      if (ni < 0 || blocked[ni] || value[ni] != kUnreachable) continue;
      if (++settledNeighbors[ni] == 2) {
        value[ni] = value[ci] + 1;
        q[tail++] = ni;
      }
    }
  }
}

//it checks if there are two routes from start to the edge that do not share a single cell
//it is the same question as asking if one block could cut start off from the edge because one block can only ever cut one of two separate routes
//it works like a max flow where every open cell can carry one route and the edge is the goal and it only needs to find 2 routes so it is just 2 quick searches
//it splits each cell into an in side and an out side with room for one route between them so two routes can never share a cell
bool Agent::hasTwoSeparateRoutes(int size, const Grid& blockedGrid, int start) {
  const NeighborTable& table = neighborTable(size);
  const int cells = size * size;
  const int goal = 2 * cells; //it is one extra node that every open border cell leads to
  const int nodes = goal + 1;
  const unsigned char* blocked = blockedGrid.data();
  const int* neighbors = table.neighbors.data();

  //it builds the graph as a list of one way edges where each edge has a partner going back for the flow to undo
  std::vector<int> head(nodes, -1), next, to, room;
  next.reserve(cells * 16);
  to.reserve(cells * 16);
  room.reserve(cells * 16);
  auto addEdge = [&](int a, int b) {
    to.push_back(b); room.push_back(1); next.push_back(head[a]); head[a] = static_cast<int>(to.size()) - 1;
    to.push_back(a); room.push_back(0); next.push_back(head[b]); head[b] = static_cast<int>(to.size()) - 1;
  };
  for (int i = 0; i < cells; ++i) {
    if (blocked[i]) continue;
    if (i != start) addEdge(2 * i, 2 * i + 1); //it is the room for one route through this cell
    const int* around = neighbors + i * 6;
    bool onBorder = false;
    for (int k = 0; k < 6; ++k) {
      int ni = around[k];
      if (ni < 0) {
        onBorder = true;
        continue;
      }
      if (!blocked[ni] && ni != start) addEdge(2 * i + 1, 2 * ni); //it is a step from this cell into an open neighbor
    }
    if (onBorder) addEdge(2 * i + 1, goal); //it is an open border cell so it leads to the edge
  }

  //it looks for a route with a bfs and pushes one route along it and it does that up to 2 times
  std::vector<int> from(nodes);
  for (int found = 0; found < 2; ++found) {
    std::fill(from.begin(), from.end(), -1);
    std::vector<int> q{2 * start + 1};
    from[2 * start + 1] = -2;
    for (size_t h = 0; h < q.size() && from[goal] == -1; ++h)
      for (int e = head[q[h]]; e != -1; e = next[e])
        if (room[e] > 0 && from[to[e]] == -1) {
          from[to[e]] = e;
          q.push_back(to[e]);
        }
    if (from[goal] == -1) return false; //it can not find another route so one block is enough to cut it off
    for (int v = goal; v != 2 * start + 1; v = to[from[v] ^ 1]) { //it walks back along the route and uses it up
      room[from[v]] -= 1;
      room[from[v] ^ 1] += 1;
    }
  }
  return true;
}






