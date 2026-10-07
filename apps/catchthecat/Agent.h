#ifndef AGENT_H
#define AGENT_H

#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include <array>
#include <cstdlib>

// Point2D is now glm::ivec2 — same x,y interface, no OOP wrapper needed.
using Point2D = glm::ivec2;

// Hash specialization so Point2D (= glm::ivec2) works in unordered containers.
namespace std {
  template <> struct hash<glm::ivec2> {
    std::size_t operator()(const glm::ivec2& v) const noexcept {
      std::size_t seed = std::hash<int>{}(v.x);
      seed ^= std::hash<int>{}(v.y) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
      return seed;
    }
  };
}  // namespace std

class CatWorld;

class Agent {
public:
  explicit Agent() = default;
  virtual ~Agent() = default;

  virtual Point2D Move(CatWorld*) = 0;

  std::vector<Point2D> generatePath(CatWorld* w);

protected:
  //it is the board as one byte for each cell where 1 means blocked
  //it is used instead of a vector of bool because reading a vector of bool is really slow when the code is built without optimization like the competition does
  using Grid = std::vector<unsigned char>;
  //it turns the world's board into a grid
  static Grid toGrid(const std::vector<bool>& state);

  //it is the escape map that gets built with one bfs from every open border cell at the same time
  //it keeps dist which is how many steps a cell is from the closest open border cell
  //it keeps paths which is how many different shortest ways out a cell has and more ways means it is harder to block
  struct EscapeField {
    std::vector<int> dist; //it is the steps to the closest exit
    std::vector<double> paths; //it is the number of shortest ways out and it uses a double so big numbers do not overflow
  };
  static constexpr int kUnreachable = 1 << 29; //it is a really big number that means the cell can not get out

  //it turns a board position into its spot in the board vector
  static int index(int size, const Point2D& p) { return (p.y + size / 2) * size + (p.x + size / 2); }
  //it checks if a position is on the board
  static bool inside(int size, const Point2D& p) { return std::abs(p.x) <= size / 2 && std::abs(p.y) <= size / 2; }

  //it builds the escape map for the whole board
  static EscapeField computeEscapeField(int size, const Grid& blocked);
  //it is the same but it fills a map that already exists so the loops that run it hundreds of times do not make new memory every time
  static void computeEscapeField(int size, const Grid& blocked, EscapeField& out);
  //it builds the two distance map where every cell is only as good as its second best neighbor because the catcher will block the best one
  //it means the smaller the number the faster the cat can force its way out
  static std::vector<int> computeTwoDistance(int size, const Grid& blocked);
  //it is the same but it fills a map that already exists
  static void computeTwoDistance(int size, const Grid& blocked, std::vector<int>& out);
  //it counts how many open cells can be reached from start and it is used when there is no way out
  static int floodFillSize(int size, const Grid& blocked, const Point2D& start);
  //it checks if there are two routes from start to the edge that do not share any cell
  //it is true exactly when no single block can cut the cell off from the edge so it is a fast way to ask if one block could seal the cat in
  static bool hasTwoSeparateRoutes(int size, const Grid& blocked, int start);

  //it is a lookup table with the six neighbor spots for every cell and a minus one means that neighbor is off the board
  //it stores them in one flat list so the neighbors of cell i are at i times 6 up to i times 6 plus 5
  //it also keeps a list of every border cell so the searches do not have to find them again every time
  struct NeighborTable {
    int size = 0;
    std::vector<int> neighbors;
    std::vector<int> border;
  };
  //it builds the table one time for each board size and then just reuses it so the searches stay fast
  static const NeighborTable& neighborTable(int size);
};

#endif  // AGENT_H







