#include "Camera.h"

#include "glm/gtx/quaternion.hpp"

Camera::Camera()
{
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    right = glm::vec3(1.0f, 0.0f, 0.0f);
    world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    movement_speed = 2.5f;
    mouse_sensitivity = 0.1f;
    zoom = 45.0f;
    update_camera_vectors();
}

Camera::Camera(const glm::vec3 position, const glm::vec3 up, const float yaw, const float pitch, float movement_speed, float mouse_sensitivity)
{
    this->position = position;
    this->world_up = up;
    this->yaw = yaw;
    this->pitch = pitch;
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    this->movement_speed = movement_speed;
    this->mouse_sensitivity = mouse_sensitivity;
    zoom = 45.0f;
    update_camera_vectors();
}

void Camera::update_camera_vectors()
{
    // calculate the new front vector
    glm::vec3 new_front;
    new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    new_front.y = sin(glm::radians(pitch));
    new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = normalize(new_front);
    // also re-calculate the right and up vector
    right = normalize(cross(front, world_up));
    up = normalize(cross(right, front));
}

glm::mat4 Camera::get_view_matrix() const
{
    return lookAt(position, position + front, up);
}

void Camera::process_mouse_scroll(const float y_offset)
{
    if (zoom >= 1.0f && zoom <= 45.0f)
        zoom -= y_offset;
    if (zoom <= 1.0f)
        zoom = 1.0f;
    if (zoom >= 45.0f)
        zoom = 45.0f;
}

void Camera::process_keyboard(const Direction direction, const float delta_time)
{
    const float velocity = movement_speed * delta_time;
    if (direction == Direction::FORWARD)
        position += front * velocity;
    if (direction == Direction::BACKWARD)
        position -= front * velocity;
    if (direction == Direction::LEFT)
        position -= right * velocity;
    if (direction == Direction::RIGHT)
        position += right * velocity;
    if (direction == Direction::UP)
        position += up * velocity;
    if (direction == Direction::DOWN)
        position -= up * velocity;
    update_camera_vectors();
}

void Camera::process_mouse_movement(const float x_offset, const float y_offset, const bool constrain_pitch)
{
    yaw += x_offset * mouse_sensitivity;
    pitch += y_offset * mouse_sensitivity;
    if (constrain_pitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }
    update_camera_vectors();
}