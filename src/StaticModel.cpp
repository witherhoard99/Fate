#include "StaticModel.h"
#include "PhysicsObjectFactory.h"

#include "Renderer.h"
#include "Texture.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "FrustumCulling.h"

StaticModel::StaticModel(Renderer &renderer, const std::string &sceneFilepath, Physics &physics, FrustumCuller& frustumCuller, bool processModel)
    :   Model(renderer),
        m_physics(physics),
        m_frustumCuller(frustumCuller)
{
    //TODO: BIGGEST HACK OF ALL TIME. If the number of textures exceeds this
    //then all textures for this model break as the memory of the vector gets reallocated, and so the pointers become invalid
    //We could heap allocated the textures using unique_pointer but that adds double indirection so this should be good enough
    m_loadedTextures.reserve(500);
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        sceneFilepath,
        aiProcess_Triangulate | aiProcess_FlipUVs |
        aiProcess_GenNormals | aiProcess_JoinIdenticalVertices |
        aiProcess_FindDegenerates | aiProcess_FindInvalidData |
        aiProcess_OptimizeMeshes
    );

    ASSERT_LOG(
        !(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode),
        "Error with loading model:" << importer.GetErrorString() << " with path " << sceneFilepath
    );

    if (processModel)
        ProcessNode(scene->mRootNode, scene, sceneFilepath.substr(0, sceneFilepath.find_last_of('/')), JPH::Mat44::sIdentity());
}

void StaticModel::Draw(Shader &shader, const JPH::Mat44 &projectionMatrix, const JPH::Mat44 &viewMatrix)
{
    JPH::Mat44 LookViewMatrix = JPH::Mat44::sLookAt({0, 13, 0}, {20, 1, 0}, {0, 1, 0});
    LookViewMatrix = viewMatrix;
    shader.SetUniform("u_viewMatrix", LookViewMatrix); //TODO: Get rid of the code here that I used to test for frustum culling

    std::vector<JPH::BodyID> allBodyIDs(m_objects.size());
    for (int i = 0; i < m_objects.size(); ++i)
    {
        allBodyIDs[i] = m_objects[i].bodyID;
    }

    // glm::mat4 viewMatrixGLM = glm::make_mat4x4(reinterpret_cast<const float*>(&viewMatrix));
    // glm::mat4 projectionMatrixGLM = glm::make_mat4x4(reinterpret_cast<const float*>(&projectionMatrix));

    // std::vector<JPH::BodyID> toDraw = FrustumCulling::GetVisibleBodies(m_physics.GetBodyManager(), allBodyIDs, viewMatrixGLM, projectionMatrixGLM);
    std::vector<JPH::BodyID> toDraw = StaticModel::m_frustumCuller.GetVisibleBodies(m_physics.GetBodyManager(), allBodyIDs, viewMatrix, projectionMatrix);
    //TODO: Fix all of this mess, it makes a performance difference

    auto contains = [&](std::vector<JPH::BodyID> &vector, JPH::BodyID target) -> bool {
        for (int i = 0; i < vector.size(); ++i)
        {
            if (vector[i] == target)
                return true;
        }

        return false;
    };

    //Find which objects we want to draw
    for (int i = 0; i < allBodyIDs.size(); ++i)
    {
        if (contains(toDraw, allBodyIDs[i]))
            m_objects[i].draw = true;
    }

    for (int i = 0; i < m_meshes.size(); ++i)
    {
        if (m_objects[i].draw)
            m_meshes[i].Draw(m_renderer, shader, projectionMatrix * LookViewMatrix);
    }

    for (int i = 0; i < m_objects.size(); ++i)
    {
        m_objects[i].draw = false;
    }
}

void StaticModel::Draw(Shader &shader, const JPH::Mat44 &projectionMatrix, const JPH::Mat44 &viewMatrix,
    const JPH::Mat44 &modelMatrix)
{
    Model::Draw(shader, projectionMatrix, viewMatrix, modelMatrix);
    // shader.SetUniform("u_viewMatrix", JPH::Mat44::sLookAt({0, 13, 0}, {20, 1, 0}, {0, 1, 0}));

    for (Mesh& mesh : m_meshes)
        mesh.Draw(m_renderer, shader, projectionMatrix * viewMatrix, modelMatrix);
        // mesh.Draw(m_renderer, shader, projectionMatrix * JPH::Mat44::sLookAt({0, 13, 0}, {20, 1, 0}, {0, 1, 0}), modelMatrix);
}

void StaticModel::ProcessNode(aiNode *node, const aiScene *scene, const std::string &directory, const JPH::Mat44& parentTransformation)
{
    m_meshes.reserve(m_meshes.size() + node->mNumMeshes);

    aiMatrix4x4 aiLocal = node->mTransformation;
    JPH::Mat44 local = ConvertAssimpMatrix(aiLocal);
    JPH::Mat44 globalTransform = parentTransformation * local;

    for (int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

        std::vector<vertexUVNormal> vertices;
        std::vector<uint> indices;
        std::vector<const Texture*> textures;

        ProcessMesh(mesh, scene, directory, vertices, indices, textures, globalTransform);

        Error error;
        m_meshes.emplace_back(vertices, indices, textures, globalTransform, error);

        HANDLE_ERROR(error,
            ASSERT_LOG(false, "Unable to process a mesh. Canceling Mesh processing.");
        );
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, directory, globalTransform);
    }
}

void StaticModel::ProcessMesh(
    aiMesh *mesh, const aiScene *scene, const std::string &directory,
    std::vector<vertexUVNormal> &outVertices, std::vector<uint> &outIndices,
    std::vector<const Texture *> &outTextures, JPH::Mat44 &transform)
{
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    outVertices.reserve(mesh->mNumVertices);
    //Roughly preallocate some memory so that lots of tiny allocations are not made
    outIndices.reserve(mesh->mFaces[0].mNumIndices * 2);
    outTextures.reserve(material->GetTextureCount(aiTextureType_DIFFUSE) + material->GetTextureCount(aiTextureType_SPECULAR));

    std::vector<JPH::Vec3> positions;

    for (uint i = 0; i < mesh->mNumVertices; i++)
    {
        vertexUVNormal vertex{};
        vertex.posX = mesh->mVertices[i].x;
        vertex.posY = mesh->mVertices[i].y;
        vertex.posZ = mesh->mVertices[i].z;

        vertex.normalX = mesh->mNormals[i].x;
        vertex.normalY = mesh->mNormals[i].y;
        vertex.normalZ = mesh->mNormals[i].z;

        if (mesh->mTextureCoords[0])
        {
            vertex.texcoordX = mesh->mTextureCoords[0][i].x;
            vertex.texcoordY = mesh->mTextureCoords[0][i].y;
        }

        positions.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        outVertices.push_back(vertex);
    }

    for(uint i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(uint j = 0; j < face.mNumIndices; j++)
            outIndices.emplace_back(face.mIndices[j]);
    }

    outIndices.shrink_to_fit();

    //TODO: Check if the 1000 mass is okay. Should be since these are static objects and mass shouldn't even matter.
    m_objects.emplace_back(PhysicsObjectFactory::ConstructStaticMesh(1000, m_physics, positions, outIndices, transform));

    LoadMaterialTextures(material, aiTextureType_DIFFUSE, Texture::TextureType::diffuse, directory, outTextures);
    LoadMaterialTextures(material, aiTextureType_SPECULAR, Texture::TextureType::specular, directory, outTextures);
}
