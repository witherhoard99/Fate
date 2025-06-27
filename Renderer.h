#ifndef RENDERER_H
#define RENDERER_H
#include "IndexBuffer.h"
#include "Shader.h"
#include "VertexArray.h"


class Renderer {
private:
public:
	inline void Clear() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Draw(const VertexArray& va, const IndexBuffer& ib) const;
};



#endif //RENDERER_H
