#ifndef PHYSICSMODEL_H
#define PHYSICSMODEL_H

#include "StaticModel.h"

class DynamicModel : public StaticModel
{
private:
    //ProcessNode function is the same as the StaticModel version but we need to call DynamicModel::ProcessMesh so this function also has to exist
    void ProcessNode(aiNode *node, const aiScene *scene, const std::string &directory, const JPH::Mat44& parentTransformation) override;

    void ProcessMesh(
        aiMesh* mesh, const aiScene* scene, const std::string &directory,
        std::vector<vertexUVNormal> &outVertices,
        std::vector<uint> &outIndices,
        std::vector<const Texture*> &outTextures,
        JPH::Mat44 &transform
    ) override;

public:
    DynamicModel(Renderer& renderer, const std::string &sceneFilepath, Physics &physics);
    DynamicModel(Renderer& renderer, const std::string &sceneFilepath, Physics &physics, const JPH::Mat44 &transform);

    void Draw(Shader &shader, const JPH::Mat44 &projectionMatrix, const JPH::Mat44 &viewMatrix) override;
    void Draw(Shader &shader, const JPH::Mat44 &projectionMatrix, const JPH::Mat44 &viewMatrix, const JPH::Mat44 &modelMatrix) override;

    void AddVelocity(const JPH::Vec3& velocity);

    void SetVelocity(const JPH::Vec3& velocity);
    void SetPosition(const JPH::Vec3& position);
    void SetRotation(const JPH::Vec3& rotation);

    JPH::Vec3 GetPosition() const;
    JPH::Mat44 GetModelMatrix() const;

    bool CastRayAgainstAllMeshes(JPH::Vec3 rayOrigin, JPH::Vec3 rayDirection);
    void RemoveFromPhysics();
};



#endif //PHYSICSMODEL_H
