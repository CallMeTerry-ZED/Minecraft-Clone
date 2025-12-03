/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Rendering/Frustum.h"
#include <cmath>

namespace MinecraftClone
{
    void Frustum::NormalizePlane(glm::vec4& plane) const
    {
        float length = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        if (length > 0.0f)
        {
            plane.x /= length;
            plane.y /= length;
            plane.z /= length;
            plane.w /= length;
        }
    }

    void Frustum::ExtractPlanes(const glm::mat4& viewProjection)
    {
        // Extract frustum planes from view-projection matrix
        // Based on: http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf

        // Left plane
        m_planes[0].x = viewProjection[0][3] + viewProjection[0][0];
        m_planes[0].y = viewProjection[1][3] + viewProjection[1][0];
        m_planes[0].z = viewProjection[2][3] + viewProjection[2][0];
        m_planes[0].w = viewProjection[3][3] + viewProjection[3][0];

        // Right plane
        m_planes[1].x = viewProjection[0][3] - viewProjection[0][0];
        m_planes[1].y = viewProjection[1][3] - viewProjection[1][0];
        m_planes[1].z = viewProjection[2][3] - viewProjection[2][0];
        m_planes[1].w = viewProjection[3][3] - viewProjection[3][0];

        // Bottom plane
        m_planes[2].x = viewProjection[0][3] + viewProjection[0][1];
        m_planes[2].y = viewProjection[1][3] + viewProjection[1][1];
        m_planes[2].z = viewProjection[2][3] + viewProjection[2][1];
        m_planes[2].w = viewProjection[3][3] + viewProjection[3][1];

        // Top plane
        m_planes[3].x = viewProjection[0][3] - viewProjection[0][1];
        m_planes[3].y = viewProjection[1][3] - viewProjection[1][1];
        m_planes[3].z = viewProjection[2][3] - viewProjection[2][1];
        m_planes[3].w = viewProjection[3][3] - viewProjection[3][1];

        // Near plane
        m_planes[4].x = viewProjection[0][3] + viewProjection[0][2];
        m_planes[4].y = viewProjection[1][3] + viewProjection[1][2];
        m_planes[4].z = viewProjection[2][3] + viewProjection[2][2];
        m_planes[4].w = viewProjection[3][3] + viewProjection[3][2];

        // Far plane
        m_planes[5].x = viewProjection[0][3] - viewProjection[0][2];
        m_planes[5].y = viewProjection[1][3] - viewProjection[1][2];
        m_planes[5].z = viewProjection[2][3] - viewProjection[2][2];
        m_planes[5].w = viewProjection[3][3] - viewProjection[3][2];

        // Normalize all planes
        for (int i = 0; i < 6; i++)
        {
            NormalizePlane(m_planes[i]);
        }
    }

    bool Frustum::IsAABBVisible(const glm::vec3& min, const glm::vec3& max) const
    {
        // Test AABB against all 6 frustum planes
        // If the AABB is completely outside any plane, it's not visible
        for (int i = 0; i < 6; i++)
        {
            const glm::vec4& plane = m_planes[i];

            // Find the "positive vertex" - the vertex of the AABB that is furthest
            // in the positive direction of the plane normal
            glm::vec3 positiveVertex;
            positiveVertex.x = (plane.x >= 0.0f) ? max.x : min.x;
            positiveVertex.y = (plane.y >= 0.0f) ? max.y : min.y;
            positiveVertex.z = (plane.z >= 0.0f) ? max.z : min.z;

            // Calculate distance from positive vertex to plane
            float distance = plane.x * positiveVertex.x + 
                           plane.y * positiveVertex.y + 
                           plane.z * positiveVertex.z + 
                           plane.w;

            // If the positive vertex is behind the plane, the entire AABB is outside
            if (distance < 0.0f)
            {
                return false;
            }
        }

        // AABB intersects or is inside the frustum
        return true;
    }
}

