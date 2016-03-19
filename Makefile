CXXFLAGS+=--std=c++11 -Wall

.PHONY: all clean run

all: build run

build: main.o
	$(CXX) $(CXXFLAGS) $^ -o run

test: build
	@./run 0 < ./data/example.in

run: build
	@./run 0 < ./data/forever_alone.in

clean:
	@rm -f *.o
	@rm run
