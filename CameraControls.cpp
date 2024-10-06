#include "CameraControls.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

CameraControls::CameraControls()
    : leftButtonPressed(false), lastX(0.0), lastY(0.0), yaw(0.0f), pitch(0.0f), distance(50.0f) {}

void CameraControls::mouseButtonCallback(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        leftButtonPressed = (action == GLFW_PRESS);
    }
}

void CameraControls::cursorPosCallback(double xpos, double ypos) {
    if (leftButtonPressed) {
        float xoffset = static_cast<float>(xpos - lastX);
        float yoffset = static_cast<float>(ypos - lastY);

        yaw += xoffset * 0.1f;
        pitch += yoffset * 0.1f;
    }
    lastX = xpos;
    lastY = ypos;
}

void CameraControls::update() {
    // Optionally implement zoom or other interactions
}

glm::mat4 CameraControls::getViewMatrix() const {
    glm::vec3 direction(
        cos(glm::radians(pitch)) * sin(glm::radians(yaw)),
        sin(glm::radians(pitch)),
        cos(glm::radians(pitch)) * cos(glm::radians(yaw))
    );
    glm::vec3 position = -direction * distance;
    return glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
}
