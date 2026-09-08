#ifndef LIFE_WORLD_H
#define LIFE_WORLD_H

#include "math/Point2D.h"
#include <vector>

struct World {
private:
  // double buffer approach to avoid memory reallocation
  std::vector<bool> buffer[2];
  std::vector<bool> extraBuffer[2];
  int currentBufferId;
  int width;
  int height;
  inline std::vector<bool>& currentBuffer() { return buffer[currentBufferId % 2]; }
  inline std::vector<bool>& nextBuffer() { return buffer[(currentBufferId + 1) % 2]; }

public:
  inline const int& Width() const { return width; };
  inline const int& Height() const { return height; };
  // square grids (visual app)
  void Resize(int sideSize);
  // rectangular grids (formal tests): C columns x L lines
  void Resize(int columns, int lines);
  //flips the buffers, promoting the next generation to current. Called by
  //whoever drives the simulation (the demo app's Manager::step or the
  //life-tests runner) right after a rule Step returns - never from inside a
  //rule, which must only write via SetNext.
  void SwapBuffers();
  // todo: make it follow the standard at() function that returns the exactly element
  bool Get(Point2D point);
  // todo: make it follow the standard at() function that returns the exactly element
  void SetNext(Point2D point, bool value);
  void SetCurrent(Point2D point, bool value);
  void Randomize();

  //stuff for more of the bonus thigns like for the muti-state rules
  bool GetExtra(Point2D point);
  void SetNextExtra(Point2D point, bool value);
  void SetCurrentExtra(Point2D point, bool value);

  //other bonus things for the thigns like snapshots the current generation of both of the channels, and only the current
  //buffer is needed and the right afte rswapbuffer both slots of a channel hold the siam entent
  std::vector<bool> Snapshot();
  std::vector<bool> SnapshotExtra();
  //it restores the last taken snapshot pait as the new current genreation and both buffer slots are set to the same content, exactly matching the
  //inveriant swapbuffer maintas, so that a restored world aks the same
  void LoadSnapshot(const std::vector<bool>& mainSnapshot, const std::vector<bool>& extraSnapshot);

};

#endif  // MOBAGEN_WORLD_H
