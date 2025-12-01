/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Physics/CharacterController.h"
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <cmath>

namespace MinecraftClone
{
    CharacterController::CharacterController(btDiscreteDynamicsWorld* world, const glm::vec3& position)
        : m_world(world)
        , m_moveDirection(0.0f, 0.0f, 0.0f)
        , m_movementSpeed(5.0f)
        , m_jumpForce(8.0f)
        , m_isOnGround(false)
        , m_wantsToJump(false)
        , m_height(1.8f)  // 1.8 blocks tall
        , m_radius(0.3f)  // 0.3 block radius
    {
        // Create capsule shape (height is the distance between sphere centers)
        float capsuleHeight = m_height - 2.0f * m_radius;
        m_shape = std::make_unique<btCapsuleShape>(m_radius, capsuleHeight);

        // Create rigid body
        // Position capsule so its bottom is at position.y (offset by half height)
        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(btVector3(position.x, position.y + m_radius + capsuleHeight * 0.5f, position.z));

        btScalar mass(1.0f);
        btVector3 localInertia(0, 0, 0);
        m_shape->calculateLocalInertia(mass, localInertia);

        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, m_shape.get(), localInertia);
        m_body = std::make_unique<btRigidBody>(rbInfo);

        // Set character controller properties
        m_body->setFriction(0.0f);
        m_body->setRestitution(0.0f);
        m_body->setAngularFactor(0.0f); // Prevent rotation
        m_body->setActivationState(DISABLE_DEACTIVATION); // Always active

        // Add to world
        m_world->addRigidBody(m_body.get());
    }

    CharacterController::~CharacterController()
    {
        if (m_body && m_world)
        {
            m_world->removeRigidBody(m_body.get());
        }
    }

    void CharacterController::Update(float /*deltaTime*/)
    {
        if (!m_body)
        {
            return;
        }

        // Check if on ground
        CheckGround();

        // Apply movement
        btTransform transform = m_body->getWorldTransform();
        btVector3 currentVelocity = m_body->getLinearVelocity();

        // Calculate desired horizontal velocity
        // Smooth out movement to prevent jitter
        btVector3 desiredVelocity(
            m_moveDirection.x * m_movementSpeed,
            currentVelocity.y(), // Keep Y velocity (gravity/jumping)
            m_moveDirection.z * m_movementSpeed
        );
        
        // Smooth interpolation to prevent sudden velocity changes
        btVector3 currentHorizontalVel(currentVelocity.x(), 0, currentVelocity.z());
        btVector3 desiredHorizontalVel(desiredVelocity.x(), 0, desiredVelocity.z());
        btVector3 horizontalVelocity = currentHorizontalVel.lerp(desiredHorizontalVel, 0.3f);
        horizontalVelocity.setY(desiredVelocity.y()); // Keep Y velocity unchanged

        // Apply jump
        // Also allow jump if vertical velocity is very small (nearly on ground) to handle edge cases
        bool canJump = m_isOnGround || (fabs(currentVelocity.y()) < 0.5f && currentVelocity.y() >= 0.0f);
        if (m_wantsToJump && canJump)
        {
            horizontalVelocity.setY(m_jumpForce);
            m_wantsToJump = false;
        }

        // Apply velocity
        m_body->setLinearVelocity(horizontalVelocity);

        // Clear move direction for next frame
        m_moveDirection = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    void CharacterController::SetMoveDirection(const glm::vec3& direction)
    {
        m_moveDirection = direction;
    }

    void CharacterController::Jump()
    {
        // Always set jump request - the Update function will check if we can actually jump
        // This allows for more responsive jumping
        m_wantsToJump = true;
    }

    glm::vec3 CharacterController::GetPosition() const
    {
        if (!m_body)
        {
            return glm::vec3(0.0f);
        }

        btTransform transform = m_body->getWorldTransform();
        btVector3 origin = transform.getOrigin();
        // Return position of feet (subtract half capsule height)
        float capsuleHeight = m_height - 2.0f * m_radius;
        return glm::vec3(origin.x(), origin.y() - m_radius - capsuleHeight * 0.5f, origin.z());
    }

    void CharacterController::SetPosition(const glm::vec3& position)
    {
        if (!m_body)
        {
            return;
        }

        btTransform transform = m_body->getWorldTransform();
        // Position capsule so its bottom is at position.y
        float capsuleHeight = m_height - 2.0f * m_radius;
        transform.setOrigin(btVector3(position.x, position.y + m_radius + capsuleHeight * 0.5f, position.z));
        m_body->setWorldTransform(transform);
        m_body->setLinearVelocity(btVector3(0, 0, 0));
    }

    void CharacterController::CheckGround()
    {
        if (!m_body || !m_world)
        {
            m_isOnGround = false;
            return;
        }

        // Raycast downward to check for ground
        // Check just below the capsule bottom
        btTransform transform = m_body->getWorldTransform();
        btVector3 from = transform.getOrigin();
        float capsuleHeight = m_height - 2.0f * m_radius;
        // Check from capsule center down to just below the bottom of the capsule
        btVector3 to = from - btVector3(0, capsuleHeight * 0.5f + m_radius + 0.2f, 0);

        btCollisionWorld::ClosestRayResultCallback callback(from, to);
        m_world->rayTest(from, to, callback);

        // Consider on ground if we hit something reasonably close (within the raycast distance)
        // The fraction check ensures we're not detecting ground that's too far away
        m_isOnGround = callback.hasHit() && callback.m_closestHitFraction < 0.9f;
    }
}

