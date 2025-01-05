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

//#define ADVANCE_END_CONDITION

static struct {
	std::atomic<uint64_t> shortestInt = std::numeric_limits<uint64_t>::max(); // Shortest distance found so far
    uint64_t max_depth = 8;	// Maximum depth before using branch and bound - updated in main
    Queue<Path*> queue; // Queue of paths to explore
    Graph* graph;		// Graph to explore
	int size;			// Size of the graph
	std::atomic<uint64_t> total;		// Number of paths to check - Atomic integer
	std::atomic<uint64_t> verified = 0;	// Number of paths checked - Atomic integer
	//factorial array
	uint64_t factorial_array[20] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320,
		362880, 3628800, 39916800, 479001600, 6227020800, 87178291200,
		1307674368000, 20922789888000, 355687428096000, 6402373705728000, 121645100408832000};
} global;

// Stats for each thread
typedef struct {
	int verified;
	int count; // Number of reads from the queue
	int counter; // Number of paths checked
	int threadTime; // Time taken by the thread
} Stats;

static const struct {
	char RED[6];
	char BLUE[6];
	char ORIGINAL[6];
} COLOR = {
	.RED = { 27, '[', '3', '1', 'm', 0 },
	.BLUE = { 27, '[', '3', '6', 'm', 0 },
	.ORIGINAL = { 27, '[', '3', '9', 'm', 0 },
};

// Branch and bound algorithm
static void branch_and_bound(Path* current, Path* minPath, uint64_t* elimine)
{
	if (current->leaf()) {
		// this is a leaf
    	current->add(0);

#ifdef ADVANCE_END_CONDITION
		// Increment the number of paths checked
		(*elimine)++;
#endif

		bool setNewPath = true;
        while(setNewPath){
        	// Use atomic compare and set to update the shortest global.sortestInt
        	uint64_t shortest = global.shortestInt.load(std::memory_order_relaxed);
        	uint64_t newPathDistance = current->distance();
        	if(shortest > newPathDistance){
            	setNewPath = !global.shortestInt.compare_exchange_strong(shortest, newPathDistance,
                                             std::memory_order_acquire,
                                             std::memory_order_relaxed);
                if(setNewPath == false){
                  minPath->copy(current);
                }
        	} else {
            	setNewPath = false;
        	}
        }

        current->pop();

	} else {
		// not yet a leaf - check if the current path is shorter than the shortest path found so far
		if (current->distance() < global.shortestInt.load(std::memory_order_relaxed)) {
			// continue branching
			for (int i=1; i<current->max(); i++) {
				if (!current->contains(i)) {
          			current->add(i);
	                branch_and_bound(current, minPath, elimine);
                    current->pop();
				}
			}
		} else {
#ifdef ADVANCE_END_CONDITION
			// current already >= shortest known so far, bound
			// Increment the number of paths checked
			(*elimine) += global.factorial_array[global.graph->size() - current->size()];
#endif

		}
	}
}

/*
	Create the next paths to explore or use branch and bound if the path is at the maximum depth
*/
static void createNextPaths(Path* current, Path* minPath){

	// Number of paths eliminated
	uint64_t nbElimine = 0;

	// If the path is not at the maximum depth, create the next paths to explore
	if (current->size() < global.max_depth) {
      for (int i=0; i<global.graph->size(); i++) {
        if (!current->contains(i)) {
          Path* newPath = new Path(global.graph);
          newPath->copy(current);
          newPath->add(i);

        	// Check if it is shorter than the shortest path found so far, if yes, add it to the queue
          if(newPath->distance() < global.shortestInt.load(std::memory_order_relaxed)) {
            global.queue.enqueue(newPath);
          } else {
#ifdef ADVANCE_END_CONDITION
          	// Increment the number of paths checked
          	nbElimine += global.factorial_array[global.graph->size() - newPath->size()];
#endif
            delete newPath;
          }
        }
      }

    } else {
    	// Use branch and bound
		branch_and_bound(current, minPath, &nbElimine);
    }

#ifdef ADVANCE_END_CONDITION
	// Update the number of paths checked
	global.verified.fetch_add(nbElimine, std::memory_order_relaxed);
#endif
}

/*
	Threaded branch and bound algorithm
*/
static void threaded_branch_and_bound(int thread_id, Path* minPath, Stats* stat)
{
	// Start the timer
	auto start_time = std::chrono::high_resolution_clock::now();

#ifdef ADVANCE_END_CONDITION
	// while the number of paths checked is less than the total number of paths to check
	while (global.verified < global.total) {
#else
	// while the queue is not empty
	while (!global.queue.empty()) {
#endif
		Path* current = nullptr;
		try {
			// Get the next path to explore - counter count the number of attempts to read from the queue
			current = global.queue.dequeue(&stat->counter);
            if(current != nullptr){
            	 // Increment the count if the path is valid
            	stat->count++;
            	// Create the next paths to explore or resolve the path using branch and bound
				createNextPaths(current, minPath);
				delete current;
            }

		} catch (EmptyQueueException& e) {

		}
	}

	// End the timer and compute the time taken by the thread
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	stat->threadTime = duration.count();

}



int main(int argc, char* argv[])
{
	// Parse the command line arguments
	char* fname = 0;
	int nombreThreads = 0;
    int max_depth = 0;
	if (argc == 2) {
		fname = argv[1];
	} else if (argc == 3) {
		fname = argv[1];
		nombreThreads = atoi(argv[2]);
	} else if (argc == 4) {
        fname = argv[1];
        nombreThreads = atoi(argv[2]);
        max_depth = atoi(argv[3]);
    }

	// Start the timer
	auto start_time = std::chrono::high_resolution_clock::now();

	Graph* g = TSPFile::graph(fname);

    std::cout << "Graph size: " << g->size() << std::endl;

    //uint64_t shortest;
	/*Path* p = new Path(g);
	for (int i=0; i<g->size(); i++) {
		p->add(i);
	}
	p->add(0);
    //global.shorts.set(p, 0);*/

	// Initialize the global variables
    global.queue = Queue<Path*>();
    global.graph = g;


	// Determine the maximum depth
    if (max_depth != 0) {
      global.max_depth = max_depth;
    } else {
    	if (g->size() <= FINAL_PATH_SIZE + 2) {
      		global.max_depth = g->size()/2;
    	} else {
      		global.max_depth = g->size() - FINAL_PATH_SIZE;
    	}
	}

	// Factoriel of the size of the graph
	global.total = global.factorial_array[g->size()-1];

	// Create the first paths to explore
	Path *path = new Path(global.graph);
    path->add(0);

    for(int i=1; i<g->size(); i++) {
      Path *path2 = new Path(global.graph);
      path2->copy(path);
      path2->add(i);
      createNextPaths(path2, nullptr);
    }
    delete path;

	// Create the threads, stats, and min path array
    std::vector<std::thread> threads;
    std::vector<Path*> paths;
    std::vector<Stats*> stats_vector;

	// If the number of threads is not specified, use the maximum number of threads
    if (nombreThreads == 0) {
	  nombreThreads = MAX_THREADS;
	}

	// Create the threads
    for (int i = 0; i < nombreThreads; i++)
      {
      	paths.push_back(new Path(global.graph));
    	stats_vector.push_back(new Stats());
		threads.push_back(std::thread(threaded_branch_and_bound, i, paths[i], stats_vector[i]));
      }

	// Wait for the threads to finish
    for (auto &th : threads)
      	th.join();

	// End the timer
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

	// Print the results
	int i = 0;
	int durations[32];
    for(auto &s : stats_vector){
		std::cout << "Thread:" << i << "\t concurrency occurrence:" << s->counter - s->count << "\t duration (ms):" << s->threadTime << std::endl;
    	durations[i] = s->threadTime;
    	i++;
	}
		//compute std deviation of the durations
		double sum = 0;
		for(int i = 0; i < nombreThreads; i++){
			sum += durations[i];
		}
		double mean = sum / nombreThreads;
		double sq_sum = 0;
		for(int i = 0; i < nombreThreads; i++){
			sq_sum += (durations[i] - mean) * (durations[i] - mean);
		}
		double stdev = sqrt(sq_sum / nombreThreads);
		std::cout << "Standard deviation: " << stdev << std::endl;
#ifdef ADVANCE_END_CONDITION
		std::cout << "End condition : count of the path explored" << std::endl;
		std::cout << "total paths: " << global.total << std::endl;
		std::cout << "total exploration: " << global.verified << std::endl;
#else
		std::cout << "End condition : empty queue" << std::endl;
#endif

		for(auto &p : paths){
			if(p->distance() == global.shortestInt.load(std::memory_order_relaxed)){
				std::cout << COLOR.RED << "shortest " << p << COLOR.ORIGINAL << '\n';
			}
		}

    std::cout << "Total Time: " << duration.count() << " milliseconds" << std::endl;


	return 0;
}
