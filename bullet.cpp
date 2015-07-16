#include "bullet.hpp"

#include <functional>

struct Node {
  Node *parent;
  Node *child[2];
  Aabb aabb;
  Item *item;
  bool childCrossed = false;

  Node(Item *);                        // leaf node
  Node(Node *parent, Node *, Node *);  // branch node
  bool isLeaf() const;
  Node *getSibling() const;
  void updateAabb();
  template <class F>
  void iterate(const F &f);
};

Node::Node(Item *item) {
  this->item = item;
  item->node = this;
  parent = child[0] = child[1] = nullptr;
  updateAabb();
}

Node::Node(Node *parent, Node *n0, Node *n1) {
  this->parent = parent;
  n0->parent = n1->parent = this;
  child[0] = n0;
  child[1] = n1;
  item = nullptr;
  updateAabb();
}

bool Node::isLeaf() const { return item; }

Node *Node::getSibling() const {
  return parent->child[this == parent->child[0]];
}

void Node::updateAabb() {
  aabb = isLeaf() ? item->getPotentialAabb() : child[0]->aabb | child[1]->aabb;
}

template <class F>
void Node::iterate(const F &f) {
  f(this);
  if (!isLeaf()) {
    child[0]->iterate(f);
    child[1]->iterate(f);
  }
}

//
// Broadphase public methods
//

Broadphase::Broadphase() : activeRoot(nullptr), inactiveRoot(nullptr) {}

void Broadphase::add(Item *item) {
  Node *&root = item->active ? activeRoot : inactiveRoot;
  Node *node = new Node(item);
  if (root)
    addNode(node, root);
  else
    root = node;
}

void Broadphase::remove(Item *item) {
  removeNode(item->node);
  delete item->node;
  item->node = nullptr;
}

void Broadphase::update() {
  if (!activeRoot)
    return;

  if (activeRoot->isLeaf()) {
    activeRoot->updateAabb();
    return;
  }

  invalidNodes.clear();
  activeRoot->iterate([this](Node *node) {
    if (node->isLeaf() && !node->aabb.contains(node->item->getAabb()))
      this->invalidNodes.push_back(node);
  });

  for (Node *node : invalidNodes) {
    node->updateAabb();
    removeNode(node);
    addNode(node, activeRoot);
  }
}

const std::vector<std::pair<Item *, Item *>> &Broadphase::getCollisions() {
  calcPairs();
  return pairs;
}

const std::vector<Item *> &Broadphase::getItemsInRange(Aabb aabb) {
  singles.clear();
  std::function<void(Node *)> run = [&](Node *node) {
    if (node->isLeaf()) {
      if (node->item->getAabb() && aabb)
        singles.push_back(node->item);
    } else {
      if (node->aabb && aabb) {
        run(node->child[0]);
        run(node->child[1]);
      }
    }
  };
  if (activeRoot)
    run(activeRoot);
  if (inactiveRoot)
    run(inactiveRoot);
  return singles;
}

//
// Broadphase protected methods
//

void Broadphase::calcPairs() {
  pairs.clear();

  if (!activeRoot)
    return;

  activeRoot->iterate([](Node *n) { n->childCrossed = false; });

  struct {
    Broadphase *b;
    void one(Node *node) {
      if (node->childCrossed || node->isLeaf())
        return;
      pair(node->child[0], node->child[1], true);
      node->childCrossed = true;
    }
    void pair(Node *n0, Node *n1, bool n1Active) {
      one(n0);
      if (n1Active)
        one(n1);
      // TODO: ↑ these two calls can be run in parallel for performance purposes
      if (!(n0->aabb && n1->aabb))
        return;
      if (n0->isLeaf()) {
        if (n1->isLeaf()) {
          if (n0->item->getAabb() && n1->item->getAabb())
            b->pairs.emplace_back(n0->item, n1->item);
        } else {
          pair(n0, n1->child[0], n1Active);
          pair(n0, n1->child[1], n1Active);
        }
      } else {
        if (n1->isLeaf()) {
          pair(n1, n0->child[0], n1Active);
          pair(n1, n0->child[1], n1Active);
        } else {
          for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
              pair(n0->child[i], n1->child[j], n1Active);
        }
      }
    }
  } cross{this};

  if (inactiveRoot)
    cross.pair(activeRoot, inactiveRoot, false);
  else
    cross.one(activeRoot);
}

void Broadphase::addNode(Node *node, Node *&parent) {
  if (parent->isLeaf()) {
    parent = new Node(parent->parent, node, parent);
    return;
  }

  double volumeDiff[2];
  for (int i = 0; i < 2; i++)
    volumeDiff[i] = (parent->child[i]->aabb | node->aabb).volume() -
                    parent->child[i]->aabb.volume();
  // volumeDiff[i] = ((parent->child[i]->aabb | node->aabb) &
  //   parent->child[1-i]->aabb).volume();

  addNode(node, parent->child[volumeDiff[0] > volumeDiff[1]]);
  parent->updateAabb();
}

void Broadphase::removeNode(Node *node) {
  if (node == activeRoot) {
    activeRoot = nullptr;
    return;
  }
  if (node == inactiveRoot) {
    inactiveRoot = nullptr;
    return;
  }
  if (node->parent == activeRoot) {
    Node *sibling = node->getSibling();
    delete activeRoot;
    activeRoot = sibling;
    activeRoot->parent = nullptr;
    return;
  }
  if (node->parent == inactiveRoot) {
    Node *sibling = node->getSibling();
    delete inactiveRoot;
    inactiveRoot = sibling;
    inactiveRoot->parent = nullptr;
    return;
  }
  Node *&parentRef =
      node->parent->parent
          ->child[node->parent != node->parent->parent->child[0]];
  node->getSibling()->parent = node->parent->parent;
  parentRef = node->getSibling();
  delete node->parent;
}
