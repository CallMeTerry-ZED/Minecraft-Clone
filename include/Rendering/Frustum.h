/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef FRUSTUM_H
#define FRUSTUM_H

#pragma once

#include <glm/glm.hpp>

namespace MinecraftClone
{
    class Frustum
    {
    public:
        Frustum() = default;
        ~Frustum() = default;

        // Extract frustum planes from view-projection matrix
        void ExtractPlanes(const glm::mat4& viewProjection);

        // Test if an AABB (axis-aligned bounding box) intersects the frustum
        // Returns true if the AABB is at least partially inside the frustum
        bool IsAABBVisible(const glm::vec3& min, const glm::vec3& max) const;

    private:
        // Frustum planes: left, right, bottom, top, near, far
        // Each plane is represented as: normal.x, normal.y, normal.z, distance
        glm::vec4 m_planes[6];

        // Helper to normalize plane equation
        void NormalizePlane(glm::vec4& plane) const;
    };
}

#endif

