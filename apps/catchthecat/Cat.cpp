#include "Cat.h"
#include "World.h"
#include <algorithm>
#include <stdexcept>

//it is the cat's move and it tries to pick the move that is the hardest for the catcher to stop
//it ranks every open neighbor by the two distance first then by the normal distance to the closest exit and then by how many shortest ways out it has
//it then looks ahead by trying each move and checking the catcher's best block in reply and it picks the move that is still the best after that block
//it is if there is no way out at all then it moves into the biggest open area so it can survive longer
namespace {
  //it holds the numbers that are used to compare two spots for the cat
  struct CatScore {
    int twoDist; //it is the two distance of the cat's best move from that spot
    int dist; //it is the normal distance to the closest exit
    double paths; //it is how many shortest ways out there are
    int area; //it is how much room the cat has when it is sealed in

    //it returns true if this score is better for the cat than the other one
    bool betterForCat(const CatScore& o) const {
      if (twoDist != o.twoDist) return twoDist < o.twoDist;

      if (dist != o.dist) return dist < o.dist;

      if (paths != o.paths) return paths > o.paths;

      return area > o.area; //it is when the cat is sealed in so more room means it lasts longer
    }
  };
} //namespace

Point2D Cat::Move(CatWorld* world) {
  const int size = world->getWorldSideSize();
  const int half = size / 2;
  const Point2D cat = world->getCat();
  std::vector<bool> blocked = world->worldState(); //it is a working copy that it can change to test moves

  //it builds both maps once so every neighbor can just look up its numbers
  const EscapeField field = computeEscapeField(size, blocked);
  const std::vector<int> twoDist = computeTwoDistance(size, blocked);

  //it lists every open neighbor the cat can move to
  std::vector<Point2D> moves;
  for (const Point2D& n : CatWorld::neighbors(cat)) {
    if (!inside(size, n)) continue; //it is off the board
    if (blocked[index(size, n)]) continue; //it is blocked
    moves.push_back(n);
  }
  if (moves.empty()) return CatWorld::NE(cat); //it is completely surrounded so any move loses and it just returns one

  //it counts the open cells within 2 steps of a spot so the cat can tell which spot has more room to move
  auto room = [&](const Point2D& p) {
    int count = 0;
    for (const Point2D& a : CatWorld::neighbors(p)) {
      if (!inside(size, a) || blocked[index(size, a)] || a == cat) continue;
      ++count;
      for (const Point2D& b : CatWorld::neighbors(a))
        if (inside(size, b) && !blocked[index(size, b)] && b != cat && b != p) ++count;
    }
    return count;
  };

  //it sorts the moves so the best one comes first and it is better if the two distance is lower or if that ties and the normal distance is lower or if that ties too and it has more ways out
  std::stable_sort(moves.begin(), moves.end(), [&](const Point2D& a, const Point2D& b) {
    int ai = index(size, a), bi = index(size, b);
    if (twoDist[ai] != twoDist[bi]) return twoDist[ai] < twoDist[bi];
    if (field.dist[ai] != field.dist[bi]) return field.dist[ai] < field.dist[bi];
    if (field.paths[ai] != field.paths[bi]) return field.paths[ai] > field.paths[bi];
    return room(a) > room(b); //it is when two moves tie on everything so it picks the one with more open cells around it
  });

  //it is when a move lands on the edge so the cat just wins right away
  if (field.dist[index(size, moves[0])] == 0) return moves[0];

  if (field.dist[index(size, moves[0])] == kUnreachable) {
    //it is trapped so it picks the neighbor that leads into the most open space
    Point2D best = moves[0];
    int bestArea = -1;
    for (const Point2D& n : moves) {
      int area = floodFillSize(size, blocked, n);
      if (area > bestArea) {
        bestArea = area;
        best = n;
      }
    }
    return best;
  }

  //it is the score for when the cat would be caught so it is the worst score the cat can get
  const CatScore kCaught{kUnreachable + 1, kUnreachable, 0.0, 0};

  //it scores a spot for the cat by looking at the best move the cat would have from there
  auto evaluate = [&](const Point2D& catPos) -> CatScore {
    const int ci = index(size, catPos);
    EscapeField f = computeEscapeField(size, blocked);
    std::vector<int> two = computeTwoDistance(size, blocked);
    //it only counts the area when the cat is sealed in because that is the only time it matters
    int area = f.dist[ci] == kUnreachable ? floodFillSize(size, blocked, catPos) : 0;
    CatScore bestHere = kCaught; //it stays caught if the cat has no open neighbors at all
    for (const Point2D& n : CatWorld::neighbors(catPos)) {
      if (!inside(size, n)) continue;
      int ni = index(size, n);
      if (blocked[ni]) continue;
      CatScore s{two[ni], f.dist[ni], f.paths[ni], area};
      if (s.betterForCat(bestHere)) bestHere = s;
    }
    return bestHere;
  };

  //it is the veto check where it keeps its first choice unless one catcher block would seal the cat in after that move
  //it is because always planning for the worst case made the cat too careful and it did worse so it only looks ahead when there is real danger
  {
    const int m0 = index(size, moves[0]);
    bool danger = false;
    for (int y = -half; y <= half && !danger; ++y)
      for (int x = -half; x <= half; ++x) {
        int ci = index(size, {x, y});
        if (ci == m0 || blocked[ci]) continue;
        blocked[ci] = true; //it tries the catcher's block
        bool sealed = computeEscapeField(size, blocked).dist[m0] == kUnreachable;
        blocked[ci] = false; //it undoes the block
        if (sealed) {
          danger = true;
          break;
        }
      }
    if (!danger) return moves[0]; //it is safe so it just takes its first choice
  }

  //it is the lookahead where it tries each move and then every block the catcher could answer with
  //it is because the catcher will always pick the block that hurts the cat the most so a move is only as good as its worst case
  //it stops checking a move early once a block makes it no better than the best move so far because the catcher would just pick that block
  Point2D best = moves[0];
  CatScore bestWorst = kCaught;
  bool found = false;
  for (const Point2D& m : moves) {
    const int mi = index(size, m);
    CatScore worst{-1, -1, 0.0, 0}; //it starts as the best score so any real block makes it lower
    bool cut = false;
    for (int y = -half; y <= half && !cut; ++y)
      for (int x = -half; x <= half; ++x) {
        int ci = index(size, {x, y});
        if (ci == mi || blocked[ci]) continue; //it is the catcher's rule so it can not block the cat or a blocked cell
        blocked[ci] = true; //it tries the catcher's block
        CatScore s = evaluate(m);
        blocked[ci] = false; //it undoes the block
        if (worst.betterForCat(s)) worst = s; //it keeps the block that is the worst for the cat
        if (found && !worst.betterForCat(bestWorst)) { //it is when this move can not beat the best one anymore
          cut = true;
          break;
        }
      }
    //it only switches when the worst case is strictly better so ties go to the move that looked best at first
    if (!cut && (!found || worst.betterForCat(bestWorst))) {
      best = m;
      bestWorst = worst;
      found = true;
    }
  }
  return best;
}


















//My second Original code/soluiton:
/*#include "Cat.h"
#include "World.h"
#include <stdexcept>

//it is the cat's move and it tries to pick the move that is the hardest for the catcher to stop
//it ranks every open neighbor by the two distance first then by the normal distance to the closest exit and then by how many shortest ways out it has
//it is if there is no way out at all then it moves into the biggest open area so it can survive longer
Point2D Cat::Move(CatWorld* world) {
  const int size = world->getWorldSideSize();
  const Point2D cat = world->getCat();
  const std::vector<bool>& blocked = world->worldState();

  //it builds both maps once so every neighbor can just look up its numbers
  const EscapeField field = computeEscapeField(size, blocked);
  const std::vector<int> twoDist = computeTwoDistance(size, blocked);

  Point2D best = cat;
  bool found = false;
  int bestTwo = 0, bestDist = 0;
  double bestPaths = 0.0;

  //it goes through all six neighbors and keeps the best one
  for (const Point2D& n : CatWorld::neighbors(cat)) {
    if (!inside(size, n)) continue; //it is off the board
    int i = index(size, n);
    if (blocked[i]) continue; //it is blocked
    int t = twoDist[i], d = field.dist[i];
    double p = field.paths[i];

    //it is better if the two distance is lower or if that ties and the normal distance is lower or if that ties too and it has more ways out
    bool better = !found || t < bestTwo || (t == bestTwo && (d < bestDist || (d == bestDist && p > bestPaths)));
    if (better) {
      best = n;
      bestTwo = t;
      bestDist = d;
      bestPaths = p;
      found = true;
    }
  }
  if (!found) return CatWorld::NE(cat); //it is completely surrounded so any move loses and it just returns one

  if (bestDist == kUnreachable) {
    //it is trapped so it picks the neighbor that leads into the most open space
    int bestArea = -1;
    for (const Point2D& n : CatWorld::neighbors(cat)) {
      if (!inside(size, n) || blocked[index(size, n)]) continue;
      int area = floodFillSize(size, blocked, n);
      if (area > bestArea) {
        bestArea = area;
        best = n;
      }
    }
  }
  return best;
}*/





//My first Original code/soluiton:
/*Point2D Cat::Move(CatWorld* world) {
  //it tries to follow the shortest path to the boorder
  auto path = generatePath(world);
  if (!path.empty()) return path.back();

  //it is if it is trapped it steps into any free neighbot to survie as long as possible
  auto pos = world->getCat();
  for (const auto& n : CatWorld::neighbors(pos))
  {
    if (world->isValidPosition(n) && !world->getContent(n)) return n;
  }

  //it is if it complettely surrounded and any move loses so it just reurns one
  return CatWorld::NE(pos);*/






  //Old Professor code:
  /*auto rand = Random::Range(0, 5);
  auto pos = world->getCat();
  switch (rand) {
    case 0:
      return CatWorld::NE(pos);
    case 1:
      return CatWorld::NW(pos);
    case 2:
      return CatWorld::E(pos);
    case 3:
      return CatWorld::W(pos);
    case 4:
      return CatWorld::SW(pos);
    case 5:
      return CatWorld::SE(pos);
    default:
      throw std::runtime_error("random out of range");
  }*/
//}
