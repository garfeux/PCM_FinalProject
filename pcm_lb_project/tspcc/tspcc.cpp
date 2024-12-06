//
//  tspcc.cpp
//  
//  Copyright (c) 2022 Marcelo Pasin. All rights reserved.
//

#include "graph.hpp"
#include "path.hpp"
#include "tspfile.hpp"
#include "queue.hpp"

#include <thread>

#define MAX_DEPTH 10
#define MAX_THREADS 8

enum Verbosity {
	VER_NONE = 0,
	VER_GRAPH = 1,
	VER_SHORTER = 2,
	VER_BOUND = 4,
	VER_ANALYSE = 8,
	VER_COUNTERS = 16,
};

static struct {
	Path* shortest;
	Verbosity verbose;
    Queue<Path*> queue;
    Graph* graph;
	struct {
		int verified;	// # of paths checked
		int found;	// # of times a shorter path was found
		int* bound;	// # of bound operations per level
	} counter;
	int size;
	int total;		// number of paths to check
	int* fact;
} global;

static const struct {
	char RED[6];
	char BLUE[6];
	char ORIGINAL[6];
} COLOR = {
	.RED = { 27, '[', '3', '1', 'm', 0 },
	.BLUE = { 27, '[', '3', '6', 'm', 0 },
	.ORIGINAL = { 27, '[', '3', '9', 'm', 0 },
};

// create a mutex to print to the console
std::mutex printMutex;
// create a function to print to the console
void print(const std::string& message, Path *path = nullptr)
{
	std::lock_guard<std::mutex> guard(printMutex);
    if (path != nullptr)
		std::cout << message << path << std::endl;
	else
		std::cout << message << std::endl;
}

static void branch_and_bound(Path* current)
{
	if (global.verbose & VER_ANALYSE)
		print("analysing ", current);

	if (current->leaf()) {
		// this is a leaf
		current->add(0);
		if (global.verbose & VER_COUNTERS)
			global.counter.verified ++;
		if (current->distance() < global.shortest->distance()) {
			if (global.verbose & VER_SHORTER)
				std::cout << "shorter: " << current << '\n';
			global.shortest->copy(current);
			if (global.verbose & VER_COUNTERS)
				global.counter.found ++;
		}
		current->pop();
	} else {
		// not yet a leaf
		if (current->distance() < global.shortest->distance()) {
			// continue branching
			for (int i=1; i<current->max(); i++) {
                //std::cout << "checking " << i << " in " << current << '\n';
				if (!current->contains(i)) {
					//std::cout << "branching " << i << " to " << current << '\n';
					current->add(i);
	                Path* newPath = new Path(global.graph);
	                newPath->copy(current);
	                global.queue.enqueue(newPath);
					current->pop();
				}
			}
		} else {
			// current already >= shortest known so far, bound
			if (global.verbose & VER_BOUND )
				std::cout << "bound " << current << '\n';
			if (global.verbose & VER_COUNTERS)
				global.counter.bound[current->size()] ++;
		}
	}
    delete current;
}

static void threaded_branch_and_bound()
{
	while (true) {
		Path* current = nullptr;
		try {
			current = global.queue.dequeue();
			if (current == nullptr)
				break;

			branch_and_bound(current);
		} catch (EmptyQueueException& e) {
			break;
		}
	}
}

void reset_counters(int size)
{
	global.size = size;
	global.counter.verified = 0;
	global.counter.found = 0;
	global.counter.bound = new int[global.size];
	global.fact = new int[global.size];
	for (int i=0; i<global.size; i++) {
		global.counter.bound[i] = 0;
		if (i) {
			int pos = global.size - i;
			global.fact[pos] = (i-1) ? (i * global.fact[pos+1]) : 1;
		}
	}
	global.total = global.fact[0] = global.fact[1];
}

void print_counters()
{
	std::cout << "total: " << global.total << '\n';
	std::cout << "verified: " << global.counter.verified << '\n';
	std::cout << "found shorter: " << global.counter.found << '\n';
	std::cout << "bound (per level):";
	for (int i=0; i<global.size; i++)
		std::cout << ' ' << global.counter.bound[i];
	std::cout << "\nbound equivalent (per level): ";
	int equiv = 0;
	for (int i=0; i<global.size; i++) {
		int e = global.fact[i] * global.counter.bound[i];
		std::cout << ' ' << e;
		equiv += e;
	}
	std::cout << "\nbound equivalent (total): " << equiv << '\n';
	std::cout << "check: total " << (global.total==(global.counter.verified + equiv) ? "==" : "!=") << " verified + total bound equivalent\n";
}

int main(int argc, char* argv[])
{
	char* fname = 0;
	if (argc == 2) {
		fname = argv[1];
        // verbose of all
		global.verbose = (Verbosity) 0x1F;
    	global.verbose = VER_ANALYSE;
	} else {
		if (argc == 3 && argv[1][0] == '-' && argv[1][1] == 'v') {
			global.verbose = (Verbosity) (argv[1][2] ? atoi(argv[1]+2) : 1);
			fname = argv[2];
		} else {
			fprintf(stderr, "usage: %s [-v#] filename\n", argv[0]);
			exit(1);
		}
	}

	Graph* g = TSPFile::graph(fname);

	global.shortest = new Path(g);
	for (int i=0; i<g->size(); i++) {
		global.shortest->add(i);
	}
	global.shortest->add(0);

    global.queue = Queue<Path*>();

    global.graph = g;

    Path *p = new Path(g);
    p->add(0);
    global.queue.enqueue(p);

    std::vector<std::thread> threads;
    for (int i = 0; i < MAX_THREADS; i++)
		threads.push_back(std::thread(threaded_branch_and_bound));

    for (auto &th : threads)
      	th.join();

    std::cout << COLOR.RED << "shortest " << global.shortest << COLOR.ORIGINAL << '\n';

	return 0;
}
