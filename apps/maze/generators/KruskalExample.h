//
// Created by robert.archambault on 9/16/2026.
//

#ifndef MOBAGEN_KRUSKALEXAMPLE_H
#define MOBAGEN_KRUSKALEXAMPLE_H

#include "../MazeGeneratorBase.h"
#include <string>
#include <vector>
#include "math/Point2D.h"

//Randomized Kruskal's algorithm: treat every adjacent pair of cells as a
//candidate "edge" (a wall that could be removed), shuffle them, then walk
//the shuffled list opening a wall whenever it connects two cells that
//aren't already joined into the same region. Union-find (disjoint set)
//tracks which cells are already connected to which.
class KruskalExample : public MazeGeneratorBase {
private:
  struct Edge {
    Point2D a;
    Point2D b;
  };

  std::vector<Edge> edges;
  std::vector<int> parent;  //union-find parent array, indexed by y * width + x
  bool initialized = false;

  //union-find with path halving + union-by-attach (no rank tracking, kept
  //simple since maze grids are small enough that this doesn't matter)
  int Find(int i);
  void Union(int a, int b);
  int CellIndex(World* w, const Point2D& p) const;

  //highlight state for the interactive visualization: the pair of cells
  //from the most recently processed edge, so we can un-highlight them
  //on the next Step call instead of leaving stray colors behind
  Point2D lastHighlightA;
  Point2D lastHighlightB;
  bool hasHighlight = false;

public:
  KruskalExample() = default;
  std::string GetName() override { return "Kruskal"; };
  bool Step(World* world) override;
  void Clear(World* world) override;
};



#endif  //MOBAGEN_KRUSKALEXAMPLE_H
