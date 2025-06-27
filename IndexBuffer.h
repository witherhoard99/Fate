#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "Util.h"


class IndexBuffer {
private:
	uint m_RendererID;
	uint m_Count;

public:

	IndexBuffer();
	IndexBuffer(const uint* indicies, uint count);
	void Init(const uint* indicies, uint count);

	IndexBuffer(const IndexBuffer& other) = delete;
	IndexBuffer& operator=(const IndexBuffer& other) = delete;

	IndexBuffer(IndexBuffer&& other) noexcept;

	~IndexBuffer();

	void Bind();
	void Unbind();

	uint GetCount() const { return m_Count; }
};



#endif //INDEXBUFFER_H
