#include "VertexArray.h"

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_RendererID);
	glBindVertexArray(m_RendererID);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_RendererID);
	m_RendererID = 0;
}

VertexArray::VertexArray(VertexArray &&other) noexcept
	:	m_RendererID(other.m_RendererID)
{
	other.m_RendererID = 0;
}

void VertexArray::Bind() const
{
	glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind()
{
	glBindVertexArray(0);
}

void VertexArray::AddBuffer(const VertexBuffer& vb, IndexBuffer &ib, const VertexBufferLayout& layout)
{
	Bind();
	vb.Bind();
	ib.Bind();

	const auto& elements = layout.GetElements();
	uint offset = 0;

	for (uint i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset);

		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}
}
