#include "bullet.hpp"
#include "svg.hpp"
#include <iostream>

struct Rectangle : Item {
  double x, y, r;
  int collides;
  Rectangle(double x, double y, double r) : x(x), y(y), r(r) {
    collides = 0;
  }
  Aabb getAabb() const override {
    return {x-r, y-r, x+r, y+r};
  }
  Aabb getPotentialAabb() const override {
    return getAabb();
  }
  void svg(Svg &s) const override {
    static const char *woof1[] = {
      "rgba(255,0,  0,  0.2)", // 0: red
      "rgba(255,255,0,  0.2)", // 1: yellow
      "rgba(0,  255,0,  0.2)", // 2: green
      "rgba(0,  255,255,0.2)", // 3: salad
      "rgba(0,  0,  255,0.2)",
      "rgba(255,0,  255,0.2)",
    };
    static const char *woof2[] = {
      "rgba(255,0,  0,  0.9)", // 0: red
      "rgba(255,255,0,  0.9)", // 1: yellow
      "rgba(0,  255,0,  0.9)", // 2: green
      "rgba(0,  255,255,0.9)", // 3: salad
      "rgba(0,  0,  255,0.9)",
      "rgba(255,0,  255,0.9)",
    };
    
    //s.circle(x, y, r, woof2[collides % 6], woof1[collides % 6]);
    s.rectangle(getAabb(), woof2[collides % 6], woof1[collides % 6]);
  }
};

struct Tester {
  Broadphase b;
  std::vector<Rectangle *> rectangles;
  
  Tester() {
    for (int i = 0; i < 4000; i++) {
      auto r = new Rectangle(drand48()*1024, drand48()*1024, 1/(0.01+drand48()));
      rectangles.push_back(r);
      b.add(r);
    }
  }
  void shuffle() {
    int i = drand48() * rectangles.size();
    rectangles[i]->x = drand48()*1024;
    rectangles[i]->y = drand48()*1024;
    b.getCollisions();
  }
  void svg(const char *fname) {
    Svg svg(fname, {0,0,1024,1024}, 512, 512);
    for (auto r : rectangles) r->collides = 0;
    auto &collisions = b.getCollisions();
    for (auto p : collisions) {
      static_cast<Rectangle*>(p.first)->collides++;
      static_cast<Rectangle*>(p.second)->collides++;
    }
    for (auto r : rectangles)
      r->svg(svg);
    for (auto p : collisions) {
      svg.line(p.first->getAabb().center(),
               p.second->getAabb().center(), "black");
    }
  }
};
  
int main() {
  Broadphase b;
  Tester t;
  t.svg("out0.svg");
  for (int i = 0; i < 10000; i++)
    t.shuffle();
  t.svg("out1.svg");
}
