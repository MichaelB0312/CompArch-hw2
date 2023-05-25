# 046267 Computer Architecture - Winter 20/21 - HW #2
CXX = g++
CXXFLAGS = -std=c++11 -g -Wall 

cacheSim: cacheSim.cpp cache.o 
	$(CXX) -o  $@ cacheSim.cpp cache.o

cache.o: cache.c
	$(CXX) -c $(CXXFLAGS) cache.c 

.PHONY: clean
clean:
	rm -f *.o
	rm -f cacheSim

