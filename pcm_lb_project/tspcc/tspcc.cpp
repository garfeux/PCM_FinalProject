#include "graph.hpp"
#include "path.hpp"
#include "tspfile.hpp"
#include "queue.hpp"
#include <atomic>
#include <thread>
#include <limits>
#include <vector> // For std::vector

#define MAX_THREADS 10
#define FINAL_PATH_SIZE 12


enum Verbosity {
	VER_NONE = 0,
	VER_GRAPH = 1,
	VER_SHORTER = 2,
	VER_BOUND = 4,
	VER_ANALYSE = 8,
	VER_COUNTERS = 16,
};

static struct {
	std::atomic<uint64_t> shortestInt = std::numeric_limits<uint64_t>::max();
    uint64_t max_depth = 8;
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

//void setShortest(Path* path){
//  	uint64_t shortest;
//	while(true){
//		if (global.shorts.cas()){
//
//        }
//    }
//}

static void branch_and_bound(Path* current, Path* minPath)
{

    //std::cout << "current b :" << current << std::endl;
	if (global.verbose & VER_ANALYSE)
        std::cout << "analysing" << current << std::endl;


	if (current->leaf()) {
		// this is a leaf

		Path* newPath = new Path(global.graph);
        newPath->copy(current);
        newPath->add(0);


		bool setNewPath = true;
        while(setNewPath){
        	// use atomic compare and set to update the shortest global.sortestInt
        	uint64_t shortest = global.shortestInt.load(std::memory_order_relaxed);
        	uint64_t newPathDistance = newPath->distance();
        	if(shortest > newPathDistance){
            	setNewPath = !global.shortestInt.compare_exchange_strong(shortest, newPathDistance,
                                             std::memory_order_acquire,
                                             std::memory_order_relaxed);
                //std::cout << "shortest: " << shortest << " newPathDistance: " << newPathDistance << std::endl;
                if(setNewPath == false){
                  //std::cout << "shortest: " << shortest << " newPathDistance: " << newPathDistance << std::endl;
                  minPath->copy(newPath);
                }
        	} else {
            	setNewPath = false;
        	}
        }

	} else {
		if (current->distance() < global.shortestInt.load(std::memory_order_relaxed)) {
			// continue branching
			for (int i=1; i<current->max(); i++) {
                //std::cout << "checking " << i << " in " << current << '\n';
				if (!current->contains(i)) {
					//std::cout << "branching " << i << " to " << current << '\n';
          			Path* newPath = new Path(global.graph);
          			newPath->copy(current);
          			newPath->add(i);
	                branch_and_bound(newPath, minPath);
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
}

/*

 */
static void createNextPaths(Path* current, Path* minPath){

	if (current->size() < global.max_depth) {
      for (int i=0; i<global.graph->size(); i++) {
        if (!current->contains(i)) {
          Path* newPath = new Path(global.graph);
          newPath->copy(current);
          newPath->add(i);

          if(newPath->distance() < global.shortestInt.load(std::memory_order_relaxed)) {
            global.queue.enqueue(newPath);
          } else {
            delete newPath;
          }
        }
      }
    } else {
      branch_and_bound(current, minPath);
    }
}

static void threaded_branch_and_bound(int thread_id, Path* minPath)
{
  	//std::cout << "Thread " << thread_id << " started" << std::endl;
	while (true) {
		Path* current = nullptr;
		try {
			current = global.queue.dequeue();
            //std::cout << "Current 1: " << current << std::endl;
            if(current != nullptr){
              createNextPaths(current, minPath);
              delete current;
            }

		} catch (EmptyQueueException& e) {
            //std::cout << "The Queue is empty" << std::endl;
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
	int nombreThreads = 0;
	if (argc == 2) {
		fname = argv[1];
		global.verbose = VER_NONE;
	} else if (argc == 3) {
		fname = argv[1];
		nombreThreads = atoi(argv[2]);
		global.verbose = VER_NONE;
	}

	Graph* g = TSPFile::graph(fname);

    std::cout << "Graph: " << g << std::endl;
    std::cout << "Graph size: " << g->size() << std::endl;

    std::cout << "shortestInt: " << global.shortestInt << std::endl;

    //uint64_t shortest;
	Path* p = new Path(g);
	for (int i=0; i<g->size(); i++) {
		p->add(i);
	}
	p->add(0);
    //global.shorts.set(p, 0);

    global.queue = Queue<Path*>();

    global.graph = g;

    if (g->size() <= FINAL_PATH_SIZE + 2) {
      global.max_depth = FINAL_PATH_SIZE/2;
    } else {
      global.max_depth = g->size() - FINAL_PATH_SIZE;
    }

	Path *path = new Path(global.graph);
    path->add(0);
    //createNextPaths(path);
    //TODO a regarder quand on aura 256 Threads
    for(int i=1; i<g->size(); i++) {
      //createNextPaths(path);
      Path *path2 = new Path(global.graph);
      path2->copy(path);
      path2->add(i);
      createNextPaths(path2, nullptr);
    }
    delete path;

    std::vector<std::thread> threads;
    std::vector<Path*> paths;

    if (nombreThreads == 0) {
	  nombreThreads = MAX_THREADS;
	}

    for (int i = 0; i < nombreThreads; i++)
      {
      	paths.push_back(new Path(global.graph));
		threads.push_back(std::thread(threaded_branch_and_bound, i, paths[i]));
      }


    for (auto &th : threads)
      	th.join();

    for(auto &p : paths){
        if(p->distance() == global.shortestInt.load(std::memory_order_relaxed)){
            std::cout << COLOR.RED << "shortest " << p << COLOR.ORIGINAL << '\n';
        }
    }



	return 0;
}
