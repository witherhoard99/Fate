#include "Model.h"

Model::Model(Renderer& renderer, const std::string& sceneFilepath)
    :   m_renderer(renderer)
{
    //TODO: BIGGEST HACK OF ALL TIME. If the number of textures exceeds this
    //then all textures for this model break as the memory of the vector gets reallocated, and so the pointers become invalid
    //We could heap allocated the textures using unique_pointer but that adds double indirection so this should be good enough
    m_loadedTextures.reserve(500);
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(sceneFilepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ForceGenNormals);

    ASSERT_LOG(
        !(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode),
        "Error with loading model:" << importer.GetErrorString() << " with path " << sceneFilepath
    );

    ProcessNode(scene->mRootNode, scene, sceneFilepath.substr(0, sceneFilepath.find_last_of('/')), JPH::Mat44::sIdentity());
}

void Model::ProcessNode(aiNode *node, const aiScene *scene, const std::string &directory, const JPH::Mat44& parentTransformation)
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

        ProcessMesh(mesh, scene, directory, vertices, indices, textures);

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

void Model::ProcessMesh(aiMesh *mesh, const aiScene *scene, const std::string &directory,
                        std::vector<vertexUVNormal> &outVertices,
                        std::vector<uint> &outIndices,
                        std::vector<const Texture*> &outTextures
)
{
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    outVertices.reserve(mesh->mNumVertices);
    //Roughly preallocate some memory so that lots of tiny allocations are not made
    outIndices.reserve(mesh->mFaces[0].mNumIndices * 2);
    outTextures.reserve(material->GetTextureCount(aiTextureType_DIFFUSE) + material->GetTextureCount(aiTextureType_SPECULAR));

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

        outVertices.push_back(vertex);
    }

    for(uint i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(uint j = 0; j < face.mNumIndices; j++)
            outIndices.emplace_back(face.mIndices[j]);
    }

    LoadMaterialTextures(material, aiTextureType_DIFFUSE, Texture::TextureType::diffuse, directory, outTextures);
    LoadMaterialTextures(material, aiTextureType_SPECULAR, Texture::TextureType::specular, directory, outTextures);

    outIndices.shrink_to_fit();
}

void Model::LoadMaterialTextures(aiMaterial *mat, aiTextureType type, Texture::TextureType texType, const std::string& directory, std::vector<const Texture*> &outTexVector)
{
    int numTexture = mat->GetTextureCount(type);
    for (int i = 0; i < numTexture; i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        //If we can find it in the global model textures vector, then just return that
        //Otherwise, add it to the global model textures vector
        bool addToGlobalTextures = false;
        for (int j = 0; j < m_loadedTextures.size(); j++)
        {
            if (m_loadedTextures[j].GetFilePath() == (directory + "/" + str.C_Str()))
            {
                outTexVector.emplace_back(&m_loadedTextures[j]);
                addToGlobalTextures = true;
                break;
            }
        }

        if (!addToGlobalTextures)
        {
            m_loadedTextures.emplace_back(directory + "/" + str.C_Str(), texType);
            outTexVector.emplace_back(&m_loadedTextures.back());
        }
    }
}

JPH::Mat44 Model::ConvertAssimpMatrix(const aiMatrix4x4 &mat)
{
    //Transpose the matrix because aiMatrix is row major, and we need column major matrix for jolt physics matrix
    JPH::Vec4 col0(mat.a1, mat.b1, mat.c1, mat.d1);
    JPH::Vec4 col1(mat.a2, mat.b2, mat.c2, mat.d2);
    JPH::Vec4 col2(mat.a3, mat.b3, mat.c3, mat.d3);
    JPH::Vec4 col3(mat.a4, mat.b4, mat.c4, mat.d4);

    return {col0, col1, col2, col3};
}


void Model::Draw(Shader& shader, const JPH::Mat44& projectionMatrix, const JPH::Mat44& viewMatrix)
{
    shader.SetUniform("u_viewMatrix", viewMatrix);

    for (Mesh& mesh : m_meshes)
        mesh.Draw(m_renderer, shader, projectionMatrix * viewMatrix);
}

void Model::Draw(Shader &shader, const JPH::Mat44 &projectionMatrix, const JPH::Mat44 &viewMatrix,
    const JPH::Mat44 &modelMatrix)
{
    shader.SetUniform("u_viewMatrix", viewMatrix);

    for (Mesh& mesh : m_meshes)
        mesh.Draw(m_renderer, shader, projectionMatrix * viewMatrix, modelMatrix);
}
