//
// Created by robert.archambault on 9/16/2026.
//

#ifndef MOBAGEN_ELLEREXAMPLE_H
#define MOBAGEN_ELLEREXAMPLE_H

#include "../MazeGeneratorBase.h"
#include <string>
#include <vector>
#include "math/Point2D.h"

//Eller's algorithm: processes the maze one row at a time using union-find
// to track which cells in the current row are already connected
class EllerExample : public MazeGeneratorBase {
private:
  int width = 0, height = 0;
  std::vector<int> rowSet;  //it is the set id for each column in the current row
  std::vector<int> parent;  //it is the union-find parent array
  int nextSetId = 0;
  int currentRow = 0;
  bool initialized = false;

  //is is for the visual and is which row was highlighted last, so it can be settled back to the normal path color on the following Step call
  int lastHighlightedRow = -1;
  bool hasHighlight = false;

  int Find(int i);
  void Union(int a, int b);

public:
  EllerExample() = default;
  std::string GetName() override { return "Eller"; };
  bool Step(World* world) override;
  void Clear(World* world) override;
};

#endif  // MOBAGEN_ELLEREXAMPLE_H
