CXXFLAGS=-std=c++11 -O3 -g
LDFLAGS=-lgsl

all: benchmark

benchmark: benchmark.o vose.o mvn.o we.o relles.o multi.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cc *.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f benchmark *.o

.PHONY: clean
