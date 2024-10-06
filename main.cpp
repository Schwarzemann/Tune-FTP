#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "LorenzAttractor.h"
#include "Renderer.h"
#include "CameraControls.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

LorenzAttractor attractor;
Renderer* renderer;
CameraControls controls;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    controls.mouseButtonCallback(button, action);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    controls.cursorPosCallback(xpos, ypos);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "Lorenz Attractor", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;
    if (glewInit() != GLEW_OK) return -1;

    // Set callbacks
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Initialize classes
    renderer = new Renderer(attractor);
    renderer->initialize();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Update
        attractor.update();
        controls.update();

        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 view = controls.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800 / 600.0f, 0.1f, 100.0f);
        renderer->render(view, projection);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    delete renderer;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
