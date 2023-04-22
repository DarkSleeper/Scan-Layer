#include "mipmap.h"
#include <math.h>

MipMap::MipMap(int screen_width, int screen_height, const float* level_0_buffer) {
	max_width = screen_width;
	max_height = screen_height;
	level = 0;
	std::vector<std::vector<float>> level_0_map(max_height, std::vector<float>(max_width));
	for (int i = 0; i < max_height; i++) {
		for (int j = 0; j < max_width; j++) {
			level_0_map[i][j] = level_0_buffer[i * max_width + j];
		}
	}
	datas.push_back(std::move(level_0_map));
	init_data();
}

void MipMap::init_data() {
	int cur_width = max_width;
	int cur_height = max_height;
	while (cur_width > 1 && cur_height > 1) {
		int next_width = (cur_width + 1) / 2;
		int next_height = (cur_height + 1) / 2;
		std::vector<std::vector<float>> next_level_map(next_height, std::vector<float>(next_width));
		auto& src_map = datas[level];

		for (int i = 0; i < cur_height / 2; i++) {
			for (int j = 0; j < cur_width / 2; j++) {
				auto m1 = fmaxf(src_map[2 * i][2 * j], src_map[2 * i][2 * j + 1]);
				auto m2 = fmaxf(src_map[2 * i + 1][2 * j], src_map[2 * i + 1][2 * j + 1]);
				next_level_map[i][j] = fmaxf(m1, m2);
			}
		}
		if (cur_width % 2 == 1) {
			for (int i = 0; i < cur_height / 2; i++) {
				int j = cur_width - 1;
				next_level_map[i][next_width - 1] = fmaxf(src_map[2 * i][j], src_map[2 * i + 1][j]);
			}
		}
		if (cur_height % 2 == 1) {
			for (int j = 0; j < cur_width / 2; j++) {
				int i = cur_height - 1;
				next_level_map[next_height - 1][j] = fmaxf(src_map[i][2 * j], src_map[i][2 * j + 1]);
			}
		}
		if (cur_width % 2 == 1 && cur_height % 2 == 1) {
			next_level_map[next_height - 1][next_width - 1] = src_map[cur_height - 1][cur_width - 1];
		}

		datas.push_back(std::move(next_level_map));
		cur_width = next_width;
		cur_height = next_height;
		level++;
	}
}

float MipMap::get_far_z(glm::vec2 rec_min, glm::vec2 rec_max) {
	int fit_level = 0;
	while (fit_level < level) {
		int a = (int)rec_max.x - (int)rec_min.x;
		int b = (int)rec_max.y - (int)rec_min.y;
		int l = a > b ? a : b;
		if (l == 0) {
			return datas[fit_level][(int)rec_max.y][(int)rec_max.x];
		}
		if (l == 1) {
			float z1 = datas[fit_level][(int)rec_max.y][(int)rec_max.x];
			float z2 = datas[fit_level][(int)rec_max.y][(int)rec_min.x];
			float z3 = datas[fit_level][(int)rec_min.y][(int)rec_max.x];
			float z4 = datas[fit_level][(int)rec_min.y][(int)rec_min.x];
			auto m1 = fmaxf(z1, z2);
			auto m2 = fmaxf(z3, z4);
			return fmaxf(m1, m2);
		}

		rec_min = rec_min / 2.f;
		rec_max = rec_max / 2.f;
		fit_level++;
	}
	return -1; //never reach
}

void MipMap::update_point(float pos_x, float pos_y, float z_value) {
	
}