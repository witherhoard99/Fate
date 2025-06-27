#ifndef FRUSTUMCULLING_H
#define FRUSTUMCULLING_H

#include "Util.h"
#include <Jolt/Jolt.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace FrustumCulling
{
    struct Plane
    {
        JPH::Vec3 normal;
        float d;

        float DistanceToPoint(const JPH::Vec3& point) const
        {
            return normal.Dot(point) + d;
        }
    };

    struct Frustum
    {
        std::array<Plane, 6> planes;
        std::array<JPH::Vec3, 8> corners; // world-space
    };

    inline Frustum ExtractFrustum(const glm::mat4& viewProj)
    {
        Frustum frustum;

        const glm::mat4& m = viewProj;

        // Extract planes
        frustum.planes[0] = { JPH::Vec3(m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0]), m[3][3] + m[3][0] }; // Left
        frustum.planes[1] = { JPH::Vec3(m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0]), m[3][3] - m[3][0] }; // Right
        frustum.planes[2] = { JPH::Vec3(m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1]), m[3][3] + m[3][1] }; // Bottom
        frustum.planes[3] = { JPH::Vec3(m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1]), m[3][3] - m[3][1] }; // Top
        frustum.planes[4] = { JPH::Vec3(m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2]), m[3][3] + m[3][2] }; // Near
        frustum.planes[5] = { JPH::Vec3(m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2]), m[3][3] - m[3][2] }; // Far

        for (Plane& p : frustum.planes)
        {
            float len = p.normal.Length();
            p.normal /= len;
            p.d /= len;
        }

        // Compute inverse to get frustum corners
        glm::mat4 inv = glm::inverse(viewProj);
        int i = 0;
        for (int y = 0; y <= 1; ++y)
        {
            for (int x = 0; x <= 1; ++x)
            {
                for (int z = 0; z <= 1; ++z)
                {
                    glm::vec4 ptNDC = glm::vec4(
                        x ? 1.0f : -1.0f,
                        y ? 1.0f : -1.0f,
                        z ? 1.0f : -1.0f,
                        1.0f
                    );
                    glm::vec4 ptWorld = inv * ptNDC;
                    ptWorld /= ptWorld.w;

                    frustum.corners[i++] = JPH::Vec3(ptWorld.x, ptWorld.y, ptWorld.z);
                }
            }
        }

        return frustum;
    }

    inline bool IsAABBInsideFrustum(const Frustum& frustum, const JPH::AABox& box)
    {
        // First, test box against each frustum plane
        for (const Plane& p : frustum.planes)
        {
            int out = 0;
            for (int i = 0; i < 8; ++i)
            {
                float x = (i & 1) ? box.mMax.GetX() : box.mMin.GetX();
                float y = (i & 2) ? box.mMax.GetY() : box.mMin.GetY();
                float z = (i & 4) ? box.mMax.GetZ() : box.mMin.GetZ();

                if (p.DistanceToPoint({x, y, z}) < 0.0f)
                    ++out;
            }
            if (out == 8) return false; // box is fully outside this plane
        }

        // Second, test frustum against box bounds
        int out;
        out = 0; for (const auto& pt : frustum.corners) if (pt.GetX() > box.mMax.GetX()) ++out; if (out == 8) return false;
        out = 0; for (const auto& pt : frustum.corners) if (pt.GetX() < box.mMin.GetX()) ++out; if (out == 8) return false;
        out = 0; for (const auto& pt : frustum.corners) if (pt.GetY() > box.mMax.GetY()) ++out; if (out == 8) return false;
        out = 0; for (const auto& pt : frustum.corners) if (pt.GetY() < box.mMin.GetY()) ++out; if (out == 8) return false;
        out = 0; for (const auto& pt : frustum.corners) if (pt.GetZ() > box.mMax.GetZ()) ++out; if (out == 8) return false;
        out = 0; for (const auto& pt : frustum.corners) if (pt.GetZ() < box.mMin.GetZ()) ++out; if (out == 8) return false;

        return true;
    }

    // Determines if a specific BodyID should be drawn
    inline bool IsVisible(const JPH::BodyLockInterfaceLocking& bodyInterface, const JPH::BodyID& bodyID, const Frustum& frustum)
    {
        const JPH::Body* body = bodyInterface.TryGetBody(bodyID);
        if (body == nullptr)
        {
            ASSERT_LOG(body == nullptr, "Body should not be nullptr!");
            return true;
        }

        JPH::AABox bounds = body->GetWorldSpaceBounds();
        return IsAABBInsideFrustum(frustum, bounds);
    }

    // Filters a list of BodyIDs to only the visible ones
    inline std::vector<JPH::BodyID> GetVisibleBodies(const JPH::BodyLockInterfaceLocking& bodyInterface,
                                                     const std::vector<JPH::BodyID>& allBodies,
                                                     const glm::mat4& viewMatrix,
                                                     const glm::mat4& projectionMatrix)
    {
        glm::mat4 viewProj = projectionMatrix * viewMatrix;
        Frustum frustum = ExtractFrustum(viewProj);

        std::vector<JPH::BodyID> visible;
        visible.reserve(allBodies.size());

        for (const auto& id : allBodies)
        {
            if (IsVisible(bodyInterface, id, frustum))
                visible.emplace_back(id);
        }

        return visible;
    }
}


#endif //FRUSTUMCULLING_H
