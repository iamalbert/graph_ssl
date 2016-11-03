CXX=clang++-3.5
CXXFLAGS=-Wall -pedantic -std=c++11 -fopenmp

.PHONY: all clean

all: ssl

clean:
	rm -rf ssl

