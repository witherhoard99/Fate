#include "JPHImpls.h"

namespace JPHImpls
{
    bool ObjectLayerPairCollisionFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const
    {
        switch (inLayer1)
        {
        case ObjectLayers::NON_MOVING:
            return inLayer2 == ObjectLayers::MOVING;

        case ObjectLayers::MOVING:
            return true;

        case ObjectLayers::FLOOR:
            return inLayer2 == ObjectLayers::MOVING;

        default:
            JPH_ASSERT(false);
            return false;
        }
    }



    BPLayerInterfaceImpl::BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[ObjectLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[ObjectLayers::MOVING] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[ObjectLayers::FLOOR] = BroadPhaseLayers::NON_MOVING;
    }

    uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
    {
        JPH_ASSERT(inLayer < ObjectLayers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

    #   if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
            const char * BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const
            {
                switch (static_cast<JPH::BroadPhaseLayer::Type>(inLayer))
                {
                case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::NON_MOVING):
                    return "NON_MOVING";

                case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::MOVING):
                    return "MOVING";

                default:
                    JPH_ASSERT(false);
                    return "INVALID";
                }
            }
    #   endif // defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)



    /// Basically mapping BroadPhaseLayers to ObjectLayers (or vice versa I guess)
    bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
    {
        switch (inLayer1)
        {
            case ObjectLayers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;

            case ObjectLayers::MOVING:
                return true;

            default:
                JPH_ASSERT(false);
                return false;
        }
    }
}