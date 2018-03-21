#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

const float HEIGHT = 0.8f;

Camera::Camera(double xpos, double ypos) {
    _position = vec3(0.0f, 0.0f, 3.0f);
    _facing = vec3(0.0f, 0.0f, -1.0f);
    _up = vec3(0.0f, 1.0f, 0.0f);
    _yaw = -90.0f;
    _pitch = 0.0f;
    _zoom = 45.0f;
    _height = HEIGHT;
    _jumpTime = -1.0f;
    _mouseSensitivity = 0.15f;
    _xpos = xpos;
    _ypos = ypos;
}

mat4 Camera::ViewMatrix() {
    return lookAt(_position, _position + _facing, _up);
}

mat4 Camera::ProjMatrix() {
    float aspectRatio = SCREEN_W / SCREEN_H;
    return perspective(radians(_zoom),
                       aspectRatio,
                       0.1f,            // "near" clipping plane
                       100.0f);         // "far" clipping plane
}

vec3 Camera::Position() {
    return _position;
}

void Camera::Look(double xpos, double ypos) {
    GLfloat xoffset = xpos - _xpos; // x coords increase from left to right
    GLfloat yoffset = _ypos - ypos; // y coords increase from top to bottom
    _xpos = xpos;
    _ypos = ypos;

    xoffset *= _mouseSensitivity;
    yoffset *= _mouseSensitivity;

    _yaw   += xoffset;
    _pitch += yoffset;

    // Limit the pitch so we don't flip "backwards"
    if (_pitch > 89.0f) _pitch = 89.0f;
    if (_pitch < -89.0f) _pitch = -89.0f;

    // Update the direction we are facing
    _facing.x = cos(radians(_yaw)) * cos(radians(_pitch));
    _facing.y = sin(radians(_pitch));
    _facing.z = sin(radians(_yaw)) * cos(radians(_pitch));
    _facing = normalize(_facing);
}

void Camera::Move(Direction d) {
    float dt = 0.02f;

    switch(d) {
        case FORWARD:
            _position += _facing * dt;
            break;
        case BACKWARD:
            _position -= _facing * dt;
            break;
        case LEFT:
            _position -= normalize(cross(_facing, _up)) * dt;
            break;
        case RIGHT:
            _position += normalize(cross(_facing, _up)) * dt;
            break;
        default: break;
    }

    _position.y = _height;
}

void Camera::Zoom(float yoffset) {
    _zoom -= yoffset;
    if (_zoom < 1.0f) _zoom = 1.0f;
    if (_zoom > 45.0f) _zoom = 45.0f;
}

void Camera::isDucking(bool b) {
    if (b) _height = HEIGHT - 0.25f;
    else _height = HEIGHT;
    _position.y = _height;
}

void Camera::Jump() {
    // Start a jump (if we're not currently in one)
    if (_jumpTime == -1) _jumpTime = 0;
}

// This fcn gets called approximately once every 0.016 seconds (1/60 FPS)
void Camera::Tick() {
    if (_jumpTime == -1) return;  // don't do anything if we're not jumping

    _height = HEIGHT + 3.0f * _jumpTime + 0.5f * -9.8f * _jumpTime * _jumpTime;
    _position.y = _height;
    _jumpTime += 0.016;

    if (_height < HEIGHT) {
        _height = HEIGHT;
        _position.y = _height;
        _jumpTime = -1;
    }
}
