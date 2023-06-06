SOURCES = PTree.cpp
CXX=g++
CXXFLAGS := $(CXXFLAGS) -std=c++17 -fopenmp -Wall -pedantic -march=native -fconcepts 

all: correctness

correctness:
	$(CXX) $(CXXFLAGS) -o ptree main.cpp $(SOURCES)
	
clean:
	rm -f ptree