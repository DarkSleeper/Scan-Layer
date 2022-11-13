#include "scan.h"
#include <algorithm>
#include <math.h>
#include <list>

Scanner::Scanner(int screen_width, int screen_height) 
{
	width = screen_width;
	height = screen_height;
	scale_z = (width + height) / 2;
	poly_table.resize(screen_height + 1);
	edge_table.resize(screen_height + 1);
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
		int _a = triangle_indexes[3 * i];
		int _b = triangle_indexes[3 * i + 1];
		int _c = triangle_indexes[3 * i + 2];
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
		//poly.color = glm::vec4(poly.id * 20);

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


bool edge_cmp(ET_Node const& a, ET_Node const& b) {
	if (abs(a.x - b.x) < 1) {
		return a.dx < b.dx;
	} else {
		return a.x < b.x;
	}
}

AEL_Node create_alive_edge(ET_Node const& e1, ET_Node const& e2, PT_Node const& poly, float cur_y) {
	auto alive_edge = AEL_Node();
	alive_edge.xl = e1.x;
	alive_edge.dxl = e1.dx;
	alive_edge.dyl = e1.dy;
	alive_edge.xr = e2.x;
	alive_edge.dxr = e2.dx;
	alive_edge.dyr = e2.dy;
	alive_edge.zl = -1.0f / poly.c * (poly.a * e1.x + poly.b * cur_y + poly.d);
	alive_edge.dzx = -1.0f * poly.a / poly.c;
	alive_edge.dzy = poly.b / poly.c;
	alive_edge.id = poly.id;
	return alive_edge;
}

//vector<map<size_t, PT_Node>> poly_table;
//vector<map<size_t, vector<ET_Node>>> edge_table;
void Scanner::update(unsigned char* frame_buffer, glm::vec4 background_color)
{
	std::unordered_map<size_t, APL_Node> alive_poly_list;
	std::list<AEL_Node> alive_edge_list;

	for (int cur_y = height; cur_y > 0; cur_y--) {
		std::vector<glm::vec4> line_color(width, background_color);
		std::vector<float> line_z(width, scale_z); //0<=z<=scale_z
		//add new poly
		if (poly_table[cur_y].size() > 0) {
			for (auto& iter: poly_table[cur_y]) {
				auto& id = iter.first;
				auto& poly = iter.second;
				auto alive_poly = APL_Node();
				alive_poly.a = poly.a;
				alive_poly.b = poly.b;
				alive_poly.c = poly.c;
				alive_poly.d = poly.d;
				alive_poly.id = poly.id;
				alive_poly.color = poly.color;
				alive_poly.dy = poly.dy;
				alive_poly_list[id] = alive_poly;

				//add new edge
				auto& edges = edge_table[cur_y][id];
				//按x坐标排序, x相同时，dx更大的在右边
				std::sort(edges.begin(), edges.end(), edge_cmp);
				int edge_num = edges.size();
				for (int i = 0; i < edge_num / 2; i++) {
					auto& e1 = edges[2 * i + 0];
					auto& e2 = edges[2 * i + 1];
					auto alive_edge = create_alive_edge(e1, e2, poly, cur_y);
					alive_edge_list.push_back(alive_edge);
				}
			}
		}

		// z update
		for (auto& ae: alive_edge_list) {
			float zx = ae.zl - (ae.xl - (int)ae.xl) * ae.dzx;
			for (float x = ae.xl; x < ae.xr; x++) {
				int idx = x;
				if (idx >= width) break;
				if (idx < 0) continue;
				if (zx < line_z[idx]) {
					line_z[idx] = zx;
					line_color[idx] = alive_poly_list[ae.id].color;
				}
				zx = zx + ae.dzx;
			}
		}

		// alive list update
		for (auto it = alive_poly_list.begin(); it != alive_poly_list.end();) {
			it->second.dy -= 1;
			if (it->second.dy < 0) {
				alive_poly_list.erase(it++);
				continue;
			}
			it++;
		}

		std::vector<AEL_Node> next_edges;
		for (auto it = alive_edge_list.begin(); it != alive_edge_list.end();) {
			it->dyl -= 1;
			it->dyr -= 1;
			if (it->dyl < 0 || it->dyr < 0) {
				//todo
				if (it->dyl < 0 && it->dyr < 0) {
					alive_edge_list.erase(it++);
					continue;
				}
				if (alive_poly_list.count(it->id) == 0) {
					alive_edge_list.erase(it++);
					continue;
				}
				if (cur_y == 1) {
					it++;
					continue;
				}
				auto& poly = alive_poly_list[it->id];
				auto& new_e = edge_table[cur_y - 1][it->id][0];
				if (it->dyl < 0) {
					it->xl = new_e.x;
					it->dxl = new_e.dx;
					it->dyl = new_e.dy;
					it->zl = -1.0f / poly.c * (poly.a * new_e.x + poly.b * (cur_y - 1) + poly.d);
				} else {
					it->xr = new_e.x;
					it->dxr = new_e.dx;
					it->dyr = new_e.dy;
				}
				next_edges.push_back(*it);
				alive_edge_list.erase(it++);
				continue;
			}
			it->xl += it->dxl;
			it->xr += it->dxr;
			it->zl = it->zl + it->dzx * it->dxl + it->dzy;
			it++;
		}
		for (auto& ae : next_edges) {
			alive_edge_list.push_back(ae);
		}

		for (int i = 0; i < width; i++) {
			auto idx = (cur_y - 1) * width + i;
			frame_buffer[idx * 4 + 0] = (unsigned char)(line_color[i].r);
			frame_buffer[idx * 4 + 1] = (unsigned char)(line_color[i].g);
			frame_buffer[idx * 4 + 2] = (unsigned char)(line_color[i].b);
			frame_buffer[idx * 4 + 3] = (unsigned char)(line_color[i].a);
		}
	}
}