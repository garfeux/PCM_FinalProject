//
//  graph.hpp
//
//  Copyright (c) 2024 Marcelo Pasin. All rights reserved.
//

#ifndef _graph_hpp
#define _graph_hpp

#include <iostream>
#include <iomanip>
#include <stdio.h>

class Graph {
private: 
	int _max_size;
	int _size;
	int *_distances;
	int *_x;
	int *_y;

public:
	Graph(int size) {
		_max_size = size;
		_distances = new int[size * size];
		for (int i=0; i<size; i++)
			for (int j=0; j<size; j++)
				sdistance(i, j) = -1;
		_x = new int[size];
		_y = new int[size];
		_size = 0;
	}

	~Graph()
	{
		delete _x;
		delete _y;
		delete _distances;
		_x = _y = _distances = 0;
		_max_size = 0;
	}

	int size() const { return _size; }
	int distance(int i, int j) const { return _distances[i + _max_size * j]; }
	int& sdistance(int i, int j) { return _distances[i + _max_size * j]; }
	int add(int x, int y) { _x[_size] = x; _y[_size] = y; return _size ++; }

	void print(std::ostream& os, bool all=true) const
	{
		os << "     ";
		for (int i=(all?0:1); i<_size; i++) {
			char fmt[10];
			snprintf(fmt, 8, "%5d", i);
			os << fmt;
		}
		os << '\n';
		for (int i=0; i<(_size-(all?0:1)); i++) {
			char fmt[10];
			snprintf(fmt, 8, "%5d", i);
			os << fmt;
			for (int j=0; j<(all?0:i); j++)
				os << "   ..";
			for (int j=(all?0:i+1); j<_size; j++) {
				snprintf(fmt, 8, "%5d", distance(i, j));
				os << fmt;
			}
			os << '\n';
		}
	}
};

std::ostream& operator <<(std::ostream& os, Graph* g)
{
	g->print(os);
	return os;
}

#endif // _graph_hpp
