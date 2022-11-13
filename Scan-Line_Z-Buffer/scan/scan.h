#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>

//classified polygon table
struct PT_Node {
	float a, b, c, d;  //ax + by + cz + d = 0, 若c=0,则平面垂直于投影平面，不考虑
	size_t id;
	int dy;			   //跨越的扫描线数目
	glm::vec4 color;   // r, g, b, a
};

//classified edge table
struct ET_Node {
	float x;		   //上端点x坐标，非极值点需截断
	float dx;		   // -1/k
	int dy;			   //跨越的扫描线数目
	size_t id;
};


//alive polygon list
struct APL_Node {
	float a, b, c, d;  //ax + by + cz + d = 0
	size_t id;
	int dy;			   //跨越的剩余扫描线数目
	glm::vec4 color;   // r, g, b, a
};

//alive edge list
struct AEL_Node {
	float xl;		   //左交点的x坐标
	float dxl;		   // -1/k for xl
	int dyl;		   //左交点所在边跨越的剩余扫描线数目
	float xr;
	float dxr;
	int dyr;
	float zl;          //z越大，越近
	float dzx;         //-a/c (c!=0)
	float dzy;		   //b/c
	size_t id;
};

struct Scanner {
public:
	Scanner(int screen_width, int screen_height);
	static size_t get_id();
	void init(std::vector<int>& triangle_indexes, std::vector<glm::vec3>& vertices, std::vector<glm::vec4>& colors);
	void update(unsigned char* frame_buffer, glm::vec4 background_color);

private:
	int width;
	int height;
	int scale_z;
	std::vector<std::unordered_map<size_t, PT_Node>> poly_table;
	std::vector<std::unordered_map<size_t, std::vector<ET_Node>>> edge_table;
};