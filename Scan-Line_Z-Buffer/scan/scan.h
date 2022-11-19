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
	float zl;          
	float dzx;         //-a/c (c!=0)
	float dzy;		   //b/c
	size_t id;
};

struct Scanner {
public:
	Scanner(int screen_width, int screen_height, int screen_scale_z);
	void init(std::vector<glm::vec3>& vertices, std::vector<glm::vec4>& colors);
	void update(unsigned char* frame_buffer, float* z_buffer);

private:
	size_t get_id();
	void clear_table();

	int width;
	int height;
	int scale_z;
	size_t max_id;
	std::vector<std::unordered_map<size_t, PT_Node>> poly_table;
	std::vector<std::unordered_map<size_t, std::vector<ET_Node>>> edge_table;
};