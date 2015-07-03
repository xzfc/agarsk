#pragma once
#include <vector>
#include <algorithm>

struct Vec2 {
  double x,y;
};

struct Aabb {
  double x0, y0, x1, y1;

  Aabb expand(float m) const
  { return Aabb{x0-m,y0-m,x1+m,y1+m}; }

  Aabb operator|(const Aabb &o) const
  { return {std::min(x0, o.x0), std::min(y0, o.y0),
          std::max(x1, o.x1), std::max(y1, o.y1)}; }

  bool collides(const Aabb &o) const
  { return std::max(x0, o.x0) < std::min(x1, o.x1) &&
                                std::max(y0, o.y0) < std::min(y1, o.y1); }

  bool contains(const Aabb &o) const
  { return x0<=o.x0 && y0<=o.y0 && x1>=o.x1 && y1>=o.y1; }

  double volume() const
  { return (x1-x0)*(y1-y0); }

  Vec2 center() const
  { return {(x0+x1)/2, (y0+y1)/2}; }
};

#include "svg.hpp"
struct Node;

struct Item {
  virtual Aabb getAabb() const = 0;
  virtual Vec2 getSpeed() const = 0;
  virtual void svg(Svg &) const = 0;
  Node *node;
};

struct Broadphase {
  void add(Item *c);
  void remove(Item *c);
  void update();
  void svg(const char *) const;

  Node *root = 0;
  double margin = 10;

  std::vector<Node*> invalidNodes;
  void grabInvalidNodes(Node *node);

  std::vector<std::pair<Item*, Item*>> pairs;
  void calcPairs();
  void crossChildren(Node *node);
  void calcPairsHelper(Node *n0, Node *n1);

  void addNode(Node *, Node *&);
  void removeNode(Node *);
};

struct Node {
  Node *parent;
  Node *child[2];

  Aabb aabb;
  Item *item;

  Node();
  bool isLeaf() const;
  void setBranch(Node*, Node*);
  void setLeaf(Item *item);
  void updateAabb(float margin);
  Node *getSibling() const;

  bool childCrossed = false;
  void cleanChildCrossed();

  int svg(Svg &, int lvl=0) const;

  void sanityCheck();
};
