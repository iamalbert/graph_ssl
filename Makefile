CXX=clang++-3.8
CXXFLAGS=-Wall -pedantic -std=c++11

.PHONY: all clean

all: ssl

clean:
	rm -rf ssl

