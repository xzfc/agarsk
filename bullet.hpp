#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

inline double sqr(double a) { return a*a; }

struct Vec2 {
  double x = 0., y = 0.;

#define VEC2_OP1(OP) \
  Vec2 operator OP(double s) const              \
  { return {x OP s, y OP s}; }                  \
                                                \
  Vec2 &operator OP##=(double s)                \
  { return *this = *this OP s; }
  
#define VEC2_OP2(OP)                            \
  Vec2 operator OP(const Vec2 &o) const         \
  { return {x OP o.x, y OP o.y}; }              \
                                                \
  Vec2 &operator OP##=(const Vec2 &o)           \
  { return *this = *this OP o; }

  VEC2_OP1(+) VEC2_OP1(-) VEC2_OP1(*) VEC2_OP1(/);
  VEC2_OP2(+) VEC2_OP2(-);

#undef VEC2_OP1
#undef VEC2_OP2

  Vec2 operator+() const
  { return *this; }
  
  Vec2 operator-() const
  { return {-x, -y}; }

  bool operator==(const Vec2 &o) const
  { return x == o.x && y == o.y; }

  bool operator!=(const Vec2 &o) const
  { return !(*this == o); }

  bool zero() const
  { return *this == Vec2(); }

  double length() const
  { return std::sqrt(x*x+y*y); }

  double length2() const
  { return x*x+y*y; }

  Vec2 normalize() const
  { return *this / length(); }

  Vec2 shorten(double s) const
  { double len = length();
    return (len > s) ? Vec2{0,0} : (*this - *this*(s/len)); }
};

inline Vec2 operator+(const Vec2 &a, const Vec2 &b) {
  return {a.x+b.x, a.y+b.y};
}

struct Aabb {
  double x0, y0, x1, y1;

  Aabb expand(float m) const
  { return Aabb{x0-m,y0-m,x1+m,y1+m}; }

  Aabb operator|(const Aabb &o) const
  { return {std::min(x0, o.x0), std::min(y0, o.y0),
          std::max(x1, o.x1), std::max(y1, o.y1)}; }

  bool collides(const Aabb &o) const
  { return std::max(x0, o.x0) < std::min(x1, o.x1) &&
                                std::max(y0, o.y0) < std::min(y1, o.y1) || this->contains(o); }

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
