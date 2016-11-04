#CXX=clang++-3.5
CXXFLAGS=-Wall -pedantic -std=c++11 -fopenmp

.PHONY: all clean

all: ssl.so

ssl.so: ssl.cpp
	$(CXX) $(CXXFLAGS) -fPIC -shared -o $@ $<

clean:
	rm -rf ssl.so

