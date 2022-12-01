#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <time.h>
#include "loader/model_loader.h"
#include "tool/camera.h"
#include "tool/octree.h"
#include "tool/mipmap.h"
#include "scan/scan.h"

#define SCR_WIDTH 1960
#define SCR_HEIGHT 1080
#define STEP 1
const float pai = 3.1415926f;

float toRadians(float degrees) {
	return (degrees * 2.f * pai) / 360.f;
}

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
float speed = 0.033f;

void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

void init_shader(const char* vertexPath, const char* fragmentPath, GLuint& ID);

bool if_scan = true;

int main(int argc, char* argv[]) {
	string model_name = "runtime/model/bunny.obj";
	if (argc == 2) {
		model_name = string("runtime/model/") + argv[1] + ".obj";
	}

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit()) return -1;

	// version 4.6 core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Heat", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, onKeyPress);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	//model
	ImportedModel my_model(model_name.data());
	int vertex_num = my_model.getNumVertices();
	auto triangle_indexes = my_model.getTriangleIndexes();
	int triangle_num = triangle_indexes.size() / 3;
	auto vertices = my_model.getOriginVertices();
	auto normals = my_model.getNormals();

	//octree
	Octree_Constructor oc;
	Octree* root = oc.construct(triangle_indexes, vertices);

	std::vector<int> _box_indexes;
	std::vector<float> _box_vertices;

	auto add_quad = [&](glm::vec3 const& center, glm::vec3 const& half_length, int dimension) {
		int i = _box_vertices.size() / 3;
		glm::vec3 a, b, c, d;
		if (dimension == 3) { // x, y
			a = center + half_length * glm::vec3(-1, -1,  0);
			b = center + half_length * glm::vec3( 1, -1,  0);
			c = center + half_length * glm::vec3( 1,  1,  0);
			d = center + half_length * glm::vec3(-1,  1,  0);
		} else if (dimension == 2) { // x, z
			a = center + half_length * glm::vec3(-1,  0, -1);
			b = center + half_length * glm::vec3( 1,  0, -1);
			c = center + half_length * glm::vec3( 1,  0,  1);
			d = center + half_length * glm::vec3(-1,  0,  1);
		} else if (dimension == 1) { //y, z
			a = center + half_length * glm::vec3( 0, -1, -1);
			b = center + half_length * glm::vec3( 0,  1, -1);
			c = center + half_length * glm::vec3( 0,  1,  1);
			d = center + half_length * glm::vec3( 0, -1,  1);
		}
		_box_vertices.push_back(a.x); _box_vertices.push_back(a.y); _box_vertices.push_back(a.z);
		_box_vertices.push_back(b.x); _box_vertices.push_back(b.y); _box_vertices.push_back(b.z);
		_box_vertices.push_back(c.x); _box_vertices.push_back(c.y); _box_vertices.push_back(c.z);
		_box_vertices.push_back(d.x); _box_vertices.push_back(d.y); _box_vertices.push_back(d.z);
		//abd
		_box_indexes.push_back(i + 0);
		_box_indexes.push_back(i + 1);
		_box_indexes.push_back(i + 3);
		//bcd
		_box_indexes.push_back(i + 1);
		_box_indexes.push_back(i + 2);
		_box_indexes.push_back(i + 3);
	};

	auto add_box = [&](Bound_Box const& box) {
		auto& center = box.center;
		auto& half_length = box.half_length;
		add_quad(center + glm::vec3(0, 0, half_length.z), half_length, 3);
		add_quad(center - glm::vec3(0, 0, half_length.z), half_length, 3);
		add_quad(center + glm::vec3(0, half_length.y, 0), half_length, 2);
		add_quad(center - glm::vec3(0, half_length.y, 0), half_length, 2);
		add_quad(center + glm::vec3(half_length.x, 0, 0), half_length, 1);
		add_quad(center - glm::vec3(half_length.x, 0, 0), half_length, 1);
	};

	GLuint box_vao = {0};
	GLuint box_vbo[1] = {0};
	GLuint box_ebo;
	{
		std::queue<Octree *> q;
		q.push(root);
		while (!q.empty()) {
			auto current = q.front();
			q.pop();
			add_box(current->box);
			if (!current->is_leaf) {
				for (int i = 0; i < 8; i++) {
					q.push(&(current->children[i]));
				}
			}
		}
		
		glGenVertexArrays(1, &box_vao);
		glBindVertexArray(box_vao);

		glGenBuffers(1, &box_ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _box_indexes.size() * sizeof(int), &(_box_indexes[0]), GL_STATIC_DRAW);

		//vert
		glGenBuffers(1, box_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, box_vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, _box_vertices.size() * sizeof(float), &(_box_vertices[0]), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, box_vbo[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}


	//mat 
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	auto aspect = (float)width / (float)height;
	glm::mat4 view_to_clip_mat = glm::perspective(toRadians(45.f), aspect, 1.f, 100.f); //for debug, todo: near-far:=0.1-1000
	glm::mat4 model_mat;
	model_mat = glm::identity<glm::mat4>();
	//fixed view
	glm::vec3 Position(0, 0, 10);
	glm::vec3 Target(0, 0, 0);
	glm::vec3 Up(0, 1, 0);

	unsigned char* img_data = new unsigned char[SCR_WIDTH * SCR_HEIGHT * 4];
	float* z_buffer = new float[SCR_WIDTH * SCR_HEIGHT];
	int scale_z = (SCR_WIDTH + SCR_HEIGHT) / 2;
	std::vector<bool> has_drawed(triangle_num, false);

	////image
	GLuint texture;
	glGenTextures(1, &texture);

	//shader
	auto vertex_path = "runtime/shader/opacity.vs";
	auto fragment_path = "runtime/shader/opacity.fs";
	GLuint renderingProgram;
	init_shader(vertex_path, fragment_path, renderingProgram);

	//vao & vbo
	GLuint vao[1] = {0};
	GLuint vbo[2] = {0};
	GLuint ebo;

	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	std::vector<float> pValues = 
		{-1, -1, 0,   1, -1, 0,
		  1,  1, 0,  -1,  1, 0};
	std::vector<float> tValues =
		{0,0,  1,0,  1,1,  0,1};

	unsigned int indices[] = {
	   0, 1, 3,
	   1, 2, 3
	};

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &(indices[0]), GL_STATIC_DRAW);

	//vert
	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pValues.size() * sizeof(float), &(pValues[0]), GL_STATIC_DRAW);

	//uv
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tValues.size() * sizeof(float), &(tValues[0]), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//display
	GLuint display_program[2];
	vertex_path = "runtime/shader/line.vs";
	fragment_path = "runtime/shader/line.fs";
	init_shader(vertex_path, fragment_path, display_program[1]);

	// timing
	float delta_time = 0.0f;
	float last_time = 0.0f;

	float camera_theta = 0.0f;

	double avg_time = 0.0;
	int frame_cnt = 0;

	//set light
	glm::vec3 direct_light = glm::vec3(1, -1, -1);

	//prepare funcs
	auto setMat4 = [&](const GLuint& program, const std::string& name, const glm::mat4& mat) -> void {
		glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	};
	auto setVec3 = [&](const GLuint& program, const std::string& name, const glm::vec3& value) -> void {
		glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
	};

	for (int i = 0; i < SCR_WIDTH * SCR_HEIGHT; i++) {
		z_buffer[i] = scale_z;
	}
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		auto current_time = (float)glfwGetTime();
		if (last_time == 0.0) {
			delta_time = 0.0;
			last_time = current_time;
		} else {
			delta_time = current_time - last_time;
			//std::cout<<"delta_time:"<<delta_time<<std::endl;
			last_time = current_time;
		}
		processInput(window);

		//Position = glm::vec3(10 * sinf(toRadians(camera_theta)), 0, 10 * cosf(toRadians(camera_theta)));
		glm::mat4 world_to_view_mat = glm::lookAt(Position, Target, Up);
		world_to_view_mat = camera.GetViewMatrix();

		clock_t start, stop;
		start = clock();
		// construct mipmap with z_buffer
		MipMap mm(SCR_WIDTH, SCR_HEIGHT, z_buffer);

		std::vector<glm::vec3> screen_vertices(vertex_num);
		for (int i = 0; i < vertex_num; i++) {
			auto scr_v = view_to_clip_mat * (world_to_view_mat * (model_mat * glm::vec4(vertices[i], 1.0f)));
			screen_vertices[i] = glm::vec3(scr_v.x, scr_v.y, scr_v.z) / scr_v.w;
		}
		std::vector<glm::vec4> colors(vertex_num);
		for (int i = 0; i < vertex_num; i++) {
			auto norm = normals[i];
			glm::normalize(norm);
			auto col = (norm + glm::vec3(1.0f)) / 2.0f * 255.0f;
			colors[i] = glm::vec4((int)col.x, (int)col.y, (int)col.z, 255.0f);
		}

		// clear depth
		for (int i = 0; i < SCR_WIDTH * SCR_HEIGHT; i++) {
			z_buffer[i] = scale_z;
		}
		for (int i = 0; i < SCR_WIDTH * SCR_HEIGHT; i++) {
			auto idx = i;
			img_data[idx * 4 + 0] = (unsigned char)(0);
			img_data[idx * 4 + 1] = (unsigned char)(0);
			img_data[idx * 4 + 2] = (unsigned char)(0);
			img_data[idx * 4 + 3] = (unsigned char)(255);
		}
		for (int i = 0; i < triangle_num; i++) {
			has_drawed[i] = false;
		}

		Scanner scanner(SCR_WIDTH, SCR_HEIGHT, scale_z);
		//add to table
		//z-buffer algorithm and img output
		//todo
		//return false if blocked
		auto in_bound = [](glm::vec3 v) -> bool {
			return (v.x >= -1.f && v.x <= 1.f) && (v.y >= -1.f && v.y <= 1.f) && (v.z >= -1.f && v.z <= 1.f);
		};
		auto test_box = [&](Bound_Box box)->bool {
			std::vector<glm::vec3> box_verts;
			for (int i = 0; i < 8; i++) {
				glm::vec3 k(1, 1, 1);
				if ((i & 1) != 0) k.x = -1;
				if ((i & 2) != 0) k.y = -1;
				if ((i & 4) != 0) k.z = -1;
				box_verts.push_back(box.center + box.half_length * k);
			}
			float near_z = scale_z;
			glm::vec2 rec_max(0, 0), rec_min(SCR_WIDTH, SCR_HEIGHT);
			for (int i = 0; i < 8; i++) {
				auto scr_v = view_to_clip_mat * (world_to_view_mat * (model_mat * glm::vec4(box_verts[i], 1.0f)));
				box_verts[i] = glm::vec3(scr_v.x, scr_v.y, scr_v.z) / scr_v.w; 
				if (!in_bound(box_verts[i])) return true;
				box_verts[i].x = (box_verts[i].x + 1) / 2.0f * SCR_WIDTH;
				box_verts[i].y = (box_verts[i].y + 1) / 2.0f * SCR_HEIGHT;
				box_verts[i].z = (box_verts[i].z + 1) / 2.0f * scale_z;
				if (box_verts[i].z < near_z) {
					near_z = box_verts[i].z;
				}
				rec_min.x = fminf(rec_min.x, box_verts[i].x);
				rec_min.y = fminf(rec_min.y, box_verts[i].y);
				rec_max.x = fmaxf(rec_max.x, box_verts[i].x);
				rec_max.y = fmaxf(rec_max.y, box_verts[i].y);
			}
			float mm_far_z = mm.get_far_z(rec_min, rec_max);
			if (mm_far_z < near_z) return false;
			else return true;
		};
		{
			std::queue<Octree*> q;
			q.push(root);
			int cnt = 0;
			while (!q.empty()) {
				auto current = q.front();
				q.pop();
				if (!test_box(current->box))
				{
					cnt++;
					continue;
				}
				if (current->is_leaf && current->data.size() > 0) {
					std::vector<glm::vec3> vs;
					std::vector<glm::vec4> cs;
					for (auto node: current->data) {
						auto t_offset = node.id;
						if (has_drawed[t_offset]) continue;
						has_drawed[t_offset] = true;
						auto idx1 = triangle_indexes[t_offset * 3];
						auto idx2 = triangle_indexes[t_offset * 3 + 1];
						auto idx3 = triangle_indexes[t_offset * 3 + 2];
						vs.push_back(screen_vertices[idx1]);
						vs.push_back(screen_vertices[idx2]);
						vs.push_back(screen_vertices[idx3]);
						cs.push_back(colors[idx1]);
						cs.push_back(colors[idx2]);
						cs.push_back(colors[idx3]);
					}
					if (vs.size() != 0) {
						scanner.init(vs, cs);
						scanner.update(img_data, z_buffer);
					}
				}
				if (!current->is_leaf) {
					for (int i = 0; i < 8; i++) {
						q.push(&(current->children[i]));
					}
				}
			}
			//cout << cnt << endl;
		}

		stop = clock();
		double duration = (double)(stop - start) / CLOCKS_PER_SEC * 1000; //ms
		//std::cout << duration << std::endl;
		string title = "time per frame :" + to_string((int)duration);
		glfwSetWindowTitle(window, title.data());
		//if (camera_theta > 60) {
		//	avg_time += duration;
		//	frame_cnt++;
		//}
		//camera_theta += STEP;
		//if (camera_theta >= 360 + 60) break;

		//image
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glm::mat4 view_mat = camera.GetViewMatrix();
		glm::mat4 inv_world_mat = glm::inverse(view_mat);

		/* Render here */
		if (if_scan) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0.0f, 0.0f, 0.0f, 1.f);

			glUseProgram(renderingProgram);

			glBindVertexArray(vao[0]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glUniform1i(glGetUniformLocation(renderingProgram, "colorMap"), 0);
		
			glDisable(GL_DEPTH_TEST);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		} else {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0.0f, 0.0f, 0.0f, 1.f);

			glUseProgram(display_program[1]);
			glBindVertexArray(box_vao);
			setMat4(display_program[1], "view_to_clip_matrix", view_to_clip_mat);
			setMat4(display_program[1], "world_to_view_matrix", view_mat);
			setMat4(display_program[1], "model", model_mat);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL); 
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLES, _box_indexes.size(), GL_UNSIGNED_INT, 0);
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();


	//std::cout << "average_time = " << avg_time / frame_cnt << std::endl;
	return 0;
}

void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, speed);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, speed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, speed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, speed);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		camera.ProcessMouseMovement(xoffset, yoffset);
}




// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
void checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}

void init_shader(const char* vertexPath, const char* fragmentPath, GLuint& ID) {
	std::string glsl_version = "#version 450\n";
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << glsl_version;
		fShaderStream << glsl_version;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");
	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}