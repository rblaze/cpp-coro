all: coro_demo

CXX=clang++
CXXFLAGS+=-std=c++20 -ggdb -W -Wall -O0
OBJS=main.o

CXXFLAGS+=-fsanitize=address

main.o: main.cpp coro.h task.h promise.h executor.h

coro_demo: $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -f $(OBJS)
	rm -f coro_demo

check:
	clang-tidy -header-filter=.* main.cpp -- $(CXXFLAGS)
