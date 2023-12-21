all: coro_demo

CXX=clang++
CXXFLAGS+=-std=c++20 -ggdb -W -Wall -O0
CXXFLAGS+=-fsanitize=address

executor.o: executor.cpp executor.h

libcoro.a: executor.o
	$(AR) rcs $@ $^

main.o: main.cpp coro.h task.h promise.h promise_impl.h executor.h event.h

coro_demo: main.o libcoro.a
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f *.o
	rm -f coro_demo

check:
	clang-tidy -header-filter=.* main.cpp executor.cpp -- $(CXXFLAGS)
