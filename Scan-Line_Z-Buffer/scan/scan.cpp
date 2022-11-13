#include "scan.h"
#include <math.h>

Scanner::Scanner(int screen_width, int screen_height) 
{
	width = screen_width;
	height = screen_height;
	poly_table.resize(screen_height);
	edge_table.resize(screen_height);
}

size_t Scanner::get_id() {
	static size_t max_id = 0;
	max_id++;
	return max_id;
}

void Scanner::init(std::vector<int>& triangle_indexes, std::vector<glm::vec3>& vertices, std::vector<glm::vec4>& colors) 
{
	auto in_bound = [](glm::vec3 v) -> bool {
		return (v.x >= -1.f && v.x <= 1.f) && (v.y >= -1.f && v.y <= 1.f) && (v.z >= -1.f && v.z <= 1.f);
	};
	int vertex_num = vertices.size();
	std::vector<bool> is_valid(vertex_num, false);
	int scale_z = (width + height) / 2;
	for (int i = 0; i < vertex_num; i++) {
		if (in_bound(vertices[i])) is_valid[i] = true;
		vertices[i].x = (vertices[i].x + 1) / 2.0f * width;
		vertices[i].y = (vertices[i].y + 1) / 2.0f * height;
		vertices[i].z = (vertices[i].z + 1) / 2.0f * scale_z;
	}

	auto swap = [](glm::vec3& a, glm::vec3& b) -> void {
		auto c = a;
		a = b;
		b = c;
	};

	auto sort = [&] (glm::vec3& a, glm::vec3& b, glm::vec3& c) -> void {
		int i = 0;
		float ymax = a.y;
		if (b.y > ymax) {
			ymax = b.y;
			i = 1;
		}
		if (c.y > ymax) {
			ymax = c.y;
			i = 2;
		}
		if (i == 1) swap(a, b);
		if (i == 2) swap(a, c);
		if (c.y > b.y) swap(b, c);
	};

	//a is higher than b
	auto create_edge = [&](glm::vec3& a, glm::vec3& b, int id, int cut_type = 0) -> ET_Node {
		ET_Node edge;
		edge.id = id;
		edge.dy = (int)a.y - (int)b.y;
		if (cut_type == 1 || cut_type == -1) edge.dy -= 1;
		edge.dx = -(a.x - b.x) / (a.y - b.y);
		float ycut;
		if (cut_type == -1) ycut = a.y - (int)a.y + 1;
		else ycut = a.y - (int)a.y;
		edge.x = a.x + ycut * edge.dx;
		return std::move(edge);
	};

	int triangle_num = triangle_indexes.size() / 3;
	for (int i = 0; i < triangle_num; i++) {
		int _a = triangle_indexes[i];
		int _b = triangle_indexes[i+1];
		int _c = triangle_indexes[i+2];
		if (_a == _b || _a == _c || _b == _c) continue;
		if (!is_valid[_a] || !is_valid[_b] || !is_valid[_c]) continue; //todo: clip, 先简单把有在范围外的点的三角形整个排除
		auto p1 = vertices[_a];
		auto p2 = vertices[_b];
		auto p3 = vertices[_c];
		sort(p1, p2, p3);

		auto id = Scanner::get_id();

		PT_Node poly;
		poly.a = ((p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y));
		poly.b = ((p2.z - p1.z) * (p3.x - p1.x) - (p2.x - p1.x) * (p3.z - p1.z));
		poly.c = ((p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x));
		poly.d = (0 - (poly.a * p1.x + poly.b * p1.y + poly.c * p1.z));
		if (abs(poly.c) < 0.1) continue; //plane is vertical to screen
		int poly_ymax = fmax(p1.y, fmax(p2.y, p3.y));
		int poly_ymin = fmin(p1.y, fmin(p2.y, p3.y));
		poly.dy = poly_ymax - poly_ymin;
		if (poly.dy <= 0)continue;
		poly.id = id;
		poly.color = (colors[_a] + colors[_b] + colors[_c]) / 3.0f;

		ET_Node edge[3];
		int y1 = (int)p1.y;
		int y2 = (int)p2.y;
		int y3 = (int)p3.y;
		if (y1 == y2) {
			edge[0] = create_edge(p1, p3, id);
			edge[1] = create_edge(p2, p3, id);
			edge_table[y1][id].push_back(edge[0]);
			edge_table[y2][id].push_back(edge[1]);
		} else if (y2 == y3) {
			edge[0] = create_edge(p1, p2, id);
			edge[1] = create_edge(p1, p3, id);
			edge_table[y1][id].push_back(edge[0]);
			edge_table[y1][id].push_back(edge[1]);
		} else if (y1 == y2 + 1 && y2 == y3 + 1) {
			//special, split this triangle into two
			auto e13 = create_edge(p1, p3, id);
			glm::vec3 p4;
			p4.x = p1.x + (p1.y - p2.y) * e13.dx;
			p4.y = p2.y;
			p4.z = -1.0f/poly.c * (poly.a * p4.x + poly.b * p4.y + poly.d);

			auto id1 = Scanner::get_id();
			edge[0] = create_edge(p1, p2, id1);
			edge[1] = create_edge(p1, p4, id1);
			edge_table[y1][id1].push_back(edge[0]);
			edge_table[y1][id1].push_back(edge[1]);
			PT_Node poly1 = poly;
			poly1.id = id1;
			poly1.dy = y1 - y2;
			poly_table[y1][id1] = poly1;

			auto id2 = Scanner::get_id();
			edge[0] = create_edge(p2, p3, id2);
			edge[1] = create_edge(p4, p3, id2);
			edge_table[y2][id2].push_back(edge[0]);
			edge_table[y2][id2].push_back(edge[1]);
			PT_Node poly2 = poly;
			poly2.id = id2;
			poly2.dy = y2 - y3;
			poly_table[y2][id2] = poly2;

			continue;
		} else if (y1 == y2 + 1) {
			edge[0] = create_edge(p1, p2, id);
			edge[1] = create_edge(p1, p3, id);
			edge[2] = create_edge(p2, p3, id, -1);
			edge_table[y1][id].push_back(edge[0]);
			edge_table[y1][id].push_back(edge[1]);
			edge_table[y2-1][id].push_back(edge[2]);
		} else if (y2 == y3 + 1) {
			edge[0] = create_edge(p1, p2, id, 1);
			edge[1] = create_edge(p1, p3, id);
			edge[2] = create_edge(p2, p3, id);
			edge_table[y1][id].push_back(edge[0]);
			edge_table[y1][id].push_back(edge[1]);
			edge_table[y2][id].push_back(edge[2]);
		} else {
			edge[0] = create_edge(p1, p2, id);
			edge[1] = create_edge(p1, p3, id);
			edge[2] = create_edge(p2, p3, id, -1);
			edge_table[y1][id].push_back(edge[0]);
			edge_table[y1][id].push_back(edge[1]);
			edge_table[y2-1][id].push_back(edge[2]);
		}

		poly_table[poly_ymax][poly.id] = poly;
	}
}