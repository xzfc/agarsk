# ◯ Agarsk ◯
## What is it?
Agarsk — yet another FOSS Agar.io server implementation.

## Implemetation details
It is written in C++14.

It uses dynamic AABB trees for collision detection.

By its very nature of game, there are huge amount inactive (not moving) objects (eg., pellets); and small amount of active objects (eg., player cells).
For performance reasons, collisions between active and inactive objects are not checked.
This is implemented by using two AABB trees, one for active objects and one for inactive objects.

## It is playable yet?
No, but you can try.

    connect("ws://89.31.114.117:4947", "")

## How to build and run?

    ./depends.sh
    make
    ./main

## License
MIT.
