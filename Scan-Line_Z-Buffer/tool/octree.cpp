#include "octree.h"

Octree::Octree(Bound_Box const& _box, int _depth) {
	box = _box;
	depth = _depth;
	is_leaf = true;
	for (int i = 0; i < 8; i++) {
		children[i] = nullptr;
	}
}

Octree::~Octree() {
	for (int i = 0; i < 8; i++) {
		if (children[i] != nullptr)
			delete children[i];
	}
}

Octree::Octree(Octree const& other)
	: box(other.box), depth(other.depth), is_leaf(other.is_leaf), data(other.data) {
	for (int i = 0; i < 8; i++) {
		children[i] = other.children[i];
	}
}

Octree& Octree::operator=(Octree const& other) {
	box = other.box;
	depth = other.depth;
	is_leaf = other.is_leaf;
	data = other.data;
	for (int i = 0; i < 8; i++) {
		children[i] = other.children[i];
	}
	return *this;
}


Octree* Octree_Constructor::construct(std::vector<int>& triangle_indexes, std::vector<glm::vec3>& vertices) {
	Octree* root;

	Bound_Box root_box;
	{
		glm::vec3 box_max(-INFINITY, -INFINITY, -INFINITY);
		glm::vec3 box_min(INFINITY, INFINITY, INFINITY);
		for (auto& v: vertices) {
			if (v.x > box_max.x) box_max.x = v.x;
			if (v.x < box_min.x) box_min.x = v.x;
			if (v.y > box_max.y) box_max.y = v.y;
			if (v.y < box_min.y) box_min.y = v.y;
			if (v.z > box_max.z) box_max.z = v.z;
			if (v.z < box_min.z) box_min.z = v.z;
		}
		root_box.center = (box_max + box_min) / 2.f;
		root_box.half_length = (box_max - box_min) / 2.f;
	}
	root = new Octree(root_box, 0);

	//todo

	return root;
}