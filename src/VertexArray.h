#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

class VertexArray {
private:
	uint m_RendererID;

public:
	VertexArray();
	~VertexArray();

	VertexArray(const VertexArray& other) = delete;
	VertexArray& operator=(const VertexArray& other) = delete;

	VertexArray(VertexArray&& other) noexcept;

	void Bind() const;
	static void Unbind();
	void AddBuffer(const VertexBuffer& vb, IndexBuffer &ib, const VertexBufferLayout& layout);
};



#endif //VERTEXARRAY_H
