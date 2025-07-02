#include "PhysicsObjectFactory.h"

#include "Physics.h"
#include "Physics/EActivation.h"
#include "Physics/Body/BodyCreationSettings.h"
#include "Physics/Body/MassProperties.h"
#include "Physics/Collision/Shape/MeshShape.h"
#include "Physics/Collision/Shape/SphereShape.h"


PhysicsObjectFactory::Object PhysicsObjectFactory::ConstructSphere(const ObjectInfo &objInfo, Physics& physics, float radius)
{
    JPH::MassProperties massPropertiesSphere;
    massPropertiesSphere.ScaleToMass(objInfo.mass);

    JPH::SphereShapeSettings sphereSettings{radius};
    sphereSettings.SetEmbedded();

    auto shapeResult = sphereSettings.Create();
    ASSERT_LOG(shapeResult.IsValid(), "Failure to create a shape: " + shapeResult.GetError());

    JPH::ShapeRefC shape = shapeResult.Get();

    JPH::BodyCreationSettings bodySettings{
        shape, objInfo.position,
        objInfo.rotation, JPH::EMotionType::Static,
        JPHImpls::ObjectLayers::NON_MOVING
    };

    bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
    bodySettings.mMassPropertiesOverride = massPropertiesSphere;

    return {physics.AddBody(bodySettings), false};
}

PhysicsObjectFactory::Object PhysicsObjectFactory::ConstructStaticMesh(
    float mass, Physics &physics,
    const std::vector<JPH::Vec3> &positions, const std::vector<uint> &indices, const JPH::Mat44& verticesTransformation
)
{
    //This is going to be a 2 step process. We first want to make the data un-indexed and get all true vertices
    //Then we want to apply transformations to all of these vertices
    std::vector<JPH::Vec3> allVertices;
    JPH::Array<JPH::Triangle> triangleList;

    allVertices.reserve(indices.size());

    //Get all vertices
    for (uint i : indices)
    {
        allVertices.emplace_back(positions[i]);
    }

    //Transform them
    for (int i = 0; i < allVertices.size(); i++)
    {
        JPH::Vec4 posVec4(allVertices[i].GetX(), allVertices[i].GetY(), allVertices[i].GetZ(), 1.0f);
        JPH::Vec4 transformed = verticesTransformation * posVec4;
        allVertices[i] = JPH::Vec3(transformed.GetX(), transformed.GetY(), transformed.GetZ());
    }

    //Make a list of triangles out of the vertices
    for (int i = 0; i < allVertices.size(); i += 3)
    {
        triangleList.emplace_back(allVertices[i], allVertices[i + 1], allVertices[i + 2]);
    }

    JPH::MeshShapeSettings meshSettings{triangleList};
    meshSettings.SetEmbedded();

    auto shapeResult = meshSettings.Create();
    ASSERT_LOG(shapeResult.IsValid(), "Failure to create a shape: " + shapeResult.GetError());

    JPH::ShapeRefC shape = shapeResult.Get();

    JPH::MassProperties massPropertiesSphere;
    massPropertiesSphere.ScaleToMass(mass);

    JPH::BodyCreationSettings bodySettings{
        shape, {0, 0, 0},
        JPH::Quat::sIdentity(), JPH::EMotionType::Static,
        JPHImpls::ObjectLayers::NON_MOVING
    };

    bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
    bodySettings.mMassPropertiesOverride = massPropertiesSphere;

    return {physics.AddBody(bodySettings), false};
}

PhysicsObjectFactory::Object PhysicsObjectFactory::ConstructDynamicMesh(float mass, Physics &physics,
    const std::vector<JPH::Vec3> &positions, const std::vector<uint> &indices, const JPH::Mat44 &verticesTransformation)
{
    //This is going to be a 2 step process. We first want to make the data un-indexed and get all true vertices
    //Then we want to apply transformations to all of these vertices
    std::vector<JPH::Vec3> allVertices;
    JPH::Array<JPH::Triangle> triangleList;

    allVertices.reserve(indices.size());

    //Get all vertices
    for (uint i : indices)
    {
        allVertices.emplace_back(positions[i]);
    }

    //Transform them
    for (int i = 0; i < allVertices.size(); i++)
    {
        JPH::Vec4 posVec4(allVertices[i].GetX(), allVertices[i].GetY(), allVertices[i].GetZ(), 1.0f);
        JPH::Vec4 transformed = verticesTransformation * posVec4;
        allVertices[i] = JPH::Vec3(transformed.GetX(), transformed.GetY(), transformed.GetZ());
    }

    //Make a list of triangles out of the vertices
    for (int i = 0; i < allVertices.size(); i += 3)
    {
        triangleList.emplace_back(allVertices[i], allVertices[i + 1], allVertices[i + 2]);
    }

    JPH::MeshShapeSettings meshSettings{triangleList};
    meshSettings.SetEmbedded();

    auto shapeResult = meshSettings.Create();
    ASSERT_LOG(shapeResult.IsValid(), "Failure to create a shape: " + shapeResult.GetError());

    JPH::ShapeRefC shape = shapeResult.Get();

    JPH::MassProperties massPropertiesSphere;
    massPropertiesSphere.ScaleToMass(mass);

    JPH::BodyCreationSettings bodySettings{
        shape, {0, 0, 0},
        JPH::Quat::sIdentity(), JPH::EMotionType::Kinematic,
        JPHImpls::ObjectLayers::MOVING
    };

    bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
    bodySettings.mMassPropertiesOverride = massPropertiesSphere;

    return {physics.AddBody(bodySettings), true};
}
