#ifndef JPHIMPLS_H
#define JPHIMPLS_H

#include "Util.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>


namespace JPHImpls
{
    namespace ObjectLayers
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer FLOOR = 2;

        static constexpr JPH::ObjectLayer NUM_LAYERS = 3;
    }

    namespace BroadPhaseLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr JPH::uint NUM_LAYERS(2);
    }


    class ObjectLayerPairCollisionFilterImpl : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override;
    };



    class BPLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl();
        uint GetNumBroadPhaseLayers() const override;
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;

#		if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
#		endif

    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[ObjectLayers::NUM_LAYERS];

    };



    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
    };
}



#endif //JPHIMPLS_H
