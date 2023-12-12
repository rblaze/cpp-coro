all: coro_demo

CXX=clang++
CXXFLAGS+=-std=c++20 -ggdb -W -Wall
OBJS=main.o

main.o: main.cpp task.h

coro_demo: $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -f $(OBJS)
	rm -f coro_demo
