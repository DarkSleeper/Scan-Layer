#pragma once
#include <glm/glm.hpp>
#include <vector>

struct MipMap {
public:
	MipMap(int screen_width, int screen_height, const float* level_0_buffer);

private:
	void init_data();
	std::vector<std::vector<std::vector<float>>> datas;
	int level;
	int max_width;
	int max_height;
};