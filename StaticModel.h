#ifndef STATICMODEL_H
#define STATICMODEL_H

#include "Model.h"

#include "Physics.h"
#include "PhysicsObjectFactory.h"

/*
 * This model is intended as a way to add model meshes to the physics engine. Use normal model if you do not want the
 * meshes to be added to the physics engine.
 */
class StaticModel : public Model
{
protected:

    /**
     * This function is the same as the prent ProcessNode function but it now calls the overridden ProcessMesh function,
     * so that it can pass along the model matrix
     */
    virtual void ProcessNode(aiNode* node, const aiScene* scene, const std::string &directory, const JPH::Mat44& parentTransformation) override;

    /**
     * This function is the same as the parent ProcessMesh function but uses StaticObjectFactory to add the meshes to
     * the physics engine
     */
    virtual void ProcessMesh(
        aiMesh* mesh, const aiScene* scene, const std::string &directory,
        std::vector<vertexUVNormal> &outVertices,
        std::vector<uint> &outIndices,
        std::vector<const Texture*> &outTextures,
        JPH::Mat44 &transform
    );

    Physics& m_physics;
    std::vector<PhysicsObjectFactory::Object> m_objects;

public:
    StaticModel(Renderer& renderer, const std::string& sceneFilepath, Physics& physics, bool processModel = true);
    virtual void Draw(Shader& shader, const JPH::Mat44& projectionMatrix, const JPH::Mat44& viewMatrix) override;
    virtual void Draw(
        Shader& shader, const JPH::Mat44& projectionMatrix,
        const JPH::Mat44& viewMatrix, const JPH::Mat44& modelMatrix
    ) override;
};



#endif //STATICMODEL_H
