#pragma once

#include <gl/glew.h>
#include "VertexBuffer.h"

class VertexArray
{
private:
	unsigned int m_RendererID;
	GLsizei m_Stride;
	GLsizei m_BufferCount;
public:
	VertexArray();
	~VertexArray();

	void LinkBuffer(const VertexBuffer& VertexBuffer, const unsigned int layout);
	void Bind() const;
	void Unbind() const;
	inline GLsizei GetBufferCount() const { return m_BufferCount; }
};