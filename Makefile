CXX := g++-5
CC := ${CXX}
CXXFLAGS := -std=c++14 -O3 -g -Iext/include
LDFLAGS := -lboost_system -lcrypto -lpthread -lboost_thread

main: bullet.o main.o ws.o
ws: ws.o

run: main
	rm -rf out
	mkdir out
	./main

bullet.o: bullet.cpp bullet.hpp
bulletTester.o: bulletTester.cpp bullet.hpp svg.hpp
main.o: main.cpp bullet.hpp svg.hpp ws.hpp
ws.o: ws.cpp ws.hpp

clean:
	rm -rf main bulletTester *.o *.svg out

clean-ext:
	rm -rf external ext
