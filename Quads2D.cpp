#include "Quads2D.h"

Quads2D::Quads2D(float originalX, float originalY, float originalZ, float width, float height)
{
	Init(originalX, originalY, originalZ, width, height);
}

void Quads2D::Init(float originalX, float originalY, float originalZ, float width, float height)
{
	vertexUVNormal vertexData[4];
	constexpr uint indices[6] = {
		0, 1, 2, 2, 3, 0,
	};

	vertexData[0] = {originalX, originalY, originalZ, 0.0, 0.0, 0.0, 0.0, -1.0};
	vertexData[1] = {originalX, originalY + height, originalZ, 0.0, 1.0, 0.0, 0.0, -1.0};
	vertexData[2] = {originalX + width, originalY + height, originalZ, 1.0, 1.0, 0.0, 0.0, -1.0};
	vertexData[3] = {originalX + width, originalY, originalZ, 1.0, 0.0, 0.0, 0.0, -1.0};

	m_vb.Init(vertexData, sizeof(vertexData));
	m_ib.Init(indices, sizeof(indices) / sizeof(uint));

	VertexBufferLayout layout;
	layout.Push<float>(3); //Position
	layout.Push<float>(2); //TexCoords
	layout.Push<float>(3); //Normal
	m_va.AddBuffer(m_vb, m_ib, layout);

	VertexArray::Unbind();
}


void Quads2D::Draw(Shader &shader, Renderer &renderer, const JPH::Mat44& projectionMatrix)
{
	shader.Bind();

	shader.SetUniform("u_MVP", projectionMatrix);
	shader.SetUniform("u_modelMatrix", JPH::Mat44::sIdentity());
	renderer.Draw(m_va, m_ib);
}

void Quads2D::Draw(Shader &shader, Renderer &renderer, const JPH::Mat44 &projectionMatrix, const JPH::Mat44 &modelMatrix)
{
	shader.Bind();

	shader.SetUniform("u_MVP", projectionMatrix * modelMatrix);
	shader.SetUniform("u_modelMatrix", modelMatrix);
	renderer.Draw(m_va, m_ib);
}
