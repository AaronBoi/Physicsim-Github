#pragma once

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "shader.h"
#include <iostream>
#include <map>



class Renderer
	
{

public:
	Renderer();
	void Clear();
	void EnableDebug();
	void Draw(const VertexArray& VAO, const IndexBuffer& IBO);
	void DrawTriangles(VertexArray& VAO, const VertexBuffer& VBO);
	void DrawLines(VertexArray& VAO, const VertexBuffer& VBO);

	float trianglePositions;
	float linePositions;

};