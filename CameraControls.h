#ifndef CAMERA_CONTROLS_H
#define CAMERA_CONTROLS_H

#include <glm/glm.hpp>

class CameraControls {
public:
    CameraControls();
    void mouseButtonCallback(int button, int action);
    void cursorPosCallback(double xpos, double ypos);
    void update();

    glm::mat4 getViewMatrix() const;

private:
    bool leftButtonPressed;
    double lastX, lastY;
    float yaw, pitch, distance;
};

#endif // CAMERA_CONTROLS_H
