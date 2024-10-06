#ifndef RENDERER_H
#define RENDERER_H

#include "LorenzAttractor.h"
#include <GL/glew.h>
#include <glm/glm.hpp>

class Renderer {
public:
    Renderer(const LorenzAttractor& attractor);
    void initialize();
    void render(const glm::mat4& view, const glm::mat4& projection);

private:
    const LorenzAttractor& attractor;
    GLuint vao, vbo;
};

#endif // RENDERER_H
