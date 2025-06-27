#include "Mesh.h"

Mesh::Mesh(const std::vector<vertexUVNormal> &vertices, const std::vector<uint> &indices, const std::vector<const Texture*>& textures, const JPH::Mat44& modelMatrix, Error &error)
{
    Init(vertices, indices, textures, modelMatrix, error);
}

void Mesh::Init(const std::vector<vertexUVNormal> &vertices, const std::vector<uint> &indices, const std::vector<const Texture*> &textures, const JPH::Mat44& modelMatrix, Error& error)
{
    //TODO: I don't think we need to do this actually
    //TODO: (IMMEDIATE) Assign the texture, and when doing so ensure that there are not more than 16 textures needed for a mesh
    //TODO: (IMMEDIATE) Check if mesh has too many diffuse, or specular textures (more than whatever number we declared in the shader)
    //If so throw an error

    m_textures = textures;
    m_modelMatrix = modelMatrix;

    VertexBufferLayout layout;
    layout.Push<float>(3); //pos
    layout.Push<float>(2); //uv
    layout.Push<float>(3); //normals

    m_vao.Bind();

    m_vbo.Init(vertices.data(), vertices.size() * sizeof(vertices[0]));
    m_ibo.Init(indices.data(), indices.size());
    m_vao.AddBuffer(m_vbo, m_ibo, layout);

    VertexArray::Unbind();
}

void Mesh::Draw(Renderer& renderer, Shader& shader, const JPH::Mat44 &projViewMatrix)
{
    //TODO: Figure out how much of this we can do at init time instead of in the draw call
    uint diffuseNum = 0;
    uint specularNum = 0;

    for (uint i = 0; i < m_textures.size(); i++)
    {
        std::string texUniformNum;
        Texture::TextureType texType = m_textures[i]->GetType();

        if (texType == Texture::TextureType::diffuse)
        {
            texUniformNum = std::to_string(diffuseNum++);
        }
        else if (texType == Texture::TextureType::specular)
        {
            texUniformNum = std::to_string(specularNum++);
            shader.SetUniform("u_haveSpecularTexture", true);
        }

        ASSERT_LOG(texType != Texture::TextureType::unknown, "Texture has not been initialized!");

        std::string name; //All appending and reserving is done specifically to reduce heap allocations
        name.reserve(49);
        name += "u_texture";
        name += Texture::ConvertTextureTypeToString(texType);
        name += texUniformNum;

        m_textures[i]->Bind(i);
        shader.SetUniform(name, i);
    }

    shader.SetUniform("u_MVP", projViewMatrix * m_modelMatrix);
    shader.SetUniform("u_modelMatrix", m_modelMatrix);

    renderer.Draw(m_vao, m_ibo);

    shader.SetUniform("u_haveSpecularTexture", false);
}

void Mesh::Draw(Renderer &renderer, Shader &shader, const JPH::Mat44 &projViewMatrix, const JPH::Mat44 &modelMatrix)
{
    //TODO: Figure out how much of this we can do at init time instead of in the draw call
    uint diffuseNum = 0;
    uint specularNum = 0;

    for (uint i = 0; i < m_textures.size(); i++)
    {
        std::string texUniformNum;
        Texture::TextureType texType = m_textures[i]->GetType();

        if (texType == Texture::TextureType::diffuse)
        {
            texUniformNum = std::to_string(diffuseNum++);
        }
        else if (texType == Texture::TextureType::specular)
        {
            texUniformNum = std::to_string(specularNum++);
            shader.SetUniform("u_haveSpecularTexture", true);
        }

        ASSERT_LOG(texType != Texture::TextureType::unknown, "Texture has not been initialized!");

        std::string name; //All appending and reserving is done specifically to reduce heap allocations
        name.reserve(49);
        name += "u_texture";
        name += Texture::ConvertTextureTypeToString(texType);
        name += texUniformNum;

        m_textures[i]->Bind(i);
        shader.SetUniform(name, i);
    }

    JPH::Mat44 modelMatrixCombined = modelMatrix * m_modelMatrix;
    shader.SetUniform("u_MVP", projViewMatrix * modelMatrixCombined);
    shader.SetUniform("u_modelMatrix", modelMatrixCombined);

    renderer.Draw(m_vao, m_ibo);

    shader.SetUniform("u_haveSpecularTexture", false);
}
