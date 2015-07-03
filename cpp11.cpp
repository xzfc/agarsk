#include <vector>
#include <iostream>
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
	Node *parent = 0;
	Node *child[2] = {0,0};

	Aabb aabb;
	Item *item = 0;

	Node();
	bool isLeaf() const;
	void setBranch(Node*, Node*);
	void setLeaf(Item *item);
	void updateAabb(float margin);
	Node *getSibling() const;

	bool childCrossed = false;
	void cleanChildCrossed();

	int svg(Svg &, int lvl=0) const;

	void sanityCheck() {
		if (isLeaf()) return;
		if (child[0]->parent != this) std::cout << ":<\n";
		if (child[1]->parent != this) std::cout << ":<\n";
		child[0]->sanityCheck();
		child[1]->sanityCheck();
	}
};




Node::Node() {
	parent = child[0] = child[1] = nullptr;
}

bool Node::isLeaf() const {
	return !child[0];
}

void Node::setBranch(Node *n0, Node *n1) {
	n0->parent = n1->parent = this;
	child[0] = n0;
	child[1] = n1;
	item = nullptr; // is ok?
}

void Node::setLeaf(Item *item) {
	this->item = item;
	item->node = this;
	child[0] = child[1] = nullptr;
}

void Node::updateAabb(float margin) {
	aabb = isLeaf() ?
		item->getAabb().expand(margin) :
		child[0]->aabb | child[1]->aabb;
}

Node *Node::getSibling() const {
	return parent->child[this == parent->child[0]];
}

void Node::cleanChildCrossed() {
	childCrossed = false;
	if (!isLeaf()) {
		child[0]->cleanChildCrossed();
		child[1]->cleanChildCrossed();
	}
}

int Node::svg(Svg &svg, int lvl) const {
	if (isLeaf()) {
		item->svg(svg);
		svg.rectangle(aabb, "rgba(0,0,0,0.5)", "none");
		return 0;
	} else {
		int l = std::max(child[0]->svg(svg,lvl+1), child[1]->svg(svg,lvl+1));
		svg.rectangle(aabb.expand(l*0.3), "rgba(0,127,0,1)", "none");
		svg.line(aabb.center(), child[0]->aabb.center(), "black");
		svg.line(aabb.center(), child[1]->aabb.center(), "black");
		return l+1;
	}
}


void Broadphase::add(Item *item) {
	if (root) {
		Node *node = new Node();
		node->setLeaf(item);
		node->updateAabb(margin);
		addNode(node, root);
	} else {
		root = new Node();
		root->setLeaf(item);
		root->updateAabb(margin);
	}
}

void Broadphase::update() {
	if (!root)
		return;

	if (root->isLeaf()) {
		root->updateAabb(margin);
		return;
	}

	invalidNodes.clear();
	grabInvalidNodes(root);
	for (Node *node : invalidNodes) {
		removeNode(node);
		addNode(node, root);
	}
}

void Broadphase::svg(const char *fname) const {
	Svg svg(fname, {-1000,-1000,1000,1000}, 512, 512);
	if (root)
		root->svg(svg);
}

void Broadphase::grabInvalidNodes(Node *node) {
	if (!node) return;
	if (node->isLeaf()) {
		if (!node->aabb.contains(node->item->getAabb()))
			invalidNodes.push_back(node);
	} else {
		grabInvalidNodes(node->child[0]);
		grabInvalidNodes(node->child[1]);
	}
}

void Broadphase::calcPairs() {
	pairs.clear();
	if (!root || root->isLeaf())
		return;
	root->cleanChildCrossed();
	calcPairsHelper(root->child[0], root->child[1]);
}

void Broadphase::crossChildren(Node *node) {
	if (node->childCrossed) return;
	calcPairsHelper(node->child[0], node->child[1]);
	node->childCrossed = true;
}

void Broadphase::calcPairsHelper(Node *n0, Node *n1) {
	if (!n0->aabb.collides(n1->aabb)) {
		if (!n0->isLeaf()) crossChildren(n0);
		if (!n1->isLeaf()) crossChildren(n1);
		return;
	}
	if (n0->isLeaf()) {
		if (n1->isLeaf()) {
			if (n0->item->getAabb().collides(n1->item->getAabb())) {
				pairs.emplace_back(n0->item, n1->item);
			}
		} else {
			crossChildren(n1);
			calcPairsHelper(n0, n1->child[0]);
			calcPairsHelper(n0, n1->child[1]);
		}
	} else {
		if (n1->isLeaf()) {
			crossChildren(n0);
			calcPairsHelper(n1, n0->child[0]);
			calcPairsHelper(n1, n0->child[1]);
		} else {
			crossChildren(n0);
			crossChildren(n1);
			for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				calcPairsHelper(n0->child[i], n1->child[j]);
		}
	}
}

void Broadphase::addNode(Node *node, Node *&parent) {
	if (parent->isLeaf()) {
		Node *newParent = new Node();
		newParent->parent = parent->parent;
		newParent->setBranch(node, parent);
		parent = newParent;
		parent->updateAabb(margin);
		return;
	}

	double volumeDiff[2];
	for (int i = 0; i < 2; i++)
		volumeDiff[i] = (parent->child[i]->aabb | node->aabb).volume()
		              - parent->child[i]->aabb.volume();

	addNode(node, parent->child[volumeDiff[0] > volumeDiff[1]]);
	parent->updateAabb(margin);
}

void Broadphase::removeNode(Node *node) {
	if (!node->isLeaf()) { int * a = nullptr;a = 0; }
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

#include <cstdlib>

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
