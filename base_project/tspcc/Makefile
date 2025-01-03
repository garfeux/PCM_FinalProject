TARGET ?= mac

# --- macOS Toolchain ---
CXX_mac = c++
CC_mac = gcc
CFLAGS_mac = -O3 -Wall --std=c++20 -g
LDFLAGS_mac = -O3 -lm -g

# --- Linux Toolchain ---
CXX_linux = g++
CC_linux = gcc
# Statically link on Linux with atomic support
CFLAGS_linux = -O3 -Wall --std=c++20 -g
LDFLAGS_linux = -O3 -lm -g -static -latomic

# Select compiler and flags based on the target
CXX = $(CXX_$(TARGET))
CC = $(CC_$(TARGET))
CFLAGS = $(CFLAGS_$(TARGET))
LDFLAGS = $(LDFLAGS_$(TARGET))

all: tspcc

tspcc: tspcc.o
	$(CXX) -o tspcc tspcc.o $(LDFLAGS)

tspcc.o: tspcc.cpp graph.hpp path.hpp tspfile.hpp
	$(CXX) $(CFLAGS) -c tspcc.cpp

testatom: testatom.cpp atomicstamped.hpp
	$(CXX) $(CFLAGS) -o testatom testatom.cpp $(LDFLAGS)

testque: testque.cpp queue.hpp
	$(CXX) $(CFLAGS) -o testque testque.cpp $(LDFLAGS)

omp:
	make tspcc TARGET=$(TARGET) CFLAGS="-fopenmp -O3" LDFLAGS="-fopenmp -O3"

clean:
	rm -f *.o tspcc atomic testatom omp testque

atomic: atomic.cpp atomicstamped.hpp
	$(CXX) $(CFLAGS) -o atomic atomic.cpp $(LDFLAGS)
