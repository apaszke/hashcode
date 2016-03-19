CXXFLAGS+=--std=c++11 -Wall

.PHONY: all clean run

all: build run

build: main.o
	$(CXX) $(CXXFLAGS) $^ -o run

test: build
	@./run < ./data/example.in

run: build
	@./run < ./data/forever_alone.in

clean:
	@rm -f *.o
	@rm run
