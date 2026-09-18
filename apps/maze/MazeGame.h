#ifndef MAZEGAME_H
#define MAZEGAME_H

#include "math/Point2D.h"
#include <vector>

class World;

//this is a playable layer on top of whatever maze is currently generated in World, it does not
//touch World, MazeGeneratorBase, or any generator, it only reads wall state through World's already
//existing public Get and ToWorldCoords functions and draws itself using the same SetNodeColor thing
//the generators already use for their cursor and frontier highlighting, so World::OnDraw() does not
//need any changes to render the player, enemy, and goal

class MazeGame {
public:
  enum class State { Idle, Playing, Won, Caught };

  bool IsActive() const { return active; }
  void SetActive(bool value) { active = value; }
  State GetState() const { return state; }
  Point2D GetPlayerPos() const { return player; }
  Point2D GetEnemyPos() const { return enemy; }
  Point2D GetGoalPos() const { return goal; }
  void SetEnemyMoveInterval(float seconds) { enemyMoveInterval = seconds; }
  //this is true if the goal was not reachable from the players start cell the moment StartGame ran,
  //it almost always just means the generator had not finished running yet, so hit restart again once
  //it finishes generating
  bool MazeLooksIncomplete() const { return mazeLooksIncomplete; }

  //it restarts the game on whatever maze currently exists in world, call this whenever play mode
  //gets turned on, or whenever the maze changes size, since a different size means the old player
  //and enemy state is not valid anymore
  void StartGame(World& world);

  //it queues up a one cell move in whatever direction, it gets consumed on the very next Update,
  //it only keeps one pending direction so whatever got requested most recently is the one that wins,
  //this only gets called from a key press edge in OnGui, not while a key is just held down, so one
  //tap always means exactly one tile, holding the key does not make it slide
  void RequestMove(int dx, int dy);

  void Update(float dt, World& world);

  //it draws the games imgui panel, which is a separate window from World::OnGui's settings panel
  //so they do not conflict, it reads wasd and arrow key state through imguis ImGuiKey api using
  //IsKeyPressed with no repeat, so a press only registers once, and sends it to RequestMove
  void OnGui(World& world);

private:
  bool active = false;
  State state = State::Idle;

  Point2D player{0, 0};
  Point2D enemy{0, 0};
  Point2D goal{0, 0};

  int lastWidth = -1;
  int lastHeight = -1;

  bool hasPendingMove = false;
  int pendingDx = 0, pendingDy = 0;

  float enemyMoveInterval = 0.30f;
  float enemyMoveCooldown = 0.0f;

  bool mazeLooksIncomplete = false;

  void PaintCell(World& world, const Point2D& formalCell, bool isGoalCell);
  void PaintPlayerTrail(World& world, const Point2D& formalCell);
  void ResetBoardColors(World& world);
  bool TryMovePlayer(World& world, int dx, int dy);
  void StepEnemyTowardPlayer(World& world);
  std::vector<Point2D> FindPathAStar(World& world, const Point2D& start, const Point2D& target);
};

#endif  //MAZEGAME_H
