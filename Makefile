CXXFLAGS+=--std=c++11 -Wall -O2

.PHONY: all clean run test_tree pack

all: build pack

pack:
	tar cf submission_source.tar *.cpp *.h

build: main.o tree.o
	$(CXX) $(CXXFLAGS) $^ -o run

test: build
	@./run 0 < ./data/example.in

test_tree: test_tree.o
	$(CXX) $(CXXFLAGS) $^ -o $@
	@./test_tree

run: build
	@echo "Forever alone"
	@./run < ./data/forever_alone.in > ./out/forever_alone.out
	@echo "Constellation"
	@./run < ./data/constellation.in > ./out/constellation.out
	@echo "Overlap"
	@./run < ./data/overlap.in > ./out/overlap.out
	@echo "Weekend"
	@./run < ./data/weekend.in > ./out/weekend.out

clean:
	@rm -f *.o
	@rm run
