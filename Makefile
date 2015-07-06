CXX := g++-5
CC := ${CXX}
CXXFLAGS := -std=c++14 -O3 -g -Iext/include
LDFLAGS := -lboost_system -lcrypto -lpthread -lboost_thread

main: bullet.o main.o ws.o game.o inputEvent.o
ws: ws.o

run: main
	rm -rf out
	mkdir out
	./main

.clang-complete:
	printf "%s %s\n" -stdlib=libc++ "${CXXFLAGS}" > $@

bullet.o: bullet.cpp bullet.hpp
bulletTester.o: bulletTester.cpp bullet.hpp svg.hpp
game.o: game.cpp game.hpp bullet.hpp player.hpp svg.hpp
inputEvent.o: inputEvent.cpp inputEvent.hpp game.hpp bullet.hpp \
 player.hpp bytes.hpp
main.o: main.cpp game.hpp bullet.hpp player.hpp ws.hpp inputEvent.hpp \
 outputEvent.hpp
ws.o: ws.cpp ws.hpp player.hpp bullet.hpp inputEvent.hpp

clean:
	rm -rf main bulletTester *.o *.svg out .clang-complete

clean-ext:
	rm -rf external ext
