#include "bullet.hpp"

#include <iostream>
#include <cstdlib>

struct Circle : Item {
  double x, y, r;
  int collides;
  Circle(double x, double y, double r) : x(x), y(y), r(r) {
    collides = 0;
  }
  Aabb getAabb() const override {
    return {x-r, y-r, x+r, y+r};
  }
  Vec2 getSpeed() const override {
    return {0,0};
  }
  void svg(Svg &s) const override {
    static const char *woof[] = {
      "rgba(255,0,  0,  0.8)", // 0: red
      "rgba(255,255,0,  0.8)", // 1: yellow
      "rgba(0,  255,0,  0.8)", // 2: green
      "rgba(0,  255,255,0.8)", // 3: salad
      "rgba(0,  0,  255,0.8)",
      "rgba(255,0,  255,0.8)",
    };
    s.circle(x, y, r, "none", woof[collides % 6]);
  }
};

int main() {
  Broadphase b;
  std::vector<Circle*> circles;
  auto collides = [&]() {
    b.calcPairs();
    for (auto c : circles) c->collides = 0;
    for (auto p : b.pairs) {
      //std::cout << ":3";
      static_cast<Circle*>(p.first)->collides++;
      static_cast<Circle*>(p.second)->collides++;
    }
  };
	
  for (int i = 0; i < 3000; i++) {
    double x = (drand48()-0.5)*2000;
    double y = (drand48()-0.5)*2000;
    circles.push_back(new Circle(x,y,10));
    b.add(circles.back());
  }

  for (int i = 0; i < 1000; i++) {
    for (auto c : circles) {
      c->x += drand48()-0.5;
      c->y += drand48()-0.5;
    }
    b.update();
    b.calcPairs();
  }

  collides();
  std::cout << "[+]\n";b.root->sanityCheck();std::cout << "[-]\n";
  b.svg("test.svg");

  /*
    b.removeNode(circles[0]->node);
    std::cout << "[+]\n";b.root->sanityCheck();std::cout << "[-]\n";
    //b.svg("test0.svg");

    circles[0]->x = (drand48()-0.5)*48;
    b.addNode(circles[0]->node, b.root);
    std::cout << "[+]\n";b.root->sanityCheck();std::cout << "[-]\n";
    //b.svg("test1.svg");
    */

  /*
  //b.update();
  circles[0]->x = (drand48()-0.5)*48;
  b.removeNode(circles[0]->node);
  b.svg("test0.svg");

  //circles[1]->x = (drand48()-0.5)*48;
  //b.update();
  b.svg("test1.svg");
  */
}
