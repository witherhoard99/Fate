#ifndef MESH_H
#define MESH_H

#include "Util.h"

#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"

class Mesh
{
    std::vector<const Texture*> m_textures; //These are pointers into the larger array (stored in Model class) for each model

    VertexArray m_vao;
    VertexBuffer m_vbo;

    //TODO: Make Mesh choose uint32 or unit16 at runtime to save memory, and update ProcessMesh function to match
    //Also update IndexBuffer to be able to do this
    IndexBuffer m_ibo;

    JPH::Mat44 m_modelMatrix;

public:

    Mesh() = default;
    Mesh(
        const std::vector<vertexUVNormal>& vertices, const std::vector<uint>& indices,
        const std::vector<const Texture*>& textures, const JPH::Mat44& modelMatrix, Error& error
    );
    void Init(const std::vector<vertexUVNormal>& vertices, const std::vector<uint>& indices,
        const std::vector<const Texture*>& textures, const JPH::Mat44& modelMatrix, Error& error
    );

    /*
     * Draws the meshes. The shader should already be bound as this method is intended to be called in a loop.
     * The other uniforms like u_MVP and u_viewMatrix should also already be set.
     * The model matrix parameter is to allow updating of the model at runtime
     */
    void Draw(Renderer& renderer, Shader& shader, const JPH::Mat44 &projViewMatrix);
    void Draw(Renderer& renderer, Shader& shader, const JPH::Mat44 &projViewMatrix, const JPH::Mat44 &modelMatrix);
};



#endif //MESH_H
