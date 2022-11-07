#include "loader/model_loader.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
	string model_name = "runtime/model/robot.obj";
	if (argc == 2) {
		model_name = string("runtime/model/") + argv[1] + ".obj";
	}

	//set model
	ImportedModel my_model(model_name.data());
	int vertex_num = my_model.getNumVertices();
	auto triangle_indexes = my_model.getTriangleIndexes();
	int triangle_num = triangle_indexes.size() / 3;
	auto vertices = my_model.getOriginVertices();
	std::cout << "success!\n";
}
