CXXFLAGS+=--std=c++11 -Wall

.PHONY: all clean run

all: build run

build: main.o
	$(CXX) $(CXXFLAGS) $^ -o run

run:
	@./run

clean:
	@rm -f *.o
	@rm run
