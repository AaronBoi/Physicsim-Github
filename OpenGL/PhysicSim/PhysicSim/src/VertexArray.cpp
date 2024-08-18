#include "VertexArray.h"

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_RendererID);
	Bind();
}

VertexArray::~VertexArray() 
{
	glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::LinkBuffer(const VertexBuffer& VertexBuffer, const unsigned int layout)
{
	m_Stride = 7 * sizeof(float);
	m_BufferCount = VertexBuffer.GetSize() / m_Stride;
	VertexBuffer.Bind();
	glVertexAttribPointer(layout, 3, GL_FLOAT, GL_FALSE, m_Stride, (void*)0);
	glEnableVertexAttribArray(layout);

	glVertexAttribPointer(layout + 1, 4, GL_FLOAT, GL_FALSE, m_Stride, (const void*)12);
	glEnableVertexAttribArray(layout + 1);
	VertexBuffer.Unbind();
}

void VertexArray::Bind() const
{
	glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
	glBindVertexArray(0);
}

