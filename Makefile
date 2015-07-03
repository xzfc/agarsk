CXX := g++-5
CC := ${CXX}
CXXFLAGS := -std=c++14 -O3

main: bullet.o main.o

bullet.o: bullet.cpp bullet.hpp svg.hpp
main.o: main.cpp bullet.hpp svg.hpp

clean:
	rm -rf main *.o *.svg
