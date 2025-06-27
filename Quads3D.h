#ifndef QUADS3D_H
#define QUADS3D_H

#include "Util.h"
#include "VertexBuffer.h"

#include <vector>

#include "IndexBuffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"

class Quads3D {
public:
	const float m_width;
	const float m_height;
	const float m_depth;
	const int m_numQuads;

	Quads3D(float originalX, float originalY, float originalZ, float width, float height, float depth, int numQuads, std::vector<JPH::Mat44>& modelMatrices);
	void Draw(Shader &shader, Renderer &renderer, const JPH::Mat44& projectionMatrix, const JPH::Mat44& viewMatrix);

private:
	std::vector<JPH::Mat44> &m_modelMatrices;

	VertexArray m_va;
	VertexBufferLayout m_layout;

	VertexBuffer m_vb;
	IndexBuffer m_ib;
};



#endif //QUADS3D_H
