CXX := g++-5
CC := ${CXX}
CXXFLAGS := -std=c++17 -O3 -g -Iext/include -Wall -Wextra
LDFLAGS := -lboost_system -lcrypto -lpthread -lboost_thread

main: bullet.o main.o ws.o game.o inputEvent.o outputEvent.o

.clang-complete:
	printf "%s\n" "-stdlib=libc++ ${CXXFLAGS}" > $@

#%.o: %.cpp
#	@printf "%s %s\n" "$<" "$(shell time -f %E ${CXX} ${CXXFLAGS} -c -o $@ $< 2>&1)"

.deps: #*.cpp
	${CXX} ${CXXFLAGS} -MM *.cpp > .deps

-include .deps

clean:
	rm -rf main *.o .deps .clang-complete

clean-ext:
	rm -rf external ext
