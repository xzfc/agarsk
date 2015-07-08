#include "game.hpp"

#include "svg.hpp"

void Modifications::clear() {
  eaten.clear();
  deleted.clear();
  added.clear();
}

static uint32_t cellId = 1;

static uint32_t randomColor() {
  constexpr uint8_t a = 7, b = 255;
  uint32_t yoba = drand48() * (6 * (b-a+1));
  uint32_t x = yoba % (b-a+1);
  switch (yoba / (b-a+1)) {
    case 0: return a <<16 | b <<8 | x;
    case 1: return a <<16 | x <<8 | b;
    case 2: return b <<16 | a <<8 | x;
    case 3: return b <<16 | x <<8 | a;
    case 4: return x <<16 | a <<8 | b;
    case 5: return x <<16 | b <<8 | a;
  }
  return 0xFFFFFF;
}

Cell::Cell(Vec2 pos, unsigned mass) : pos(pos) {  
  this->mass(mass);
  id = cellId++;
  color = randomColor();

  eaten = false;
  updated = true;
}

unsigned Cell::mass()
{ return mass_; }

void Cell::mass(int mass)
{ this->mass_ = mass;
    r = 10. * sqrt((double)mass_); }

Aabb Cell::getAabb() const
{ return {pos.x-r, pos.y-r, pos.x+r, pos.y+r}; }

Aabb Cell::getPotentialAabb() const {
    return type == Type::PELLET ? getAabb() : getAabb().expand(3);
}

void Cell::svg(Svg &s) const
{ s.circle(pos.x, pos.y, r, "none", "rgba(255,0,  0,  0.3)"); }

void Cell::step(Modifications &) {
  eaten = updated = false;
  newMass = mass();
  if (velocity != Vec2 {0,0}) {
    pos += velocity;
    velocity = velocity.shorten(0.01);
    updated = true;
  }
}


PlayerCell::PlayerCell(Player *p, Vec2 pos, unsigned mass)
    : Cell(pos, mass)
{
    player = p;
    type = Type::PLAYER;
    name = player->name;
    player->cells.insert(this);
    player->newCells.push_back(id);
}

PlayerCell::~PlayerCell() {
    player->cells.erase(this);
    // todo:check if player have other cells
}

PlayerCell *PlayerCell::split(Modifications &m, double size) {
    if (player->cells.size() >= 16)
        return nullptr;
    auto newCell = new PlayerCell(player, pos, mass() * size);
    newCell->pos.x += 0.1*(drand48()-0.5);
    newCell->pos.y += 0.1*(drand48()-0.5);
    mass(mass() * (1-size));
    m.added.push_back(newCell);
    return newCell;
}

void PlayerCell::step(Modifications &m) {
    Cell::step(m);
    if (exploded) {
        PlayerCell *c = this;
        do {
            c = c->split(m, 0.5);
        } while(c);
        exploded = false;
    }
}


FoodCell::FoodCell(Vec2 pos, unsigned mass) : Cell(pos, mass) {
    type = Type::FOOD;
}


void Game::addPellets(int count) {
    for (int i = 0; i < count; i++) {
        auto cell = new FoodCell({drand48()*1000, drand48()*1000}, 1);
        cells.insert(cell);
        b.add(cell);
    }
}

void Game::joinPlayer(Player *player) {
    if (player->mode == Player::Mode::GAME) return;
    player->mode = Player::Mode::GAME;

    for(auto i = 0; i < 1; i++) {
      auto cell = new PlayerCell(player, {drand48()*1000, drand48()*1000}, 10);
      cells.insert(cell);
      b.add(cell);
    }
    players.insert(player);
}

void Game::step() {
  mod.clear();
  for (auto c : cells)
    c->step(mod);

  for (auto p : players) {
    for (auto c : p->cells)
      c->velocity = (p->target - c->pos).normalize() * 0.5 * 10;
    p->shoot = false; // TODO
    p->split = false; // TODO
  }
  
  for (auto p : b.getCollisions()) {
    Cell *fstCell = dynamic_cast<Cell*>(p.first),
         *sndCell = dynamic_cast<Cell*>(p.second);
    if (fstCell && sndCell) {
      if (fstCell->r < sndCell->r)
        std::swap(fstCell, sndCell);
      handleInteraction(fstCell, sndCell);
    }
  }

  for (auto c : cells) {
    if (c->eaten) {
      mod.deleted.push_back(c);
      continue;
    }
    if (c->newMass != c->mass()) {
      c->mass(c->newMass);
      c->updated = true;
    }
  }

  for (auto c : mod.deleted) {
    b.remove(c);
    cells.erase(c);
    delete c;
  }
  
  for (auto c : mod.added) {
    cells.insert(c);
    b.add(c);
  }
}

void Game::handleInteraction(Cell *fst, Cell *snd) {
    bool actionEat = false;
    bool actionCollide = false;
    bool actionExplode = false;

    double dist = (fst->pos - snd->pos).length();
    bool canCollide = dist <= fst->r + snd->r;
    bool canEatD = fst->r - dist - 0.354*snd->r >= -11;
    bool canEatR = 4*fst->mass() >= 5*snd->mass();

    auto fstP = dynamic_cast<PlayerCell *>(fst);
    auto sndP = dynamic_cast<PlayerCell *>(snd);

    if (fstP) {
        if (sndP && fstP->player == sndP->player) {
            bool canMerge = false; // TODO
            if (!canMerge && canCollide)
                actionCollide = true;
            else if (canMerge && canEatD)
                actionEat = true;
        } else if (snd->type == Cell::Type::FOOD && canEatD) {
            actionEat = true;
            //if (fst->mass() > 20)
            //  actionExplode = true;
        } else if (canEatD && canEatR) {
            actionEat = true;
            if (snd->type == Cell::Type::VIRUS)
                actionExplode = true;
        }
    } else if (fst->type == Cell::Type::VIRUS &&
               snd->type == Cell::Type::FOOD &&
               canEatD) {
        actionEat = true;
        // TODO: split virus
    }

    if (actionEat && !fst->eaten && !snd->eaten) {
        snd->eaten = true;
        fst->newMass += snd->newMass;
        mod.eaten.push_back({fst->id, snd->id});
    }

    if (actionCollide) {
        auto diff = (fst->pos-snd->pos);
        auto vec = diff.normalize() * (-diff.length()+fst->r+snd->r) * 0.1;
        fst->velocity += vec;
        snd->velocity -= vec;
    }

    if (fstP && actionExplode && !fst->eaten)
        fstP->exploded = true;
}

void Game::stop() {
    for (auto c: cells) {
        b.remove(c);
        delete c;
    }
    cells.clear();
    /*
    for (auto p: players)
      delete p;
    */
    players.clear();
}

void Game::svg(const char *fname) {
    Svg svg(fname, {-100,-100,1000,1000}, 512, 512);
    for (Cell *c : cells)
        c->svg(svg);
}
