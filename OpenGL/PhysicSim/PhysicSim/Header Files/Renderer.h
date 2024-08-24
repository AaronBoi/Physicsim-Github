#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "VertexArray.h"
#include <iostream>
#include "Vector3.h"


class Renderer
	
{

private:
	


public:

	
	static void Clear();
	static void EnableDebug();

	//hier entweder vector für variable size oder von anfang an große Array
	static std::vector<float> triangles;
	static std::vector<float> lines;

	static void pushTriangles(std::vector<float> vertices);
	static void pushLines(std::vector<float> vertices);

	static void DrawTriangles(VertexArray& VAO, const VertexBuffer& VBO);
	static void DrawLines(VertexArray& VAO, const VertexBuffer& VBO);

	struct window {
		static int height;
		static int width;
	};

	static void window_size_callback(GLFWwindow* window, int width, int height);

	static float transX(float x);
	static float transY(float y);

	static float simHeight;
	static float simWidth;

};