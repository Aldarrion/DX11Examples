#pragma once
#include <DirectXMath.h>
#include <iostream>

namespace dx = DirectX;

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 5.0f;
constexpr float SENSITIVTY = 0.1f;
constexpr float ZOOM = 45.0f;

class Camera {
public:
    // Camera Attributes
    dx::XMFLOAT3 Position;
    dx::XMFLOAT3 Front;
    dx::XMFLOAT3 Up;
    dx::XMFLOAT3 Right;
    dx::XMFLOAT3 WorldUp;

    // Eular Angles
    float Yaw;
    float Pitch;
    
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(
            dx::XMFLOAT3 position = dx::XMFLOAT3(0.0f, 0.0f, 0.0f), 
            dx::XMFLOAT3 up = dx::XMFLOAT3(0.0f, 1.0f, 0.0f), 
            float yaw = YAW,
            float pitch = PITCH)
            : Front(dx::XMFLOAT3(0.0f, 0.0f, -1.0f))
            , MovementSpeed(SPEED)
            , MouseSensitivity(SENSITIVTY)
            , Zoom(ZOOM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) 
            : Front(dx::XMFLOAT3(0.0f, 0.0f, -1.0f))
            , MovementSpeed(SPEED)
            , MouseSensitivity(SENSITIVTY)
            , Zoom(ZOOM) {
        Position = dx::XMFLOAT3(posX, posY, posZ);
        WorldUp = dx::XMFLOAT3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    dx::XMMATRIX GetViewMatrix() const {
        const dx::XMVECTOR pos = XMLoadFloat3(&Position);
        const auto focusPosition = dx::XMVectorAdd(XMLoadFloat3(&Position), XMLoadFloat3(&Front));
        const dx::XMVECTOR up = XMLoadFloat3(&Up);
        return dx::XMMatrixLookAtRH(pos, focusPosition, up);
    }
    
    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(CameraMovement direction, float deltaTime) {
        using namespace dx;
        float velocity = MovementSpeed * deltaTime;
        if (direction == CameraMovement::FORWARD)
            XMStoreFloat3(&Position, XMLoadFloat3(&Position) - XMLoadFloat3(&Front) * velocity);
        if (direction == CameraMovement::BACKWARD)
            XMStoreFloat3(&Position, XMLoadFloat3(&Position) + XMLoadFloat3(&Front) * velocity);
        if (direction == CameraMovement::LEFT)
            XMStoreFloat3(&Position, XMLoadFloat3(&Position) - XMLoadFloat3(&Right) * velocity);
        if (direction == CameraMovement::RIGHT)
            XMStoreFloat3(&Position, XMLoadFloat3(&Position) + XMLoadFloat3(&Right) * velocity);
        if (direction == CameraMovement::UP)
            XMStoreFloat3(&Position, XMLoadFloat3(&Position) + XMLoadFloat3(&Up) * velocity);
        if (direction == CameraMovement::DOWN)
            XMStoreFloat3(&Position, XMLoadFloat3(&Position) - XMLoadFloat3(&Up) * velocity);
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Eular angles
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        if (Zoom >= 1.0f && Zoom <= 45.0f)
            Zoom -= yoffset;
        if (Zoom <= 1.0f)
            Zoom = 1.0f;
        if (Zoom >= 45.0f)
            Zoom = 45.0f;
    }

private:
    // Calculates the front vector from the Camera's (updated) Eular Angles
    void updateCameraVectors()
    {
        using namespace dx;
        // Calculate the new Front vector
        XMFLOAT3 front;
        front.x = cos(XMConvertToRadians(Yaw)) * cos(XMConvertToRadians(Pitch));
        front.y = sin(XMConvertToRadians(Pitch));
        front.z = sin(XMConvertToRadians(Yaw)) * cos(XMConvertToRadians(Pitch));
        XMStoreFloat3(&Front, XMVector3Normalize(XMLoadFloat3(&front)));
        
        // Also re-calculate the Right and Up vector
        const auto frontV = XMLoadFloat3(&Front);
        const auto worldUpV = XMLoadFloat3(&WorldUp);
        XMStoreFloat3(&Right, XMVector3Normalize(XMVector3Cross(frontV, worldUpV))); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.

        const auto rightV = XMLoadFloat3(&Right);
        XMStoreFloat3(&Up, XMVector3Normalize(XMVector3Cross(rightV, frontV)));
    }
};