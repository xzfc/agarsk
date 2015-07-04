CXX := g++-5
CC := ${CXX}
CXXFLAGS := -std=c++14 -O3 -g

main: bullet.o main.o

run: main
	rm -rf out
	mkdir out
	./main

bullet.o: bullet.cpp bullet.hpp svg.hpp
main.o: main.cpp bullet.hpp svg.hpp

clean:
	rm -rf main *.o *.svg out
