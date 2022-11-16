#pragma once
#include <glm/glm.hpp>

struct Bound_Box {
	glm::vec3 center;
	glm::vec3 half_length;
};

//conatiner for triangle
struct Triangle {
	glm::vec3 a, b, c;
};

bool is_intersect(Bound_Box box, Triangle tri);