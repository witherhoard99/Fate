#ifndef MODEL_H
#define MODEL_H

#include "Renderer.h"
#include "Texture.h"
#include "../Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Physics.h"

class Model
{
protected:
    Renderer& m_renderer;
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_loadedTextures;

    virtual void ProcessNode(aiNode* node, const aiScene* scene, const std::string &directory, const JPH::Mat44& parentTransformation);

    virtual void ProcessMesh(
        aiMesh* mesh, const aiScene* scene, const std::string &directory,
        std::vector<vertexUVNormal> &outVertices,
        std::vector<uint> &outIndices,
        std::vector<const Texture*> &outTextures
    );

    void LoadMaterialTextures(
        aiMaterial *mat, aiTextureType type,
        Texture::TextureType texType, const std::string& directory,
        std::vector<const Texture*> &outTexVector
    );

    static JPH::Mat44 ConvertAssimpMatrix(const aiMatrix4x4& matrix);

    //This constructor is so that the model itself is not loaded, instead that it left for the child class to do
    explicit Model(Renderer& renderer) : m_renderer(renderer) {}

public:
    virtual ~Model() = default;

    Model(Renderer& renderer, const std::string& sceneFilepath);
    virtual void Draw(Shader& shader, const JPH::Mat44& projectionMatrix, const JPH::Mat44& viewMatrix);
    virtual void Draw(Shader& shader, const JPH::Mat44& projectionMatrix, const JPH::Mat44& viewMatrix, const JPH::Mat44& modelMatrix);
};



#endif //MODEL_H
