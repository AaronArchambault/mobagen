#include "Cat.h"
#include "World.h"
#include <stdexcept>

Point2D Cat::Move(CatWorld* world) {
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
  return CatWorld::NE(pos);
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
}
