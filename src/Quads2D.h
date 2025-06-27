#ifndef QUADS2D_H
#define QUADS2D_H

#include "Util.h"
#include "VertexBuffer.h"

#include <vector>

#include "../IndexBuffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "../VertexArray.h"

class Quads2D {
public:
    Quads2D() = default;
    Quads2D(float originalX, float originalY, float originalZ, float width, float height);
    void Init(float originalX, float originalY, float originalZ, float width, float height);

    void Draw(Shader &shader, Renderer &renderer, const JPH::Mat44& projectionMatrix);
    void Draw(Shader &shader, Renderer &renderer, const JPH::Mat44& projectionMatrix, const JPH::Mat44& modelMatrix);

private:
    VertexArray m_va;

    VertexBuffer m_vb;
    IndexBuffer m_ib;
};



#endif //QUADS2D_H
