#include "../DynamicModel.h"

#include "imgui/imgui.h"
#include "Physics/Collision/CastResult.h"
#include "Physics/Collision/RayCast.h"

DynamicModel::DynamicModel(Renderer& renderer, const std::string &sceneFilepath, Physics &physics) :
    StaticModel(renderer, sceneFilepath, physics, false)
{
    //All of the code is the exact same as StaticModel but we must override all methods so that we can all our ProcessMesh function, which is actually new code
    //this is because we cant call subclass methods from the superclass

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

    DynamicModel::ProcessNode(scene->mRootNode, scene, sceneFilepath.substr(0, sceneFilepath.find_last_of('/')), JPH::Mat44::sIdentity());
}

DynamicModel::DynamicModel(Renderer &renderer, const std::string &sceneFilepath, Physics &physics, const JPH::Mat44 &transform):
    StaticModel(renderer, sceneFilepath, physics, false)
{
    //All of the code is the exact same as StaticModel but we must override all methods so that we can all our ProcessMesh function, which is actually new code
    //this is because we cant call subclass methods from the superclass

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

    DynamicModel::ProcessNode(scene->mRootNode, scene, sceneFilepath.substr(0, sceneFilepath.find_last_of('/')), transform);
}

void DynamicModel::ProcessNode(aiNode *node, const aiScene *scene, const std::string &directory, const JPH::Mat44& parentTransformation)
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

        DynamicModel::ProcessMesh(mesh, scene, directory, vertices, indices, textures, globalTransform);

        Error error;
        m_meshes.emplace_back(vertices, indices, textures, globalTransform, error);

        HANDLE_ERROR(error,
            ASSERT_LOG(false, "Unable to process a mesh. Canceling Mesh processing.");
        );
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        DynamicModel::ProcessNode(node->mChildren[i], scene, directory, globalTransform);
    }
}

void DynamicModel::ProcessMesh(aiMesh *mesh, const aiScene *scene, const std::string &directory,
    std::vector<vertexUVNormal> &outVertices, std::vector<uint> &outIndices, std::vector<const Texture *> &outTextures,
    JPH::Mat44 &transform)
{
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    outVertices.reserve(mesh->mNumVertices);
    //Roughly preallocate some memory so that lots of tiny allocations are not made, and then we later reduce size to fit
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

    //This is an awful, horrendous way of getting rid of 2 specific meshes that I do not want. However, this is the easiest
    //way without having to learn blender to edit the mesh.
    //TODO: Use blender to get rid of these 2 meshes instead of this horrible method
    if (outIndices.size() == 216)
    {
        outVertices.clear();
        outIndices.clear();
        outTextures.clear();

        return;
    }

    m_objects.emplace_back(PhysicsObjectFactory::ConstructDynamicMesh(1000, m_physics, positions, outIndices, transform));

    LoadMaterialTextures(material, aiTextureType_DIFFUSE, Texture::TextureType::diffuse, directory, outTextures);
    LoadMaterialTextures(material, aiTextureType_SPECULAR, Texture::TextureType::specular, directory, outTextures);
}


void DynamicModel::Draw(Shader &shader, const JPH::Mat44 &projectionMatrix, const JPH::Mat44 &viewMatrix)
{
    StaticModel::Draw(shader, projectionMatrix, viewMatrix);
}


void DynamicModel::Draw(Shader &shader, const JPH::Mat44 &projectionMatrix, const JPH::Mat44 &viewMatrix,
    const JPH::Mat44 &modelMatrix)
{
    StaticModel::Draw(shader, projectionMatrix, viewMatrix, modelMatrix);
}


void DynamicModel::SetPosition(const JPH::Vec3 &position)
{
    for (auto& object : m_objects)
    {
        m_physics.SetPosition(object.bodyID, position);
    }
}


JPH::Vec3 DynamicModel::GetPosition() const
{
    return m_physics.GetPosition(m_objects[0].bodyID);
}


void DynamicModel::AddVelocity(const JPH::Vec3 &velocity)
{
    for (auto& object : m_objects)
    {
        m_physics.AddVelocity(object.bodyID, velocity);
    }
}


void DynamicModel::SetVelocity(const JPH::Vec3 &velocity)
{
    for (auto& object : m_objects)
    {
        m_physics.SetVelocity(object.bodyID, velocity);
    }
}


void DynamicModel::SetRotation(const JPH::Vec3& rotation)
{
    for (auto& object : m_objects)
    {
        m_physics.SetRotation(object.bodyID, rotation);
    }
}


JPH::Mat44 DynamicModel::GetModelMatrix() const
{
    return  JPH::Mat44::sTranslation(m_physics.GetPosition(m_objects[0].bodyID)) *
            JPH::Mat44::sRotation(m_physics.GetRotation(m_objects[0].bodyID));
}


bool DynamicModel::CastRayAgainstAllMeshes(JPH::Vec3 rayOrigin, JPH::Vec3 rayDirection)
{
    JPH::Vec3 rayEnd = rayOrigin + rayDirection * 1000.0f;

    for (auto& object : m_objects)
    {
        JPH::Body* body = m_physics.GetBodyManager().TryGetBody(object.bodyID);
        if (!body)
            continue;

        const JPH::Shape* shape = body->GetShape();
        if (!shape)
            continue;

        // Get the body's world transform
        JPH::Mat44 bodyTransform = body->GetWorldTransform();

        // Transform ray to body's local space
        JPH::Mat44 invTransform = bodyTransform.Inversed();
        JPH::Vec3 localOrigin = invTransform * rayOrigin;
        JPH::Vec3 localEnd = invTransform * rayEnd;
        JPH::Vec3 localDirection = localEnd - localOrigin;

        JPH::RayCast ray;
        ray.mOrigin = localOrigin;
        ray.mDirection = localDirection;

        JPH::RayCastResult result;
        if (shape->CastRay(ray, JPH::SubShapeIDCreator(), result))
            return true;
    }

    return false;
}

void DynamicModel::RemoveFromPhysics()
{
    for (auto& object : m_objects)
        m_physics.RemoveBody(object.bodyID);
}





