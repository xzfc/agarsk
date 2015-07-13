#include "bullet.hpp"

#include <iostream>

struct Node {
  Node *parent;
  Node *child[2];
  Aabb aabb;
  Item *item;
  bool childCrossed = false;

  Node(Item *); // leaf node
  Node(Node *parent, Node*, Node*); // branch node
  bool isLeaf() const;
  Node *getSibling() const;
  void updateAabb();
  template <class F> void iterate(const F &f);
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

bool Node::isLeaf() const {
  return !child[0];
}

Node *Node::getSibling() const {
  return parent->child[this == parent->child[0]];
}

void Node::updateAabb() {
  aabb = isLeaf() ?
         item->getPotentialAabb() :
         child[0]->aabb | child[1]->aabb;
}

template <class F> void Node::iterate(const F &f) {
  f(this);
  if (!isLeaf()) {
    child[0]->iterate(f);
    child[1]->iterate(f);
  }
}

//
// Broadphase public methods
//

Broadphase::Broadphase() : root(nullptr) {
}

void Broadphase::add(Item *item) {
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

const std::vector<std::pair<Item*, Item*>>& Broadphase::getCollisions() {
  updateTree();
  calcPairs();
  return pairs;
}

//
// Broadphase protected methods
//

void Broadphase::updateTree() {
  if (!root)
    return;

  if (root->isLeaf()) {
    root->updateAabb();
    return;
  }

  invalidNodes.clear();
  root->iterate([this](Node *node) {
      if (node->isLeaf() && !node->aabb.contains(node->item->getAabb()))
        this->invalidNodes.push_back(node);
    });
  
  for (Node *node : invalidNodes) {
    node->updateAabb();
    removeNode(node);
    addNode(node, root);
  }
}

void Broadphase::calcPairs() {
  pairs.clear();
  
  if (!root || root->isLeaf())
    return;
  
  root->iterate([](Node *n) { n->childCrossed = false; });
  
  struct {
    Broadphase *b;
    void one(Node *node) {
      if (node->childCrossed || node->isLeaf()) return;
      pair(node->child[0], node->child[1]);
      node->childCrossed = true;
    }
    void pair(Node *n0, Node *n1) {
      one(n0);
      one(n1);
      // TODO: ↑ these two calls can be run in parallel for performance purposes
      if (!(n0->aabb && n1->aabb)) return;
      if (n0->isLeaf()) {
        if (n1->isLeaf()) {
          if (n0->item->getAabb() && n1->item->getAabb())
            b->pairs.emplace_back(n0->item, n1->item);
        } else {
          pair(n0, n1->child[0]);
          pair(n0, n1->child[1]);
        }
      } else {
        if (n1->isLeaf()) {
          pair(n1, n0->child[0]);
          pair(n1, n0->child[1]);
        } else {
          for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
              pair(n0->child[i], n1->child[j]);
        }
      }
    }
  } cross {this};
  
  cross.one(root);
}

void Broadphase::addNode(Node *node, Node *&parent) {
  if (parent->isLeaf()) {
    parent = new Node(parent->parent, node, parent);
    return;
  }

  double volumeDiff[2];
  for (int i = 0; i < 2; i++)
    volumeDiff[i] = (parent->child[i]->aabb | node->aabb).volume()
                    - parent->child[i]->aabb.volume();

  addNode(node, parent->child[volumeDiff[0] > volumeDiff[1]]);
  parent->updateAabb();
}

void Broadphase::removeNode(Node *node) {
  if (!node->parent) {
    root = nullptr;
    return;
  }
  if (node->parent->parent) {
    Node *&parentRef =
        node->parent->parent->child
        [node->parent != node->parent->parent->child[0]];
    node->getSibling()->parent = node->parent->parent;
    parentRef = node->getSibling();
    delete node->parent;
  } else {
    Node *sibling = node->getSibling();
    delete root;
    root = sibling;
    root->parent = nullptr;
  }
}
