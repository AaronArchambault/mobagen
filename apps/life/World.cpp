#include "World.h"
#include "Random.h"
void World::Resize(int size) { Resize(size, size); }
void World::Resize(int columns, int lines) {
  currentBufferId = 0;
  width = columns;
  height = lines;
  buffer[0].clear();
  buffer[0].resize(columns * lines);
  buffer[1].clear();
  buffer[1].resize(columns * lines);
  //it is for the bonus and it keeps the extra channel in lockstep with the main one so rules that don't use it never have to think about it, and rules that do
  //and so it always find it correctly sized.
  extraBuffer[0].clear();
  extraBuffer[0].resize(columns * lines);
  extraBuffer[1].clear();
  extraBuffer[1].resize(columns * lines);
}
void World::SwapBuffers() {
  currentBufferId = (currentBufferId + 1) % 2;
  for (int i = 0; i < buffer[currentBufferId].size(); i++) buffer[(currentBufferId + 1) % 2][i] = buffer[currentBufferId][i];
  //it is for the bonus and it mirror the same promote-forward step for the extra channel.
  for (int i = 0; i < extraBuffer[currentBufferId].size(); i++)
    extraBuffer[(currentBufferId + 1) % 2][i] = extraBuffer[currentBufferId][i];
}
// todo: improve those set / get accessors
void World::SetNext(Point2D point, bool value) {
  if (point.x < 0) point.x += width;
  if (point.x >= width) point.x %= width;
  if (point.y < 0) point.y += height;
  if (point.y >= height) point.y %= height;
  auto index = point.y * width + point.x;
  auto size = width * height;
  if (index >= size) index %= size;
  buffer[(currentBufferId + 1) % 2][index] = value;
}
// todo: improve those set / get accessors
void World::SetCurrent(Point2D point, bool value) {
  if (point.x < 0) point.x += width;
  if (point.x >= width) point.x %= width;
  if (point.y < 0) point.y += height;
  if (point.y >= height) point.y %= height;
  auto index = point.y * width + point.x;
  auto size = width * height;
  if (index >= size) index %= size;
  buffer[currentBufferId % 2][index] = value;
}
// todo: improve those set / get accessors
bool World::Get(Point2D point) {
  if (point.x < 0) point.x += width;
  if (point.x >= width) point.x %= width;
  if (point.y < 0) point.y += height;
  if (point.y >= height) point.y %= height;
  auto index = point.y * width + point.x;
  auto size = width * height;
  if (index >= size) index %= size;
  return buffer[currentBufferId % 2][index];
}
void World::Randomize() {
  for (auto&& elem : buffer[0]) elem = (Random::Range(0, 1) != 0);

  for (int i = 0; i < buffer[0].size(); i++) buffer[1][i] = buffer[0][i];

  //it is for the bonus it clear the extra channel on randomize so a leftover "Dying" flag
  //from a previous Brian's Brain run can't bleed into a fresh board
  for (auto&& elem : extraBuffer[0]) elem = false;
  for (int i = 0; i < extraBuffer[0].size(); i++) extraBuffer[1][i] = false;
}

//it is the bonus for the extra-channel accessors, mirroring Get/SetNext/SetCurrent
bool World::GetExtra(Point2D point) {
  if (point.x < 0) point.x += width;
  if (point.x >= width) point.x %= width;
  if (point.y < 0) point.y += height;
  if (point.y >= height) point.y %= height;
  auto index = point.y * width + point.x;
  auto size = width * height;
  if (index >= size) index %= size;
  return extraBuffer[currentBufferId % 2][index];
}
void World::SetNextExtra(Point2D point, bool value) {
  if (point.x < 0) point.x += width;
  if (point.x >= width) point.x %= width;
  if (point.y < 0) point.y += height;
  if (point.y >= height) point.y %= height;
  auto index = point.y * width + point.x;
  auto size = width * height;
  if (index >= size) index %= size;
  extraBuffer[(currentBufferId + 1) % 2][index] = value;
}
void World::SetCurrentExtra(Point2D point, bool value) {
  if (point.x < 0) point.x += width;
  if (point.x >= width) point.x %= width;
  if (point.y < 0) point.y += height;
  if (point.y >= height) point.y %= height;
  auto index = point.y * width + point.x;
  auto size = width * height;
  if (index >= size) index %= size;
  extraBuffer[currentBufferId % 2][index] = value;
}

//for the bonus snapshot/restore for undo
std::vector<bool> World::Snapshot() { return buffer[currentBufferId % 2]; }
std::vector<bool> World::SnapshotExtra() { return extraBuffer[currentBufferId % 2]; }
void World::LoadSnapshot(const std::vector<bool>& mainSnapshot, const std::vector<bool>& extraSnapshot) {
  currentBufferId = 0;
  buffer[0] = mainSnapshot;
  buffer[1] = mainSnapshot;
  extraBuffer[0] = extraSnapshot;
  extraBuffer[1] = extraSnapshot;
}
