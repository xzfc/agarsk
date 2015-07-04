CXX := g++-5
CC := ${CXX}
CXXFLAGS := -std=c++14 -O3 -g -Iext/include
CFLAGS := -O3 -g -Iext/include
LDFLAGS := -Lext/lib -lwebsockets

main: bullet.o main.o
bulletTester: bulletTester.o bullet.o
ws: ws.o

run: main
	rm -rf out
	mkdir out
	./main

bullet.o: bullet.cpp bullet.hpp svg.hpp
bulletTester.o: bulletTester.cpp bullet.hpp svg.hpp
main.o: main.cpp bullet.hpp svg.hpp

clean:
	rm -rf main bulletTester *.o *.svg out
