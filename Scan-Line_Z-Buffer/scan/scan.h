#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>

//classified polygon table
struct PT_Node {
	float a, b, c, d;  //ax + by + cz + d = 0, ��c=0,��ƽ�洹ֱ��ͶӰƽ�棬������
	size_t id;
	int dy;			   //��Խ��ɨ������Ŀ
	glm::vec4 color;   // r, g, b, a
};

//classified edge table
struct ET_Node {
	float x;		   //�϶˵�x���꣬�Ǽ�ֵ����ض�
	float dx;		   // -1/k
	int dy;			   //��Խ��ɨ������Ŀ
	size_t id;
};


//alive polygon list
struct APL_Node {
	float a, b, c, d;  //ax + by + cz + d = 0
	size_t id;
	int dy;			   //��Խ��ʣ��ɨ������Ŀ
	glm::vec4 color;   // r, g, b, a
};

//alive edge list
struct AEL_Node {
	float xl;		   //�󽻵��x����
	float dxl;		   // -1/k for xl
	int dyl;		   //�󽻵����ڱ߿�Խ��ʣ��ɨ������Ŀ
	float xr;
	float dxr;
	int dyr;
	float zl;          //zԽ��Խ��
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