#include "graph.hpp"
#include "path.hpp"
#include "tspfile.hpp"
#include "queue.hpp"
#include <atomic>
#include <thread>
#include <limits>
#include <vector> // For std::vector

#define MAX_THREADS 10
#define FINAL_PATH_SIZE 10

//#define ADVANCE_END_CONDITION


static struct {
	std::atomic<uint64_t> shortestInt = std::numeric_limits<uint64_t>::max();
    uint64_t max_depth = 8;
    Queue<Path*> queue;
    Graph* graph;
	int size;
	std::atomic<uint64_t> total;		// number of paths to check
	std::atomic<uint64_t> verified = 0;	// shortest path found so far
	//factorial array
	uint64_t factorial_array[20] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320,
		362880, 3628800, 39916800, 479001600, 6227020800, 87178291200,
		1307674368000, 20922789888000, 355687428096000, 6402373705728000, 121645100408832000};
} global;


typedef struct {
	int verified;

	int count;
	int counter;
	int threadTime;
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

static void branch_and_bound(Path* current, Path* minPath, uint64_t* elimine)
{

	if (current->leaf()) {
		// this is a leaf
    	current->add(0);
#ifdef ADVANCE_END_CONDITION
		(*elimine)++;
#endif
		bool setNewPath = true;
        while(setNewPath){
        	// use atomic compare and set to update the shortest global.sortestInt
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
				(*elimine) += global.factorial_array[global.graph->size() - current->size()];
#endif

		}
	}
}

/*

 */
static void createNextPaths(Path* current, Path* minPath){

	uint64_t nbElimine = 0;

	if (current->size() < global.max_depth) {
      for (int i=0; i<global.graph->size(); i++) {
        if (!current->contains(i)) {
          Path* newPath = new Path(global.graph);
          newPath->copy(current);
          newPath->add(i);

          if(newPath->distance() < global.shortestInt.load(std::memory_order_relaxed)) {
            global.queue.enqueue(newPath);
          } else {
#ifdef ADVANCE_END_CONDITION
          	 nbElimine += global.factorial_array[global.graph->size() - newPath->size()];
#endif
            delete newPath;
          }
        }
      }

    } else {
      branch_and_bound(current, minPath, &nbElimine);
    }

#ifdef ADVANCE_END_CONDITION
	global.verified.fetch_add(nbElimine, std::memory_order_relaxed);
#endif
}

static void threaded_branch_and_bound(int thread_id, Path* minPath, Stats* stat)
{
	auto start_time = std::chrono::high_resolution_clock::now();

#ifdef ADVANCE_END_CONDITION
	while (global.verified < global.total) {
#else
	while (!global.queue.empty()) {
#endif
		Path* current = nullptr;
		try {
			current = global.queue.dequeue(&stat->counter);
            if(current != nullptr){
            	stat->count++;
              createNextPaths(current, minPath);
              delete current;
            }

		} catch (EmptyQueueException& e) {

		}
	}

	if ( true) {
		auto end_time = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		stat->threadTime = duration.count();
    }


}



int main(int argc, char* argv[])
{
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
	Path* p = new Path(g);
	for (int i=0; i<g->size(); i++) {
		p->add(i);
	}
	p->add(0);
    //global.shorts.set(p, 0);

    global.queue = Queue<Path*>();

    global.graph = g;

    if (max_depth != 0) {
      global.max_depth = max_depth;
    } else {
    	if (g->size() <= FINAL_PATH_SIZE + 2) {
      		global.max_depth = FINAL_PATH_SIZE/2;
    	} else {
      		global.max_depth = g->size() - FINAL_PATH_SIZE;
    	}
	}

	// Factoriel of the size of the graph
	global.total = global.factorial_array[g->size()-1];

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
    std::vector<Stats*> stats_vector;

    if (nombreThreads == 0) {
	  nombreThreads = MAX_THREADS;
	}

    for (int i = 0; i < nombreThreads; i++)
      {
      	paths.push_back(new Path(global.graph));
    	stats_vector.push_back(new Stats());
		threads.push_back(std::thread(threaded_branch_and_bound, i, paths[i], stats_vector[i]));
      }


    for (auto &th : threads)
      	th.join();

	// End the timer
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Total Time: " << duration.count() << " milliseconds" << std::endl;


	int i = 0;
	int durations[32];
    for(auto &s : stats_vector){
		std::cout << "Thread:" << i << "\t conccureny occurence:" << s->counter - s->count << "\t duration (ms):" << s->threadTime << std::endl;
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
		std::cout << "End condition : queue empty" << std::endl;
#endif

		for(auto &p : paths){
			if(p->distance() == global.shortestInt.load(std::memory_order_relaxed)){
				std::cout << COLOR.RED << "shortest " << p << COLOR.ORIGINAL << '\n';
			}
		}


	return 0;
}
