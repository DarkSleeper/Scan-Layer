#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "loader/model_loader.h"
#include "tool/camera.h"
#include "scan/scan.h"

#define SCR_WIDTH 1960
#define SCR_HEIGHT 1080
const float pai = 3.1415926f;

float toRadians(float degrees) {
	return (degrees * 2.f * pai) / 360.f;
}

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
float speed = 0.016f;

void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

void init_shader(const char* vertexPath, const char* fragmentPath, GLuint& ID);

int main(int argc, char* argv[]) {
	string model_name = "runtime/model/robot.obj";
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

	//mat //todo: move into loop
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	auto aspect = (float)width / (float)height;
	glm::mat4 view_to_clip_mat = glm::perspective(toRadians(45.f), aspect, 1.f, 100.f); //for debug, todo: near-far:=0.1-1000
	glm::mat4 model_mat;
	model_mat = glm::identity<glm::mat4>();
	//fixed view
	glm::mat4 world_to_view_mat = camera.GetViewMatrix();

	/*vertices.clear();
	vertex_num = 3;
	vertices.push_back({0,0,0});
	vertices.push_back({1,1,1});
	vertices.push_back({-1,-1,-1});*/
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

	//todo: add to table
	Scanner scanner(SCR_WIDTH, SCR_HEIGHT);
	scanner.init(triangle_indexes, screen_vertices, colors);

	//todo: z-buffer algorithm and img output






	//image
	unsigned char* img_data = new unsigned char[SCR_WIDTH * SCR_HEIGHT * 4];
	for (int i = 0; i < SCR_WIDTH * SCR_HEIGHT; i++) {
		img_data[i * 4 + 0] = (unsigned char)(255);
		img_data[i * 4 + 1] = 0;
		img_data[i * 4 + 2] = 0;
		img_data[i * 4 + 3] = (unsigned char)(255);
	}
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

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

	// timing
	float delta_time = 0.0f;
	float last_time = 0.0f;

	int frame_cnt = 0;
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

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.f);

		glUseProgram(renderingProgram);

		glBindVertexArray(vao[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(renderingProgram, "colorMap"), 0);
		
		glDisable(GL_DEPTH_TEST);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		frame_cnt++;
	}

	glfwDestroyWindow(window);
	glfwTerminate();


	std::cout << "success!\n";
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