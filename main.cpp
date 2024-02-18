#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <fstream>
#include <vector>
#include <cstdarg>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "Shader.h"

#include <glm/gtc/quaternion.hpp>

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

//Camera settings
float lastX = 400, lastY = 300;
bool firstMouse = true;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), 
	glm::vec3(0.0f, 1.0f, 0.0f), 
	-90.0f, 0.0f, 25000.0f, 0.01f);

Direction direction = Direction::NONE;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	std::cout << " windows resized with " << width << " Height " << height << std::endl;
}

//processing all our key inputs for moving the camera
//this is going to be changed later for oblig 2 when 
//we need to move a player object
void process_key_input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		direction = direction | Direction::FORWARD;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		direction = direction | Direction::BACKWARD;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		direction = direction | Direction::LEFT;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		direction = direction | Direction::RIGHT;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		direction = direction | Direction::UP;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		direction = direction | Direction::DOWN;

	//Press the F key to turn on Wireframe mode, must be held
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

//function for processing the mouse input
void process_mouse_input(GLFWwindow* window, const double x_pos, const double y_pos)
{

	if (firstMouse)
	{
		lastX = x_pos;
		lastY = y_pos;
		firstMouse = false;
	}

	float x_offset = x_pos - lastX;
	float y_offset = lastY - y_pos; // reversed: y ranges bottom to top
	lastX = x_pos;
	lastY = y_pos;

	camera.process_mouse_movement(x_offset, y_offset);
}

void process_direction(double delta_time)
{
	camera.process_keyboard(direction, delta_time);
}

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texture_coord;

	Vertex()
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		color = glm::vec3(0.0f, 0.0f, 0.0f);
		texture_coord = glm::vec2(0.0f, 0.0f);
	}

	Vertex(glm::vec3 position) : Vertex()
	{
		this->position = position;
	}

	Vertex(glm::vec3 position, glm::vec3 color) : Vertex()
	{
		this->position = position;
		this->color = color;
	}

	Vertex(glm::vec3 position, glm::vec3 color, glm::vec2 texture_coord)
	{
		this->position = position;
		this->color = color;
		this->texture_coord = texture_coord;
	}

	void rotate(glm::quat quaternion)
	{
		position = quaternion * position;
	}

	void scale(glm::vec3 scale)
	{
		position.x *= scale.x;
		position.y *= scale.y;
		position.z *= scale.z;
	}

	void translate(glm::vec3 translation)
	{
		position += translation;
	}
};

class file_handle
{
	FILE* file;
public:
	file_handle(const char* path, const char* mode)
	{
		fopen_s(&file, path, mode);
	}
	void printf(_Printf_format_string_ char const* const format, ...) const
	{
		va_list args;
		va_start(args, format);
		vfprintf(file, format, args);
		va_end(args);
	}
	~file_handle()
	{
		fclose(file);
	}
};

//=========================================================//
//                                                         //
//                     _----------_,                       //
//                  ,"__          _-:,                     //
//                 /     ""--_--""...:\                    //
//                /          |.........\                   //
//               /           |..........\                  //
//              /,          _'_........./:                 //
//              ! -,     _-"   "- _...,;;:                 //
//              \   -_- "         "-_/;;;;                 //
//               \   \              /;;;;'                 //
//                \   \            /;;;;                   //
//                 '.  \          /;;;'                    //
//                   "-_\________/;;'                      //
//                                                         //
//=========================================================//

//=========================================================//
//            Oblig 1 beginning (Math and Prog)            //
//=========================================================//

//Code for altering the RGB colors
class hsl
{
	template <typename T>
	static T clamp(T i, T min, T max)
	{
		if (i < min)
			return min;
		if (i > max)
			return max;
		return i;
	}

	static float get_modif(float time, float (*func)(float))
	{
		auto t = func(time);
		if (t < 0)
			t = -t;
		return 0.25f * t + 0.25f;
	}

	template <typename T>
	static T mod(T i, int mod)
	{
		return i - mod * static_cast<int>(i / mod);
	}
public:
	hsl() : h(0), s(0), l(0), rgb{ 0,0,0 }
	{
	}

	hsl(float h, float s, float l) : h(h), s(s), l(l), rgb{ 0,0,0 }
	{
	}

	int h;
	float s;
	float l;
	float rgb[3];
	void shift(float time)
	{
		h = mod(time * 100, 360.0f);
	}
	void from_time(float time)
	{
		h = static_cast<int>(time) % 360;
		s = 1;
		l = clamp(get_modif(time, sin), 0.25f, 0.75f);
	}
	float* get_rgb()
	{
		float c = (1.0f - abs(2.0f * l - 1.0f)) * s;
		float x = c * (1.0f - abs(mod(h / 60.0f, 2) - 1.0f));
		float m = l - c / 2;
		float r, g, b;
		if (h < 60)
		{
			r = c;
			g = x;
			b = 0;
		}
		else if (h < 120)
		{
			r = x;
			g = c;
			b = 0;
		}
		else if (h < 180)
		{
			r = 0;
			g = c;
			b = x;
		}
		else if (h < 240)
		{
			r = 0;
			g = x;
			b = c;
		}
		else if (h < 300)
		{
			r = x;
			g = 0;
			b = c;
		}
		else
		{
			r = c;
			g = 0;
			b = x;
		}
		r += m;
		g += m;
		b += m;
		rgb[0] = r;
		rgb[1] = g;
		rgb[2] = b;
		return rgb;
	}
	glm::vec3 get_rgb_vec3()
	{
		get_rgb();
		return { rgb[0], rgb[1], rgb[2] };
	}
};

//A simple function Class
class func
{
	std::vector<Vertex> vertices;
	std::vector<Vertex> finalVertecies;
	std::vector<unsigned int> indices;

	float fX(float x)
	{
		//A very simple normal function x^2 - 2
		//will be a upside down arch
		return pow(x, 2) - 2;
	}
	float dfx(float x)
	{
		//the derived function, 6x^2
		return 2 * x;
	}
	std::vector<Vertex> getVertices(float min, float max, float step, bool useZ)
	{
		std::vector<Vertex> vertices;
		auto diff = (max + min) / 2;
		while (min < max)
		{
			auto v = Vertex();
			v.position.x = min - diff;
			v.position.y = fX(min);
			if (useZ)
				v.position.z = dfx(min);
			min += step;
			vertices.push_back(v);
		}
		auto v = Vertex();
		v.position.x = min - diff;
		v.position.y = fX(min);
		if (useZ)
			v.position.z = dfx(min);
		vertices.push_back(v);
		return vertices;
	}
	unsigned int vao;
	unsigned int ebo;
	unsigned int vbo;
public:
	func(bool useZ = false)
	{
		vertices = getVertices(-1.f, 1.f, 0.1f, useZ);
		for (int i = 0; i < vertices.size() - 1; i++)
		{
			auto vertex = vertices[i];
			auto nextVertex = vertices[i + 1];
			if (nextVertex.position.y < vertex.position.y)
				vertex.color.r = 1;
			else
				vertex.color.g = 1;
			vertices[i] = vertex;
		}
		FILE* outpFile;
		fopen_s(&outpFile, "function_output.txt", "w");
		fprintf(outpFile, "%d points\n", vertices.size());
		for (auto vertex : vertices)
		{
			fprintf(outpFile, "%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f\n", vertex.position.x, vertex.position.y, vertex.position.z, vertex.color.r, vertex.color.g, vertex.color.b, vertex.texture_coord.x, vertex.texture_coord.y);
		}
		fclose(outpFile);
	}

	void initDraw()
	{
		for (int i = 0; i < vertices.size() - 1; i++)
		{
			auto vertex = vertices[i];
			auto nextVertex = vertices[i + 1];
			auto betweenVertex = Vertex();
			finalVertecies.push_back(vertex);
			indices.push_back(finalVertecies.size() - 1);
			betweenVertex.position.x = nextVertex.position.x;
			betweenVertex.position.y = nextVertex.position.y;
			betweenVertex.position.z = vertex.position.z;
			betweenVertex.color.r = vertex.color.r;
			betweenVertex.color.g = vertex.color.g;
			betweenVertex.color.b = vertex.color.b;
			finalVertecies.push_back(betweenVertex);
			indices.push_back(finalVertecies.size() - 1);
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &ebo);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	void draw()
	{
		glBufferData(GL_ARRAY_BUFFER, finalVertecies.size() * sizeof(Vertex), finalVertecies.data(), GL_STATIC_DRAW);
		glBindVertexArray(vao);
		glDrawElements(GL_LINES, finalVertecies.size(), GL_UNSIGNED_INT, 0);
	}
};

//A vertex function class
class vertexFunc
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	std::vector<Vertex> getVertices(float min, float max, float step)
	{
		std::vector<Vertex> vertices;
		hsl hsl{ 0,1,0.5 };
		auto diff = (max + min) / 2;
		while (min < max)
		{
			auto v = Vertex();
			v.position.x = cos(min);
			v.position.y = sin(min);
			v.position.z = min / 10;
			hsl.shift(min);
			auto rgb = hsl.get_rgb();
			v.color.r = rgb[0];
			v.color.g = rgb[1];
			v.color.b = rgb[2];
			min += step;
			vertices.push_back(v);
		}
		auto v = Vertex();
		v.position.x = cos(min);
		v.position.y = sin(min);
		v.position.z = min / 10;
		hsl.shift(min);
		auto rgb = hsl.get_rgb();
		v.color.r = rgb[0];
		v.color.g = rgb[1];
		v.color.b = rgb[2];
		vertices.push_back(v);
		return vertices;
	}
	unsigned int vao;
	unsigned int ebo;
	unsigned int vbo;
public:
	vertexFunc()
	{
		vertices = getVertices(0, 6 * 3, 0.1f);
		const file_handle file("VertexFunc_outp.txt", "w");
		file.printf("%d points\n", vertices.size());
		for (auto vertex : vertices)
		{
			file.printf("%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f\n", 
				vertex.position.x, vertex.position.y, vertex.position.z, 
				vertex.color.r, vertex.color.g, vertex.color.b, 
				vertex.texture_coord.x, vertex.texture_coord.y);
		}
	}

	void initDraw()
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			indices.push_back(i);
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &ebo);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	void draw()
	{
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glBindVertexArray(vao);
		glDrawElements(GL_LINE_STRIP, vertices.size(), GL_UNSIGNED_INT, 0);
	}
};

//a two variable function class
class twoVarFunc
{
	int size;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Vertex> getVertices(int size, float step, float offset_x, float offset_y)
	{
		std::vector<Vertex> vertices;
		hsl hsl{ 0,1,0.5 };
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				auto v = Vertex();
				v.position.x = -offset_x + i * step;
				v.position.y = -offset_y + j * step;
				v.position.z = cos(v.position.x + v.position.y);
				hsl.shift(v.position.z);
				auto rgb = hsl.get_rgb();
				v.color.r = rgb[0];
				v.color.g = rgb[1];
				v.color.b = rgb[2];
				vertices.push_back(v);
			}
		}
		return vertices;
	}
	unsigned int vao;
	unsigned int ebo;
	unsigned int vbo;
public:
	twoVarFunc(int size, float step)
	{
		this->size = size;
		const float offset = step * size / 2.f;
		vertices = getVertices(size, step, offset, offset);
		const file_handle file("TwoVariable_outp.txt", "w");
		file.printf("%d points\n", vertices.size());
		for (auto vertex : vertices)
		{
			file.printf("%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f\n", 
				vertex.position.x, vertex.position.y, vertex.position.z, 
				vertex.color.r, vertex.color.g, vertex.color.b, 
				vertex.texture_coord.x, vertex.texture_coord.y);
		}
	}
	void initDraw()
	{
		for (int i = 0; i < size - 1; i++)
		{
			for (int j = 0; j < size - 1; j++)
			{
				indices.push_back(i * size + j);
				indices.push_back(i * size + j + 1);
				indices.push_back((i + 1) * size + j);
				indices.push_back((i + 1) * size + j);
				indices.push_back((i + 1) * size + j + 1);
				indices.push_back(i * size + j + 1);
			}
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &ebo);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	void draw()
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}
};

//a class to draw from file, can be called separately from the 
//other functions
class drawFromFile
{
	int size;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int vao;
	unsigned int ebo;
	unsigned int vbo;
	GLenum mode;
	bool optimizedTriangles;
public:
	drawFromFile(const char* filename, GLenum mode = GL_LINES, bool optimizedTriangles = false)
	{
		std::ifstream file(filename);
		std::string line;
		int size = 0;
		while (std::getline(file, line))
		{
			std::stringstream ss(line);
			if (size == 0)
			{
				ss >> size;
				continue;
			}
			auto v = Vertex();
			ss >> v.position.x >> v.position.y >> v.position.z 
				>> v.color.r >> v.color.g >> v.color.b 
				>> v.texture_coord.x >> v.texture_coord.y;
			vertices.push_back(v);
			if (!optimizedTriangles)
				indices.push_back(indices.size());
		}
		this->mode = mode;
		this->optimizedTriangles = optimizedTriangles;
	}
	void initDraw()
	{
		if (optimizedTriangles)
		{
			int size = sqrt(vertices.size());

			for (int i = 0; i < size - 1; i++)
			{
				for (int j = 0; j < size - 1; j++)
				{
					indices.push_back(i * size + j);
					indices.push_back(i * size + j + 1);
					indices.push_back((i + 1) * size + j);
					indices.push_back((i + 1) * size + j);
					indices.push_back((i + 1) * size + j + 1);
					indices.push_back(i * size + j + 1);
				}
			}
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &ebo);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	void draw()
	{
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glBindVertexArray(vao);
		glDrawElements(this->mode, indices.size(), GL_UNSIGNED_INT, 0);
	}
};

//=========================================================//
//               Oblig 1 end (Math and Prog)               //
//=========================================================//

int main() 
{
	// Initialize GLFW library
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Big Nice Windows", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, process_mouse_input);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//calling the functions to draw
	//Uncomment to run the code, which has to be done at least once
	//to create the txt file
	//--------------------------------
	auto f = func();
	//auto f = vertexFunc();
	//auto f = twoVarFunc(100, .1f);

	//--------------------------------
	//Printing using DrawFromFile
	//Just uncomment any of the functions below to run them
	//auto f = drawFromFile("function_output.txt", GL_LINE_STRIP, false);
	//auto f = drawFromFile("VertexFunc_outp.txt", GL_LINE_STRIP, false);
	//auto f = drawFromFile("TwoVariable_outp.txt", GL_TRIANGLES, true);
    //--------------------------------

	f.initDraw();

	Shader shader("shader.vs", "shader.fs");
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
		800.0f / 600.0f, 0.1f, 100.0f);
	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		const double deltaTime = glfwGetTime() - lastTime;
		// input
		process_key_input(window);
		process_direction(deltaTime);
		//               Render
		//            R     G     B
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	
		shader.use();
		shader.set_mat4("view", value_ptr(camera.get_view_matrix()));
		shader.set_mat4("model", value_ptr(model));
		shader.set_mat4("projection", value_ptr(projection));

		f.draw();
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
		direction = Direction::NONE;
		lastTime = glfwGetTime();
	}

	glfwTerminate();
	return 0;
}

