#include "Renderer.h"

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib) const
{
	va.Bind();
	glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}
