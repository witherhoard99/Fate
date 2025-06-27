#ifndef PHYSICSOBJECT_H
#define PHYSICSOBJECT_H

#include <span>

#include "Physics.h"
#include "Util.h"

#include "Geometry/Sphere.h"
#include "Physics/Body/BodyID.h"

class PhysicsObjectFactory
{
public:
    struct Object
    {
        JPH::BodyID bodyID;
        bool draw;
    };

    struct Objects
    {
        std::vector<JPH::BodyID> bodyID;
    };

    struct ObjectInfo
    {
        JPH::RVec3 position;
        JPH::Quat rotation;

        float mass;
    };

    struct ObjectsInfo
    {
        std::vector<JPH::RVec3> positions;
        std::vector<JPH::Quat> rotations;

        float mass;
    };

    static Object ConstructSphere(    const ObjectInfo& objInfo,  Physics& physics, float radius);

    /**
     *
     * @param positions All of the unique positions for the mesh
     * @param indices The indices to which to get all positions for the mesh (mesh is assumed to be triangles)
     * @param verticesTransformation The model matrix of the mesh to multiply the positions by
     * @return
     */
    static Object ConstructStaticMesh(
        float mass, Physics& physics,
        const std::vector<JPH::Vec3> &positions, const std::vector<uint>& indices,
        const JPH::Mat44& verticesTransformation
    );

    static Object ConstructDynamicMesh(
        float mass, Physics& physics,
        const std::vector<JPH::Vec3> &positions, const std::vector<uint>& indices,
        const JPH::Mat44& verticesTransformation
    );
};



#endif //PHYSICSOBJECT_H
