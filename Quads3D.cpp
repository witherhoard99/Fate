#include "Quads3D.h"

Quads3D::Quads3D(float originalX, float originalY, float originalZ, float width, float height, float depth, int numQuads, std::vector<JPH::Mat44> &modelMatrices)
	:	m_width(width),
		m_height(height),
		m_depth(depth),
		m_numQuads(numQuads),
		m_modelMatrices(modelMatrices)
{
	if (numQuads != modelMatrices.size())
	{
		std::string errorMsg = "The number of quads and the size of quadPositionTranslations are not the same. numQuads: " + numQuads;
		errorMsg += " quadPositionTranslations.size(): " + modelMatrices.size();

		throw std::runtime_error(errorMsg.c_str());
	}

	vertexUVNormal vertexData[24];
	constexpr uint indices[36] = {
		0, 1, 2, 2, 3, 0,       // Front face
		4, 5, 6, 6, 7, 4,       // Left face
		8, 9, 10, 10, 11, 8,    // Right face
		12, 13, 14, 14, 15, 12, // Back face
		16, 17, 18, 18, 19, 16, // Top face
		20, 21, 22, 22, 23, 20  // Bottom face
	};

	// Front face (normal points in the negative Z direction)
	vertexData[0] = {originalX, originalY, originalZ, 0.0, 0.0, 0.0, 0.0, -1.0};
	vertexData[1] = {originalX, originalY + height, originalZ, 0.0, 1.0, 0.0, 0.0, -1.0};
	vertexData[2] = {originalX + width, originalY + height, originalZ, 1.0, 1.0, 0.0, 0.0, -1.0};
	vertexData[3] = {originalX + width, originalY, originalZ, 1.0, 0.0, 0.0, 0.0, -1.0};

	// Left face (normal points in the negative X direction)
	vertexData[4] = {originalX, originalY, originalZ, 1.0, 0.0, -1.0, 0.0, 0.0};
	vertexData[5] = {originalX, originalY, originalZ + depth, 0.0, 0.0, -1.0, 0.0, 0.0};
	vertexData[6] = {originalX, originalY + height, originalZ + depth, 0.0, 1.0, -1.0, 0.0, 0.0};
	vertexData[7] = {originalX, originalY + height, originalZ, 1.0, 1.0, -1.0, 0.0, 0.0};

	// Right face (normal points in the positive X direction)
	vertexData[8] = {originalX + width, originalY, originalZ, 1.0, 0.0, 1.0, 0.0, 0.0};
	vertexData[9] = {originalX + width, originalY + height, originalZ, 1.0, 1.0, 1.0, 0.0, 0.0};
	vertexData[10] = {originalX + width, originalY + height, originalZ + depth, 0.0, 1.0, 1.0, 0.0, 0.0};
	vertexData[11] = {originalX + width, originalY, originalZ + depth, 0.0, 0.0, 1.0, 0.0, 0.0};

	// Back face (normal points in the positive Z direction)
	vertexData[12] = {originalX, originalY, originalZ + depth, 0.0, 0.0, 0.0, 0.0, 1.0};
	vertexData[13] = {originalX + width, originalY, originalZ + depth, 1.0, 0.0, 0.0, 0.0, 1.0};
	vertexData[14] = {originalX + width, originalY + height, originalZ + depth, 1.0, 1.0, 0.0, 0.0, 1.0};
	vertexData[15] = {originalX, originalY + height, originalZ + depth, 0.0, 1.0, 0.0, 0.0, 1.0};

	// Top face (normal points in the positive Y direction)
	vertexData[16] = {originalX, originalY + height, originalZ, 0.0, 0.0, 0.0, 1.0, 0.0};
	vertexData[17] = {originalX, originalY + height, originalZ + depth, 0.0, 1.0, 0.0, 1.0, 0.0};
	vertexData[18] = {originalX + width, originalY + height, originalZ + depth, 1.0, 1.0, 0.0, 1.0, 0.0};
	vertexData[19] = {originalX + width, originalY + height, originalZ, 1.0, 0.0, 0.0, 1.0, 0.0};

	// Bottom face (normal points in the negative Y direction)
	vertexData[20] = {originalX, originalY, originalZ, 0.0, 0.0, 0.0, -1.0, 0.0};
	vertexData[21] = {originalX + width, originalY, originalZ, 1.0, 0.0, 0.0, -1.0, 0.0};
	vertexData[22] = {originalX + width, originalY, originalZ + depth, 1.0, 1.0, 0.0, -1.0, 0.0};
	vertexData[23] = {originalX, originalY, originalZ + depth, 0.0, 1.0, 0.0, -1.0, 0.0};


	m_vb.Init(vertexData, sizeof(vertexData));
	m_ib.Init(indices, sizeof(indices) / sizeof(uint));

	m_layout.Push<float>(3); //Position
	m_layout.Push<float>(2); //TexCoords
	m_layout.Push<float>(3); //Normal
	m_va.AddBuffer(m_vb, m_ib, m_layout);

	VertexArray::Unbind();
}



void Quads3D::Draw(Shader &shader, Renderer &renderer, const JPH::Mat44& projectionMatrix, const JPH::Mat44& viewMatrix)
{
	JPH::Mat44 projViewMatrix = projectionMatrix * viewMatrix;
	shader.Bind();

	for (const auto& i : m_modelMatrices)
	{
		shader.SetUniform("u_MVP", projViewMatrix * i);
		shader.SetUniform("u_modelMatrix", i);
		renderer.Draw(m_va, m_ib);
	}
}
