#include "Catcher.h"
#include "World.h"

//it is the catcher's move and it tries every open cell as a block and keeps the one that leaves the cat's best move as weak as possible
//it scores each block by the move the cat would make next and it wants that move to have a big two distance first then a big normal distance and then as few shortest ways out as it can
//it is if a few blocks would all seal the cat in then it picks the one that leaves the cat the smallest area so the game ends faster
//it is once the cat is sealed off it just closes in on the cat by blocking its neighbors
namespace {
  //it holds the numbers that are used to compare two blocks
  struct Score {
    int twoDist; //it is the two distance of the cat's best move
    int dist; //it is the normal distance to the closest exit
    double paths; //it is how many shortest ways out there are
    int area; //it is how much room the cat has left when it is sealed in and it is only used to break ties

    //it returns true if this score is better for the catcher than the other one
    bool beats(const Score& o) const {
      if (twoDist != o.twoDist) return twoDist > o.twoDist;

      if (dist != o.dist) return dist > o.dist;

      if (paths != o.paths) return paths < o.paths;

      return area < o.area; //it is when both blocks seal the cat in so it picks the one that leaves the cat less room
    }
  };
} //namespace

Point2D Catcher::Move(CatWorld* world) {
  const int size = world->getWorldSideSize();
  const int half = size / 2;
  const Point2D cat = world->getCat();
  const int catIdx = index(size, cat);
  std::vector<bool> blocked = world->worldState(); //it is a working copy that it can change to test blocks

  //it is if the cat already can not escape so it plays the endgame inside the cat's pocket
  //it tries every open cell in the pocket as a block and looks at where the cat would run next
  //it picks the block where the biggest area the cat can run into is the smallest and if that ties it leaves the cat fewer open neighbors
  if (computeEscapeField(size, blocked).dist[catIdx] == kUnreachable) {
    Point2D best = cat;
    int bestRun = kUnreachable, bestExits = kUnreachable;
    for (int y = -half; y <= half; ++y)
      for (int x = -half; x <= half; ++x) {
        Point2D c = {x, y};
        int ci = index(size, c);
        if (ci == catIdx || blocked[ci]) continue;
        blocked[ci] = true; //it tries the block
        blocked[catIdx] = true; //it treats the cat's spot as closed so each run only counts the side the cat steps into

        //it finds the cat's best run after this block
        int run = 0, exits = 0;
        for (const Point2D& n : CatWorld::neighbors(cat)) {
          if (!inside(size, n) || blocked[index(size, n)]) continue;
          ++exits;
          int a = floodFillSize(size, blocked, n);
          if (a > run) run = a;
        }
        blocked[catIdx] = false;
        blocked[ci] = false; //it undoes the block

        if (run < bestRun || (run == bestRun && exits < bestExits)) {
          bestRun = run;
          bestExits = exits;
          best = c;
        }
      }
    if (bestRun != kUnreachable) return best;
  }

  Point2D best = {kUnreachable, kUnreachable};
  Score bestScore{-1, -1, 0.0, 0};

  //it tries every open cell on the board as a block
  for (int y = -half; y <= half; ++y)
    for (int x = -half; x <= half; ++x) {
      Point2D c = {x, y};
      int ci = index(size, c);
      if (ci == catIdx || blocked[ci]) continue; //it can not block the cat or a cell that is already blocked

      blocked[ci] = true; //it tries the block
      EscapeField f = computeEscapeField(size, blocked);
      std::vector<int> two = computeTwoDistance(size, blocked);
      //it only counts the area when this block seals the cat in because that is the only time it matters
      int area = f.dist[catIdx] == kUnreachable ? floodFillSize(size, blocked, cat) : 0;
      blocked[ci] = false; //it undoes the block

      //it finds the cat's best move after this block and the lowest numbers are the best for the cat
      Score reply{kUnreachable, kUnreachable, 0.0, area};
      for (const Point2D& n : CatWorld::neighbors(cat)) {
        if (!inside(size, n)) continue;
        int ni = index(size, n);
        if (blocked[ni] || ni == ci) continue;
        Score s{two[ni], f.dist[ni], f.paths[ni], area};
        if (reply.beats(s)) reply = s; //it keeps the move that is the worst for the catcher
      }

      //it keeps this block if it is the first one or if it is better than the best one so far
      if (best.x == kUnreachable || reply.beats(bestScore)) {
        best = c;
        bestScore = reply;
      }
    }

  if (best.x == kUnreachable) { //it is if there are no open cells at all which should not happen
    for (int y = -half; y <= half; ++y)
      for (int x = -half; x <= half; ++x)
        if (index(size, {x, y}) != catIdx) return {x, y};
  }
  return best;
}









//my old original code/solution:
/*Point2D Catcher::Move(CatWorld* world) {
  //it blocks the cat's escape route at the border end of its shortest path
  auto path = generatePath(world);
  if (!path.empty()) return path.front();

  //it makes it so the cat can't reach the border anymore and it attempts to close off its remaining free neighbors
  auto cat = world->getCat();
  for (const auto& n : CatWorld::neighbors(cat)) {
    if (world->isValidPosition(n) && !world->getContent(n)) return n;
  }

  //it is its fallback so random free cell
  auto side = world->getWorldSideSize() / 2;
  for (;;) {
    Point2D p = {Random::Range(-side, side), Random::Range(-side, side)};
    if (p != cat && !world->getContent(p)) return p;
  }
}*/
