CXX := g++-5
CC := ${CXX}
CXXFLAGS := -std=c++14 -O3 -g -Iext/include -Wall -Wextra -Wno-unused-parameter
LDFLAGS := -lboost_system -lcrypto -lpthread -lboost_thread

main: bullet.o main.o ws.o game.o inputEvent.o outputEvent.o

.clang-complete:
	printf "%s\n" "-stdlib=libc++ ${CXXFLAGS}" > $@

.deps: #*.cpp
	${CXX} ${CXXFLAGS} -MM *.cpp > .deps

-include .deps

reformat:
	clang-format -i *.cpp *.hpp

clean:
	rm -rf main *.o .deps .clang-complete

clean-ext:
	rm -rf external ext
