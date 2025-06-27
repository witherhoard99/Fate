#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "Util.h"

class VertexBuffer
{
private:
	uint m_rendererID;
public:
	VertexBuffer();
	VertexBuffer(const void* data, uint size);

	VertexBuffer(const VertexBuffer&) = delete; //No copying!!! Leads to us after free issues
	VertexBuffer& operator=(const VertexBuffer&) = delete;

	VertexBuffer(VertexBuffer&& other) noexcept;

	~VertexBuffer();

	void Init(const void* data, uint size); //For delayed initialization

	void Bind() const;
	static void Unbind();

	void ChangeData(const void *newData, uint size, uint offset = 0);
};



#endif //VERTEXBUFFER_H
