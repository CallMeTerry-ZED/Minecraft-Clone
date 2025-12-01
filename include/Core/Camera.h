/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef CAMERA_H
#define CAMERA_H

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace MinecraftClone
{
    class Camera
    {
    public:
        Camera();
        ~Camera() = default;

        void Update(float deltaTime);

        // Getters
        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;
        glm::vec3 GetPosition() const { return m_position; }
        glm::vec3 GetFront() const { return m_front; }
        glm::vec3 GetUp() const { return m_up; }
        glm::vec3 GetRight() const { return m_right; }
        float GetYaw() const { return m_yaw; }
        float GetPitch() const { return m_pitch; }

        // Setters
        void SetPosition(const glm::vec3& position) { m_position = position; }
        void SetFOV(float fov) { m_fov = fov; UpdateProjection(); }
        void SetAspectRatio(float aspect) { m_aspectRatio = aspect; UpdateProjection(); }
        void SetNearPlane(float nearPlane) { m_nearPlane = nearPlane; UpdateProjection(); }
        void SetFarPlane(float farPlane) { m_farPlane = farPlane; UpdateProjection(); }

        // Movement settings
        void SetMovementSpeed(float speed) { m_movementSpeed = speed; }
        void SetMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

    private:
        void UpdateVectors();
        void UpdateProjection();
        void ProcessKeyboard(float deltaTime);
        void ProcessMouseMovement(float deltaTime);

        // Camera attributes
        glm::vec3 m_position;
        glm::vec3 m_front;
        glm::vec3 m_up;
        glm::vec3 m_right;
        glm::vec3 m_worldUp;

        // Euler angles
        float m_yaw;
        float m_pitch;

        // Camera options
        float m_movementSpeed;
        float m_mouseSensitivity;
        float m_fov;
        float m_aspectRatio;
        float m_nearPlane;
        float m_farPlane;

        glm::mat4 m_projectionMatrix;
    };
}

#endif