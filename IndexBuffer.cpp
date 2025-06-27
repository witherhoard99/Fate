#include "IndexBuffer.h"
#include <GL/glew.h>

IndexBuffer::IndexBuffer() {}

IndexBuffer::IndexBuffer(const uint* indicies, uint count)
{
 	m_Count = count;
	glGenBuffers(1, &m_RendererID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint), indicies, GL_STATIC_DRAW);
}

void IndexBuffer::Init(const uint* indicies, uint count)
{
	m_Count = count;
	glGenBuffers(1, &m_RendererID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint), indicies, GL_STATIC_DRAW);
}

IndexBuffer::IndexBuffer(IndexBuffer &&other) noexcept
	:	m_RendererID(other.m_RendererID),
		m_Count(other.m_Count)
{
	other.m_RendererID = 0;
	other.m_Count = 0;
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_RendererID);
	m_RendererID = 0;
}


void IndexBuffer::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void IndexBuffer::Unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}