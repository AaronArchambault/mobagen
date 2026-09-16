//
// Created by robert.archambault on 9/16/2026.
//

#include "EllerExample.h"
#include "../World.h"
#include "Random.h"
#include <map>

namespace {
void OpenWallBetween(World* w, const Point2D& a, const Point2D& b)
  {
  Point2D worldA = w->ToWorldCoords(a);
  if (b.x == a.x && b.y == a.y - 1)
    {
    w->SetNorth(worldA, false);
  }
  else if (b.x == a.x + 1 && b.y == a.y)
    {
    w->SetEast(worldA, false);
  }
  else if (b.x == a.x && b.y == a.y + 1)
    {
    w->SetSouth(worldA, false);
  }
  else if (b.x == a.x - 1 && b.y == a.y)
    {
    w->SetWest(worldA, false);
  }
}

const Color32 kPathColor = {1.0f, 1.0f, 1.0f, 1.0f};        //white: settled row
const Color32 kActiveRowColor = {0.4f, 1.0f, 0.4f, 1.0f};   //green: row just processed
}  // namespace

int EllerExample::Find(int i) {
  while (parent[i] != i) {
    parent[i] = parent[parent[i]];  //path halving
    i = parent[i];
  }
  return i;
}

void EllerExample::Union(int a, int b) {
  int ra = Find(a);
  int rb = Find(b);
  if (ra != rb) parent[ra] = rb;
}

void EllerExample::Clear(World* world) {
  rowSet.clear();
  parent.clear();
  nextSetId = 0;
  currentRow = 0;
  initialized = false;
  hasHighlight = false;
}

bool EllerExample::Step(World* w) {
  if (!initialized) {
    width = w->GetWidth();
    height = w->GetHeight();

    //it is the upper bound on distinct set ids ever needed: one per cell, worst case
    parent.resize((size_t)width * height);
    for (int i = 0; i < (int)parent.size(); i++) parent[i] = i;

    nextSetId = 0;
    rowSet.assign(width, 0);
    for (int x = 0; x < width; x++) rowSet[x] = nextSetId++;

    currentRow = 0;
    hasHighlight = false;
    initialized = true;
    return true;
  }

  //settle the previously-highlighted row before starting the next one
  if (hasHighlight) {
    for (int x = 0; x < width; x++) w->SetNodeColor(w->ToWorldCoords(Point2D(x, lastHighlightedRow)), kPathColor);
    hasHighlight = false;
  }

  if (currentRow >= height) return false;  //maze complete

  bool isLastRow = (currentRow == height - 1);

  //horizontal merges and it is so that it join adjacent cells in different sets. On the
  //last row this is forced (no randomness) to guarantee full
  //connectivity, since there's no next row to fix things up later.
  for (int x = 0; x < width - 1; x++)
    {
    int rootA = Find(rowSet[x]);
    int rootB = Find(rowSet[x + 1]);
    if (rootA == rootB) continue;  //already connected would create a cycle
    bool shouldMerge = isLastRow || (Random::Range(0, 1) == 1);
    if (shouldMerge)
      {
      Union(rootA, rootB);
      OpenWallBetween(w, Point2D(x, currentRow), Point2D(x + 1, currentRow));
    }
  }

  if (!isLastRow)
    {
    //it is for the vertical connections and it is for every set must carry at least one member
    //down to the next row (otherwise that set gets sealed off forever),
    //plus some random extras so the maze doesn't look too uniform
    std::vector<bool> connectDown(width, false);
    for (int x = 0; x < width; x++) connectDown[x] = (Random::Range(0, 1) == 1);

    std::map<int, std::vector<int>> membersByRoot;
    for (int x = 0; x < width; x++) membersByRoot[Find(rowSet[x])].push_back(x);

    for (auto& entry : membersByRoot) {
      const std::vector<int>& members = entry.second;
      bool anyConnected = false;
      for (int x : members) anyConnected = anyConnected || connectDown[x];
      if (!anyConnected) {
        int pick = members[Random::Range(0, (int)members.size() - 1)];
        connectDown[pick] = true;
      }
    }

    std::vector<int> nextRowSet(width);
    for (int x = 0; x < width; x++) {
      if (connectDown[x]) {
        OpenWallBetween(w, Point2D(x, currentRow), Point2D(x, currentRow + 1));
        nextRowSet[x] = rowSet[x];  //it so that it carries its set down to the next row
      } else {
        nextRowSet[x] = nextSetId++;  //it starts a fresh set next row
      }
    }
    rowSet = nextRowSet;
  }

  //highlight this row as just-processed and it settles to white on the next Step
  for (int x = 0; x < width; x++) w->SetNodeColor(w->ToWorldCoords(Point2D(x, currentRow)), kActiveRowColor);
  lastHighlightedRow = currentRow;
  hasHighlight = true;

  currentRow++;
  return true;
}
