CXXFLAGS+=--std=c++11 -Wall -O2

.PHONY: all clean run test_tree

all: build pack 

pack:
	@tar cf submission_source.tar *.cpp *.h

build: main.o tree.o
	$(CXX) $(CXXFLAGS) $^ -o run

test: build
	@./run 0 < ./data/example.in

test_tree: test_tree.o
	$(CXX) $(CXXFLAGS) $^ -o $@
	@./test_tree

run: build
	@./run 0 < ./data/forever_alone.in

clean:
	@rm -f *.o
	@rm run
