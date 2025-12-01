/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef RAYCAST_H
#define RAYCAST_H

#pragma once

#include "World/World.h"
#include "World/BlockType.h"
#include <glm/glm.hpp>

namespace MinecraftClone
{
    struct RaycastResult
    {
        bool hit;
        glm::ivec3 blockPosition;      // Position of the block that was hit
        glm::ivec3 faceNormal;         // Normal of the face that was hit
        glm::ivec3 adjacentPosition;   // Position where we can place a block
        float distance;                 // Distance from ray origin to hit point
    };

    class Raycast
    {
    public:
        // Cast a ray from origin in direction, returns hit information
        static RaycastResult Cast(const glm::vec3& origin, const glm::vec3& direction, World* world, float maxDistance = 10.0f);
    };
}

#endif