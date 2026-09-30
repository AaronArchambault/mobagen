#include "Catcher.h"
#include "World.h"

Point2D Catcher::Move(CatWorld* world) {
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
}
