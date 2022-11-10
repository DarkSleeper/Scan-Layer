#include "scan.h"
#include <math.h>

Scanner::Scanner(int screen_width, int screen_height) 
{
	width = screen_width;
	height = screen_height;
	poly_table.resize(screen_height);
	edge_table.resize(screen_height);
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
		std::cout << p1.y << "  " << p2.y << "  " << p3.y << "\n";

		PT_Node poly;
		poly.a = ((p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y));
		poly.b = ((p2.z - p1.z) * (p3.x - p1.x) - (p2.x - p1.x) * (p3.z - p1.z));
		poly.c = ((p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x));
		poly.d = (0 - (poly.a * p1.x + poly.b * p1.y + poly.c * p1.z));
		if (abs(poly.c) < 0.1) continue; //plane is vertical to screen
		int ymax = fmax(p1.y, fmax(p2.y, p3.y));
		int ymin = fmin(p1.y, fmin(p2.y, p3.y));
		poly.dy = ymax - ymin;
		if (poly.dy <= 0)continue;
		poly.id = i;
		poly.color = (colors[_a] + colors[_b] + colors[_c]) / 3.0f;
		poly_table[ymax].push_back(poly);

		ET_Node edge[3];
		edge[0].id = i;
		edge[1].id = i;
		edge[2].id = i;

	}
}