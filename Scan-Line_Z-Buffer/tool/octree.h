#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "intersect.h"

struct Node_Data {
	Triangle t;
	size_t id;
};

struct Octree {
	Octree();
	Octree(Bound_Box const& _box, int _depth);
	void split();
	void insert(Node_Data const& n);

	Bound_Box box;
	int depth;
	bool is_leaf;
	std::vector<Octree> children; // size = 8
	std::vector<Node_Data> data; // id pointing to triangle

	const int max_depth = 7;
	const int max_data_num = 5;
};

struct Octree_Constructor {
	Octree* construct(std::vector<int>& triangle_indexes, std::vector<glm::vec3>& vertices);
};