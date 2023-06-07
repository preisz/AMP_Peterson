# requirements on ubuntu
# sudo apt-get build-essentials
CXX=g++

CXXFLAGS=-std=c++17 -O3 -Wall -pedantic -fopenmp -fconcepts

.DEFAULT_GOAL := all

all: FilterLock
 
FilterLock: Makefile FilterLock2.cpp main.cpp
	$(CXX) main.cpp -o FilterLock -v $(CXXFLAGS) 

.PHONY: clean
clean:
	rm FilterLock
