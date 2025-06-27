#ifndef VERTEXBUFFERLAYOUT_H
#define VERTEXBUFFERLAYOUT_H

#include "Util.h"

struct VertexBufferElement
{
	uint type;
	uint count;
	int normalized;

	static uint GetSizeOfType(uint type)
	{
		switch (type)
		{
			case GL_FLOAT:
				return sizeof(GLfloat);

			case GL_UNSIGNED_INT:
				return sizeof(GLuint);

			case GL_UNSIGNED_BYTE:
				return sizeof(GLubyte);

			default: break;
		}

		std::cerr << "Unknown type in GetSizeOfType(): " << type << std::endl;
		__debugbreak();
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_Elements;
	uint m_Stride;

public:
	VertexBufferLayout();
	uint GetStride() const;
	const std::vector<VertexBufferElement>& GetElements() const;

	template<typename T>
	void Push(uint count);
};


#endif //VERTEXBUFFERLAYOUT_H
