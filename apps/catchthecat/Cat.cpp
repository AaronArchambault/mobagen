#include "Cat.h"
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
}





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
