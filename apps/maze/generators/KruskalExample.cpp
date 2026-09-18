#include "KruskalExample.h"
#include "../World.h"
#include "Random.h"
#include <numeric>
#include <utility>

namespace {
void OpenWallBetween(World* w, const Point2D& a, const Point2D& b) {
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

const Color32 kPathColor = {1.0f, 1.0f, 1.0f, 1.0f};    //white, it means part of the finished maze
const Color32 kMergeColor = {1.0f, 0.6f, 0.0f, 1.0f};   //orange, it is the edge that just got processed
}  //namespace

int KruskalExample::CellIndex(World* w, const Point2D& p) const { return p.y * w->GetWidth() + p.x; }

int KruskalExample::Find(int i) {
  while (parent[i] != i)
  {
    parent[i] = parent[parent[i]];  //it does path halving here
    i = parent[i];
  }
  return i;
}

void KruskalExample::Union(int a, int b) {
  int ra = Find(a);
  int rb = Find(b);
  if (ra != rb) parent[ra] = rb;
}

void KruskalExample::Clear(World* world) {
  edges.clear();
  parent.clear();
  initialized = false;
  hasHighlight = false;
}

bool KruskalExample::Step(World* w) {
  if (!initialized)
  {
    int width = w->GetWidth();
    int height = w->GetHeight();

    //it starts every cell out as its own region
    parent.resize((size_t)width * height);
    std::iota(parent.begin(), parent.end(), 0);

    //every adjacent pair of cells becomes a candidate edge, it only does east and south so that
    //each wall between two cells only gets listed once
    edges.clear();
    for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
      {
        if (x + 1 < width) edges.push_back({Point2D(x, y), Point2D(x + 1, y)});
        if (y + 1 < height) edges.push_back({Point2D(x, y), Point2D(x, y + 1)});
      }
    }

    //it does a fisher yates shuffle using Random::Range which is inclusive on both ends
    for (int i = (int)edges.size() - 1; i > 0; i--)
    {
      int j = Random::Range(0, i);
      std::swap(edges[i], edges[j]);
    }

    //kruskal does not really have one current cell the way a walk based generator does, so it just
    //paints every cell as part of the maze right up front
    for (int y = 0; y < height; y++)
      for (int x = 0; x < width; x++) w->SetNodeColor(w->ToWorldCoords(Point2D(x, y)), kPathColor);

    initialized = true;
    return true;
  }

  //it clears the last steps highlight before it processes the next edge
  if (hasHighlight)
  {
    w->SetNodeColor(w->ToWorldCoords(lastHighlightA), kPathColor);
    w->SetNodeColor(w->ToWorldCoords(lastHighlightB), kPathColor);
    hasHighlight = false;
  }

  if (edges.empty()) return false;  //it means there are no edges left so the maze is done

  Edge e = edges.back();
  edges.pop_back();

  int ia = CellIndex(w, e.a);
  int ib = CellIndex(w, e.b);

  if (Find(ia) != Find(ib))
  {
    Union(ia, ib);
    OpenWallBetween(w, e.a, e.b);
    w->SetNodeColor(w->ToWorldCoords(e.a), kMergeColor);
    w->SetNodeColor(w->ToWorldCoords(e.b), kMergeColor);
    lastHighlightA = e.a;
    lastHighlightB = e.b;
    hasHighlight = true;
  }
  //if they are already in the same region it just skips this edge, since opening it would make a
  //cycle, but it still counts as progress since it consumed an edge either way

  return true;
}
