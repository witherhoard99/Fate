#ifndef FRUSTUMCULLING_H
#define FRUSTUMCULLING_H

#include "Util.h"
#include "JPHImpls.h"

#include <vector>
#include <array>
#include <memory>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Geometry/AABox.h>

class FrustumCuller {
private:
    JPH::JobSystemThreadPool m_jobSystem;
    JPH::PhysicsSystem m_physicsSystem;
    JPHImpls::BPLayerInterfaceImpl m_broadPhaseLayerInterface;
    JPHImpls::ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseLayerFilter;
    JPHImpls::ObjectLayerPairCollisionFilterImpl m_objectLayerPairCollisionFilter;

    JPH::RefConst<JPH::ConvexHullShape> m_frustumShape;
    std::vector<JPH::Vec3> m_frustumVertices;
    std::array<JPH::Vec4, 6> m_frustumPlanes;

    // Extract frustum planes from view-projection matrix
    void ExtractFrustumPlanes(const JPH::Mat44& viewProjMatrix) {
        m_frustumPlanes[0] = JPH::Vec4(
            viewProjMatrix(3, 0) + viewProjMatrix(0, 0),
            viewProjMatrix(3, 1) + viewProjMatrix(0, 1),
            viewProjMatrix(3, 2) + viewProjMatrix(0, 2),
            viewProjMatrix(3, 3) + viewProjMatrix(0, 3)
        );

        m_frustumPlanes[1] = JPH::Vec4(
            viewProjMatrix(3, 0) - viewProjMatrix(0, 0),
            viewProjMatrix(3, 1) - viewProjMatrix(0, 1),
            viewProjMatrix(3, 2) - viewProjMatrix(0, 2),
            viewProjMatrix(3, 3) - viewProjMatrix(0, 3)
        );

        m_frustumPlanes[2] = JPH::Vec4(
            viewProjMatrix(3, 0) + viewProjMatrix(1, 0),
            viewProjMatrix(3, 1) + viewProjMatrix(1, 1),
            viewProjMatrix(3, 2) + viewProjMatrix(1, 2),
            viewProjMatrix(3, 3) + viewProjMatrix(1, 3)
        );

        m_frustumPlanes[3] = JPH::Vec4(
            viewProjMatrix(3, 0) - viewProjMatrix(1, 0),
            viewProjMatrix(3, 1) - viewProjMatrix(1, 1),
            viewProjMatrix(3, 2) - viewProjMatrix(1, 2),
            viewProjMatrix(3, 3) - viewProjMatrix(1, 3)
        );

        m_frustumPlanes[4] = JPH::Vec4(
            viewProjMatrix(3, 0) + viewProjMatrix(2, 0),
            viewProjMatrix(3, 1) + viewProjMatrix(2, 1),
            viewProjMatrix(3, 2) + viewProjMatrix(2, 2),
            viewProjMatrix(3, 3) + viewProjMatrix(2, 3)
        );

        m_frustumPlanes[5] = JPH::Vec4(
            viewProjMatrix(3, 0) - viewProjMatrix(2, 0),
            viewProjMatrix(3, 1) - viewProjMatrix(2, 1),
            viewProjMatrix(3, 2) - viewProjMatrix(2, 2),
            viewProjMatrix(3, 3) - viewProjMatrix(2, 3)
        );

        for (auto& plane : m_frustumPlanes)
        {
            JPH::Vec3 normal(plane.GetX(), plane.GetY(), plane.GetZ());
            float length = normal.Length();

            if (length > 1e-6f)
            {
                normal /= length;
                float distance = plane.GetW() / length;
                plane = JPH::Vec4(normal.GetX(), normal.GetY(), normal.GetZ(), distance);
            }
        }
    }

    std::vector<JPH::Vec3> ExtractFrustumVertices(const JPH::Mat44& viewMatrix, const JPH::Mat44& projMatrix) {
        std::vector<JPH::Vec3> vertices;
        vertices.reserve(8);

        JPH::Mat44 viewProj = projMatrix * viewMatrix;
        JPH::Mat44 invViewProj = viewProj.Inversed();

        std::array<JPH::Vec4, 8> ndcVertices = {{
            JPH::Vec4(-1, -1, -1, 1),
            JPH::Vec4( 1, -1, -1, 1),
            JPH::Vec4( 1,  1, -1, 1),
            JPH::Vec4(-1,  1, -1, 1),
            JPH::Vec4(-1, -1,  1, 1),
            JPH::Vec4( 1, -1,  1, 1),
            JPH::Vec4( 1,  1,  1, 1),
            JPH::Vec4(-1,  1,  1, 1)
        }};

        // Transform NDC vertices to world space
        for (const auto& ndcVertex : ndcVertices) {
            JPH::Vec4 worldVertex = invViewProj * ndcVertex;
            if (abs(worldVertex.GetW()) > 1e-6f) {
                worldVertex /= worldVertex.GetW(); // Perspective divide
            }
            vertices.push_back(JPH::Vec3(worldVertex.GetX(), worldVertex.GetY(), worldVertex.GetZ()));
        }

        return vertices;
    }

    JPH::RefConst<JPH::ConvexHullShape> CreateFrustumShape(const std::vector<JPH::Vec3>& vertices) {
        if (vertices.size() < 4) {
            return nullptr;
        }

        JPH::ConvexHullShapeSettings settings(vertices.data(), (int)vertices.size());
        settings.mMaxConvexRadius = 0.0f;

        auto result = settings.Create();
        if (result.HasError()) {
            return nullptr;
        }

        return JPH::RefConst<JPH::ConvexHullShape>(
            dynamic_cast<const JPH::ConvexHullShape*>(
                result.Get().GetPtr()
            )
        );
    }

    bool IsAABBVisible(const JPH::AABox& aabb) const {
        JPH::Vec3 center = aabb.GetCenter();
        JPH::Vec3 extents = aabb.GetExtent();

        // Test against each frustum plane
        for (const auto& plane : m_frustumPlanes) {
            JPH::Vec3 normal(plane.GetX(), plane.GetY(), plane.GetZ());
            float distance = plane.GetW();

            JPH::Vec3 positiveVertex = center;
            if (normal.GetX() >= 0)
                positiveVertex.SetX(center.GetX() + extents.GetX());
            else
                positiveVertex.SetX(center.GetX() - extents.GetX());

            if (normal.GetY() >= 0)
                positiveVertex.SetY(center.GetY() + extents.GetY());
            else
                positiveVertex.SetY(center.GetY() - extents.GetY());

            if (normal.GetZ() >= 0)
                positiveVertex.SetZ(center.GetZ() + extents.GetZ());
            else
                positiveVertex.SetZ(center.GetZ() - extents.GetZ());

            // If the positive vertex is behind the plane, the AABB is completely outside
            float distanceToPlane = normal.Dot(positiveVertex) + distance;
            if (distanceToPlane < 0.0f) {
                return false;
            }
        }

        return true;
    }

    // Precise shape-based culling
    // Does not work for now because no bodies were added to the physics system
    bool IsShapeVisible(const JPH::Body& body) const {
        if (!m_frustumShape) {
            return true;
        }

        JPH::Mat44 bodyTransform = body.GetWorldTransform();
        JPH::RefConst<JPH::Shape> bodyShape = body.GetShape();

        if (!bodyShape) {
            return true;
        }

        JPH::CollideShapeSettings settings;
        settings.mMaxSeparationDistance = 0.0f;
        settings.mActiveEdgeMode = JPH::EActiveEdgeMode::CollideOnlyWithActive;

        JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;

        const JPH::NarrowPhaseQuery& narrowPhase = m_physicsSystem.GetNarrowPhaseQuery();

        narrowPhase.CollideShape(
            m_frustumShape,
            JPH::Vec3::sReplicate(1.0f),
            JPH::Mat44::sIdentity(),
            settings,
            bodyTransform.GetTranslation(),
            collector
        );

        return collector.HadHit();
    }

    void UpdateFrustum(const JPH::Mat44& viewMatrix, const JPH::Mat44& projMatrix) {
        JPH::Mat44 viewProj = projMatrix * viewMatrix;
        ExtractFrustumPlanes(viewProj);

        m_frustumVertices = ExtractFrustumVertices(viewMatrix, projMatrix);
        m_frustumShape = CreateFrustumShape(m_frustumVertices);
    }

public:
    FrustumCuller() :
        m_jobSystem(
            1024,
            std::clamp((int)std::thread::hardware_concurrency() / 2, 1, 12) + 2,
            std::clamp((int)std::thread::hardware_concurrency() / 2, 1, 12)
        )
    {
        m_physicsSystem.Init(8192, 0, 8192, 2048, m_broadPhaseLayerInterface,
                           m_objectVsBroadPhaseLayerFilter, m_objectLayerPairCollisionFilter);
    }

    std::vector<JPH::BodyID> GetVisibleBodies(
        const JPH::BodyLockInterfaceLocking& bodyInterface,
        const std::vector<JPH::BodyID>& allBodies,
        const JPH::Mat44& viewMatrix,
        const JPH::Mat44& projectionMatrix
    ) {
        UpdateFrustum(viewMatrix, projectionMatrix);

        std::vector<JPH::BodyID> visibleBodies;
        std::vector<JPH::BodyID> aabbCandidates;

        visibleBodies.reserve(allBodies.size());
        aabbCandidates.reserve(allBodies.size());

        for (const JPH::BodyID& bodyID : allBodies) {
            JPH::BodyLockRead lock(bodyInterface, bodyID);
            if (!lock.Succeeded()) {
                continue;
            }

            const JPH::Body& body = lock.GetBody();
            JPH::AABox worldAABB = body.GetWorldSpaceBounds();

            // Test AABB against frustum planes
            if (IsAABBVisible(worldAABB)) {
                aabbCandidates.push_back(bodyID);
            }
        }

        // For now, just return AABB candidates (disable shape testing until it is working)
        return aabbCandidates;

        // Phase 2: Precise shape-based culling on AABB candidates
        /*for (const JPH::BodyID& bodyID : aabbCandidates) {
            JPH::BodyLockRead lock(bodyInterface, bodyID);
            if (!lock.Succeeded()) {
                continue;
            }

            const JPH::Body& body = lock.GetBody();

            // Perform precise shape-based culling
            bool isShapeVisible = IsShapeVisible(body);
            if (isShapeVisible) {
                visibleBodies.push_back(bodyID);
            }
        }

        return visibleBodies;*/
    }
};

#endif //FRUSTUMCULLING_H
