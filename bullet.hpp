#pragma once
#include <vector>
#include <cmath>
#include "geometry.hpp"

struct Node;
struct Broadphase;

struct Item {
  virtual Aabb getAabb() const = 0;
  virtual Aabb getPotentialAabb() const { return getAabb(); }
  bool active = true;

 private:
  Node *node;
  friend struct Node;
  friend struct Broadphase;
};

struct Broadphase {
  Broadphase();
  void add(Item *c);
  void remove(Item *c);
  void update();
  //! Return container of pairs of colliding items. Pairs are unordered.
  //! \warning Result may be invalidated by subsequent method calling.
  const std::vector<std::pair<Item *, Item *>> &getCollisions();
  const std::vector<Item *> &getItemsInRange(Aabb);

 private:
  Node *activeRoot;
  Node *inactiveRoot;
  std::vector<std::pair<Item *, Item *>> pairs;
  std::vector<Item *> singles;
  std::vector<Node *> invalidNodes;

  void calcPairs();
  void addNode(Node *, Node *&);
  void removeNode(Node *);
};
