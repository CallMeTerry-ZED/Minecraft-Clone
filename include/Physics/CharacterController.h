/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

#pragma once

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <memory>

namespace MinecraftClone
{
    class CharacterController
    {
    public:
        CharacterController(btDiscreteDynamicsWorld* world, const glm::vec3& position);
        ~CharacterController();

        void Update(float deltaTime);

        // Movement
        void SetMoveDirection(const glm::vec3& direction);
        void Jump();

        // Getters
        glm::vec3 GetPosition() const;
        bool IsOnGround() const { return m_isOnGround; }
        float GetHeight() const { return m_height; }
        float GetRadius() const { return m_radius; }

        // Setters
        void SetPosition(const glm::vec3& position);
        void SetMovementSpeed(float speed) { m_movementSpeed = speed; }
        void SetJumpForce(float force) { m_jumpForce = force; }

    private:
        void CheckGround();

        btDiscreteDynamicsWorld* m_world;
        std::unique_ptr<btCapsuleShape> m_shape;
        std::unique_ptr<btRigidBody> m_body;

        glm::vec3 m_moveDirection;
        float m_movementSpeed;
        float m_jumpForce;
        bool m_isOnGround;
        bool m_wantsToJump;

        // Character dimensions
        float m_height;   // Total height (2.0 blocks = 2.0 units)
        float m_radius;   // Capsule radius (0.3 blocks = 0.3 units)
    };
}

#endif

