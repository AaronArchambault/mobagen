#ifndef ELLEREXAMPLE_H
#define ELLEREXAMPLE_H

#include "../MazeGeneratorBase.h"
#include <string>
#include <vector>
#include "math/Point2D.h"

//this is eller's algorithm, it processes the maze one row at a time and uses union find to track
//which cells in the current row are already connected, unlike the other generators here it never
//needs to look more than one row ahead, it is the standard way people do infinite or streamed mazes
class EllerExample : public MazeGeneratorBase {
private:
  int width = 0, height = 0;
  std::vector<int> rowSet;  //it is the set id for each column in the current row
  std::vector<int> parent;  //it is the union find parent array
  int nextSetId = 0;
  int currentRow = 0;
  bool initialized = false;

  //this is for the visualization, it tracks which row got highlighted last so it can settle
  //it back to the normal path color on the next Step
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

#endif  // ELLEREXAMPLE_H
