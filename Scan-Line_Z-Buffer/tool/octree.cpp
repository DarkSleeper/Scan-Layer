#include "octree.h"

Octree::Octree() {
	is_leaf = true;
	children.resize(0);
}

Octree::Octree(Bound_Box const& _box, int _depth) {
	box = _box;
	depth = _depth;
	is_leaf = true;
	children.resize(0);
}

void Octree::split() {
	for (int i = 0; i < 8; i++) {
		Bound_Box new_box;
		new_box.center = box.center;
		new_box.center.x += box.half_length.x * (i & 4 ? 0.5f : -0.5f);
		new_box.center.y += box.half_length.y * (i & 2 ? 0.5f : -0.5f);
		new_box.center.z += box.half_length.z * (i & 1 ? 0.5f : -0.5f);
		new_box.half_length = box.half_length * 0.5f;
		children.push_back(Octree(new_box, depth + 1));
	}
	is_leaf = false;
}

void Octree::insert(Node_Data const& n) {
	if (is_leaf) {
		data.push_back(n);

		if (data.size() > max_data_num) {
			//depth judge
			if (depth < max_depth) {
				split();
				for (auto& old_data: data) {
					for (int i = 0; i < 8; i++) {
						if (is_intersect(children[i].box, old_data.t)) {
							children[i].insert(old_data);
						}
					}
				}
				int child_data_num = 0;
				for (int i = 0; i < 8; i++) {
					child_data_num += children[i].data.size();
				}
				if (child_data_num == 8 * data.size()) {
					children.resize(0);
					is_leaf = true;
				} else {
					data.resize(0);
				}
			}
		}
	} else {
		for (int i = 0; i < 8; i++) {
			if (is_intersect(children[i].box, n.t)) {
				children[i].insert(n);
			}
		}
	}
	return;
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

	int triangle_num = triangle_indexes.size() / 3;
	for (auto i = 0; i < triangle_num; i++) {
		int _a = triangle_indexes[3 * i];
		int _b = triangle_indexes[3 * i + 1];
		int _c = triangle_indexes[3 * i + 2];
		Node_Data node;
		node.id = i;
		node.t.a = vertices[_a];
		node.t.b = vertices[_b];
		node.t.c = vertices[_c];
		root->insert(node);
	}

	return root;
}