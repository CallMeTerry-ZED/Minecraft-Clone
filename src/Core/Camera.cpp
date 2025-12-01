/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include <spdlog/spdlog.h>
#include "Core/Camera.h"
#include "Core/Input.h"
#include "Physics/CharacterController.h"
#include <GLFW/glfw3.h>

namespace MinecraftClone
{
    Camera::Camera()
        : m_position(0.0f, 0.0f, 3.0f)
        , m_front(0.0f, 0.0f, -1.0f)
        , m_up(0.0f, 1.0f, 0.0f)
        , m_worldUp(0.0f, 1.0f, 0.0f)
        , m_yaw(-90.0f)
        , m_pitch(0.0f)
        , m_movementSpeed(5.0f)
        , m_mouseSensitivity(0.05f)  // Reduced from 0.1f for smoother rotation
        , m_fov(45.0f)
        , m_aspectRatio(16.0f / 9.0f)
        , m_nearPlane(0.1f)
        , m_farPlane(1000.0f)
    {
        UpdateVectors();
        UpdateProjection();
    }

    void Camera::Update(float deltaTime)
    {
        ProcessKeyboard(deltaTime);
        ProcessMouseMovement(deltaTime);
    }

    void Camera::ProcessKeyboard(float deltaTime)
    {
        if (m_characterController)
        {
            // Use physics-based movement
            glm::vec3 moveDirection(0.0f, 0.0f, 0.0f);
            static bool logged = false;
            if (!logged)
            {
                spdlog::info("Camera using physics-based movement");
                logged = true;
            }

            if (Input::IsKeyDown(GLFW_KEY_W))
            {
                moveDirection += m_front;
            }
            if (Input::IsKeyDown(GLFW_KEY_S))
            {
                moveDirection -= m_front;
            }
            if (Input::IsKeyDown(GLFW_KEY_A))
            {
                moveDirection -= m_right;
            }
            if (Input::IsKeyDown(GLFW_KEY_D))
            {
                moveDirection += m_right;
            }

            // Normalize horizontal movement (prevent faster diagonal movement)
            if (glm::length(moveDirection) > 0.0f)
            {
                moveDirection.y = 0.0f; // Remove vertical component
                moveDirection = glm::normalize(moveDirection);
            }

            m_characterController->SetMoveDirection(moveDirection);

            // Jump
            if (Input::IsKeyPressed(GLFW_KEY_SPACE))
            {
                m_characterController->Jump();
            }

            // Update camera position from physics body
            glm::vec3 controllerPos = m_characterController->GetPosition();
            // Adjust camera height (character controller is at feet, camera is at eyes)
            controllerPos.y += m_characterController->GetHeight() * 0.9f; // Eye level
            m_position = controllerPos;
        }
        else
        {
            // Fallback to old movement (no physics)
            static bool logged = false;
            if (!logged)
            {
                spdlog::info("Camera using fly-cam movement (no character controller)");
                logged = true;
            }
            float velocity = m_movementSpeed * deltaTime;

            if (Input::IsKeyDown(GLFW_KEY_W))
            {
                m_position += m_front * velocity;
            }
            if (Input::IsKeyDown(GLFW_KEY_S))
            {
                m_position -= m_front * velocity;
            }
            if (Input::IsKeyDown(GLFW_KEY_A))
            {
                m_position -= m_right * velocity;
            }
            if (Input::IsKeyDown(GLFW_KEY_D))
            {
                m_position += m_right * velocity;
            }
            if (Input::IsKeyDown(GLFW_KEY_SPACE))
            {
                m_position += m_worldUp * velocity;
            }
            if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
            {
                m_position -= m_worldUp * velocity;
            }
        }
    }

    void Camera::ProcessMouseMovement(float /*deltaTime*/)
    {
        if (!Input::IsMouseLocked())
        {
            return;
        }

        float mouseDeltaX, mouseDeltaY;
        Input::GetMouseDelta(mouseDeltaX, mouseDeltaY);

        // Invert Y for natural mouse look
        mouseDeltaY = -mouseDeltaY;

        mouseDeltaX *= m_mouseSensitivity;
        mouseDeltaY *= m_mouseSensitivity;

        m_yaw += mouseDeltaX;
        m_pitch += mouseDeltaY;

        // Constrain pitch
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;

        UpdateVectors();
    }

    void Camera::UpdateVectors()
    {
        // Calculate the new front vector
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);

        // Recalculate right and up vectors
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

    void Camera::UpdateProjection()
    {
        m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        return m_projectionMatrix;
    }
}