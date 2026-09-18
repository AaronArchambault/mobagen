#ifndef KRUSKALEXAMPLE_H
#define KRUSKALEXAMPLE_H

#include "../MazeGeneratorBase.h"
#include <string>
#include <vector>
#include "math/Point2D.h"

//this is randomized kruskal, it treats every adjacent pair of cells as a candidate edge, that being
//a wall that could get removed, it shuffles them all and then walks the shuffled list, opening a wall
//whenever it connects two cells that are not already joined into the same region, union find is what
//keeps track of which cells are already connected to which
class KruskalExample : public MazeGeneratorBase {
private:
  struct Edge {
    Point2D a;
    Point2D b;
  };

  std::vector<Edge> edges;
  std::vector<int> parent;  //it is the union find parent array, indexed by y times width plus x
  bool initialized = false;

  //it does union find with path halving and just attaches on union, no rank tracking, i kept it
  //simple since the maze grids are small enough that it does not really matter
  int Find(int i);
  void Union(int a, int b);
  int CellIndex(World* w, const Point2D& p) const;

  //this is the highlight state for the interactive part, it is the pair of cells from the last
  //edge it processed, so it can un highlight them on the next Step instead of leaving stray colors
  Point2D lastHighlightA;
  Point2D lastHighlightB;
  bool hasHighlight = false;

public:
  KruskalExample() = default;
  std::string GetName() override { return "Kruskal"; };
  bool Step(World* world) override;
  void Clear(World* world) override;
};

#endif  //KRUSKALEXAMPLE_H
