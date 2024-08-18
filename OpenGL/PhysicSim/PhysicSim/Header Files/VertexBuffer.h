#pragma once
#include <gl/glew.h>

class VertexBuffer
{
private:
	unsigned int m_RendererID;
	GLsizei m_Size;
public:
	VertexBuffer(const void* data, GLsizei size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
	inline GLsizei GetSize() const { return m_Size; }
};