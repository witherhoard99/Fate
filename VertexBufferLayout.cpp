#include "VertexBufferLayout.h"

#include <iostream>

VertexBufferLayout::VertexBufferLayout()
{
	m_Stride = 0;
}


template<typename T>
void VertexBufferLayout::Push(uint count)
{
	ASSERT_LOG(false, "You must call specialization!!");
	std::terminate();
}

template<>
void VertexBufferLayout::Push<float>(uint count)
{
	m_Elements.emplace_back(GL_FLOAT, count, GL_FALSE);
	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
}

template<>
void VertexBufferLayout::Push<uint>(uint count)
{
	m_Elements.emplace_back(GL_UNSIGNED_INT, count, GL_FALSE);
	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
}

template<>
void VertexBufferLayout::Push<unsigned char>(uint count)
{
	m_Elements.emplace_back(GL_UNSIGNED_BYTE, count, GL_TRUE);
	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
}


uint VertexBufferLayout::GetStride() const
{
	return m_Stride;
}

const std::vector<VertexBufferElement>& VertexBufferLayout::GetElements() const
{
	return m_Elements;
}