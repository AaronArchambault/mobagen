#include "Catcher.h"
#include "World.h"
#include <algorithm>
#include <utility>

//it is the catcher's move and it tries every open cell as a block and keeps the one that leaves the cat's best move as weak as possible
//it scores each block by the move the cat would make next and it wants that move to have a big two distance first then a big normal distance and then as few shortest ways out as it can
//it is if a few blocks would all seal the cat in then it picks the one that leaves the cat the smallest area so the game ends faster
//it looks ahead on the 5 best blocks by checking the cat's 2 best replies and the catcher's best follow up to each one and it picks the block with the best worst case
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
  Grid blocked = toGrid(world->worldState()); //it is a working copy that it can change to test blocks
  const NeighborTable& table = neighborTable(size);

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

  //it is the score for when the cat gets caught and for when it gets out so the lookahead can tell a win and a loss apart
  const Score kCaught{kUnreachable + 1, kUnreachable, 0.0, 0};
  const Score kEscaped{-1, -1, 0.0, 0};

  //it scores the board from the catcher's side by looking at the best move the cat has on it
  //it is the same score as before so the cat's best move is the one that is the worst for the catcher
  //it reuses these two maps for every call so it does not make new memory hundreds of times
  EscapeField evalField;
  std::vector<int> evalTwo;
  auto evaluate = [&](const Point2D& catPos) -> Score {
    const int ci = index(size, catPos);
    computeEscapeField(size, blocked, evalField);
    computeTwoDistance(size, blocked, evalTwo);
    const int* dist = evalField.dist.data();
    const double* paths = evalField.paths.data();
    const int* two = evalTwo.data();
    //it only counts the area when the cat is sealed in because that is the only time it matters
    int area = dist[ci] == kUnreachable ? floodFillSize(size, blocked, catPos) : 0;
    Score reply = kCaught; //it stays caught if the cat has no open neighbors at all
    const int* around = table.neighbors.data() + ci * 6; //it uses the neighbor table instead of making a new list of neighbors
    for (int k = 0; k < 6; ++k) {
      int ni = around[k];
      if (ni < 0 || blocked[ni]) continue; //it is off the board or blocked
      Score s{two[ni], dist[ni], paths[ni], area};
      if (reply.beats(s)) reply = s; //it keeps the move that is the worst for the catcher
    }
    return reply;
  };

  //it is the first look where it scores every open cell as a block just like before
  std::vector<std::pair<Score, Point2D>> ranked;
  for (int y = -half; y <= half; ++y)
    for (int x = -half; x <= half; ++x) {
      Point2D c = {x, y};
      int ci = index(size, c);
      if (ci == catIdx || blocked[ci]) continue; //it can not block the cat or a cell that is already blocked
      blocked[ci] = true; //it tries the block
      Score s = evaluate(cat);
      blocked[ci] = false; //it undoes the block
      ranked.push_back({s, c});
    }

  if (ranked.empty()) { //it is if there are no open cells at all which should not happen
    for (int y = -half; y <= half; ++y)
      for (int x = -half; x <= half; ++x)
        if (index(size, {x, y}) != catIdx) return {x, y};
  }

  //it sorts the blocks from best to worst and stable sort keeps the board order for ties so it plays the same way every time
  std::stable_sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) { return a.first.beats(b.first); });
  if (ranked[0].first.twoDist == kCaught.twoDist) return ranked[0].second; //it is a block that catches the cat right now so it just takes it

  //it is how far the lookahead goes and these numbers keep it fast enough for the time limit
  const int kTopBlocks = 5; //it only looks ahead on the 5 best blocks from the first look
  const int kCatReplies = 2; //it only checks the cat's 2 best replies
  const int kNearSteps = 3; //it tries follow up blocks within 3 steps of the cat
  const int kTopFollowUps = 10; //it also tries the 10 best blocks from the first look as follow ups

  //it gets the follow up blocks to try after the cat moves to that spot
  //it uses the cells close to the cat because blocking near the cat matters the most and it adds the best cells from the first look so it does not miss a good wall farther away
  auto followUpCells = [&](const Point2D& catPos) {
    std::vector<int> cells;
    std::vector<int> steps(size * size, -1);
    int start = index(size, catPos);
    steps[start] = 0;
    cells.push_back(start);
    for (size_t head = 0; head < cells.size(); ++head) {
      int cur = cells[head];
      if (steps[cur] == kNearSteps) continue;
      for (int k = 0; k < 6; ++k) {
        int ni = table.neighbors[cur * 6 + k];
        if (ni < 0 || steps[ni] >= 0) continue;
        steps[ni] = steps[cur] + 1;
        cells.push_back(ni);
      }
    }
    for (int k = 0; k < kTopFollowUps && k < static_cast<int>(ranked.size()); ++k) {
      int ri = index(size, ranked[k].second);
      if (steps[ri] < 0) {
        steps[ri] = kNearSteps + 1;
        cells.push_back(ri);
      }
    }
    return cells;
  };

  //it finds the worst case after a block that is already placed on the board
  //it is the cat's reply that is the worst for the catcher after the catcher answers it with its best follow up block
  auto worstCase = [&]() -> Score {
    EscapeField f = computeEscapeField(size, blocked);
    std::vector<int> two = computeTwoDistance(size, blocked);

    //it lists the cat's possible replies and sorts them the same way the cat ranks its moves so the cat's best ones come first
    std::vector<Point2D> replies;
    for (const Point2D& n : CatWorld::neighbors(cat))
      if (inside(size, n) && !blocked[index(size, n)]) replies.push_back(n);
    if (replies.empty()) return kCaught; //it is when the block already surrounds the cat
    std::stable_sort(replies.begin(), replies.end(), [&](const Point2D& a, const Point2D& b) {
      int ai = index(size, a), bi = index(size, b);
      if (two[ai] != two[bi]) return two[ai] < two[bi];
      if (f.dist[ai] != f.dist[bi]) return f.dist[ai] < f.dist[bi];
      return f.paths[ai] > f.paths[bi];
    });
    if (two[index(size, replies[0])] == 0) return kEscaped; //it is when the cat can step onto the edge so the cat wins

    Score worst = kCaught;
    for (int r = 0; r < kCatReplies && r < static_cast<int>(replies.size()); ++r) {
      const Point2D reply = replies[r];
      //it tries every follow up block and keeps the best one for the catcher
      Score bestFollowUp = kEscaped;
      for (int ci : followUpCells(reply)) {
        if (blocked[ci] || ci == index(size, reply)) continue;
        blocked[ci] = true; //it tries the follow up
        Score s = evaluate(reply);
        blocked[ci] = false; //it undoes the follow up
        if (s.beats(bestFollowUp)) bestFollowUp = s;
      }
      if (worst.beats(bestFollowUp)) worst = bestFollowUp; //it is the cat picking the reply that is the worst for the catcher
    }
    return worst;
  };

  //it is the lookahead where it plays out the 5 best blocks and picks the one with the best worst case
  //it is because the cat will always pick the reply that hurts the catcher the most so a block is only as good as its worst case
  Point2D best = ranked[0].second;
  Score bestWorst = kEscaped;
  bool found = false;
  for (int k = 0; k < kTopBlocks && k < static_cast<int>(ranked.size()); ++k) {
    int bi = index(size, ranked[k].second);
    blocked[bi] = true; //it tries the block
    Score worst = worstCase();
    blocked[bi] = false; //it undoes the block
    //it only switches when the worst case is strictly better so ties go to the block that looked best at first
    if (!found || worst.beats(bestWorst)) {
      best = ranked[k].second;
      bestWorst = worst;
      found = true;
    }
  }
  return best;
}
