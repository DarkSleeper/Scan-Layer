#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "intersect.h"

struct Octree {
	Octree(Bound_Box const& _box, int _depth);
	~Octree();
	Octree(Octree const& other);
	Octree& operator=(Octree const& other);

	Bound_Box box;
	int depth;
	bool is_leaf;
	Octree* children[8];
	std::vector<size_t> data; // id pointing to triangle
};

struct Octree_Constructor {
public:
	Octree* construct(std::vector<int>& triangle_indexes, std::vector<glm::vec3>& vertices);
private:
	const int max_depth = 10;
};