/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/Raycast.h"
#include "World/BlockType.h"
#include <algorithm>
#include <cmath>

namespace MinecraftClone
{
    RaycastResult Raycast::Cast(const glm::vec3& origin, const glm::vec3& direction, World* world, float maxDistance)
    {
        RaycastResult result;
        result.hit = false;
        result.distance = maxDistance;

        if (!world)
        {
            return result;
        }

        // Normalize direction
        glm::vec3 dir = glm::normalize(direction);

        // Start from origin
        glm::vec3 currentPos = origin;

        // Step size (smaller = more accurate but slower)
        const float stepSize = 0.1f;
        const int maxSteps = static_cast<int>(maxDistance / stepSize);

        // Previous block position
        glm::ivec3 lastBlockPos(
            static_cast<int>(std::floor(currentPos.x)),
            static_cast<int>(std::floor(currentPos.y)),
            static_cast<int>(std::floor(currentPos.z))
        );

        for (int i = 0; i < maxSteps; i++)
        {
            currentPos += dir * stepSize;

            // Current block position
            glm::ivec3 blockPos(
                static_cast<int>(std::floor(currentPos.x)),
                static_cast<int>(std::floor(currentPos.y)),
                static_cast<int>(std::floor(currentPos.z))
            );

            // Check if we moved to a new block
            if (blockPos != lastBlockPos)
            {
                // Check if this block is solid
                const Block& block = world->GetBlock(blockPos.x, blockPos.y, blockPos.z);
                if (!block.IsAir() && block.IsSolid())
                {
                    // We hit a solid block!
                    result.hit = true;
                    result.blockPosition = blockPos;
                    result.distance = glm::length(currentPos - origin);

                    // Determine which face was hit by checking which axis we crossed
                    glm::ivec3 diff = blockPos - lastBlockPos;

                    if (diff.x != 0)
                    {
                        result.faceNormal = glm::ivec3(-diff.x, 0, 0);
                    }
                    else if (diff.y != 0)
                    {
                        result.faceNormal = glm::ivec3(0, -diff.y, 0);
                    }
                    else if (diff.z != 0)
                    {
                        result.faceNormal = glm::ivec3(0, 0, -diff.z);
                    }

                    // Calculate adjacent position (where we can place a block)
                    result.adjacentPosition = blockPos + result.faceNormal;

                    return result;
                }

                lastBlockPos = blockPos;
            }
        }

        return result;
    }
}