#include "Renderer.h"
#include <GL/glew.h>

Renderer::Renderer(const LorenzAttractor& attractor) : attractor(attractor), vao(0), vbo(0) {}

void Renderer::initialize() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
}

void Renderer::render(const glm::mat4& view, const glm::mat4& projection) {
    const auto& points = attractor.getPoints();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_DYNAMIC_DRAW);

    // Shader (assume shader is built and linked)
    // ... Shader code comes here ...

    // Set uniforms
    // glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    // glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    // Draw
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glDrawArrays(GL_LINE_STRIP, 0, points.size());
    glDisableVertexAttribArray(0);

    glBindVertexArray(0);
}
