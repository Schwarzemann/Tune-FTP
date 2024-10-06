#ifndef LORENZ_ATTRACTOR_H
#define LORENZ_ATTRACTOR_H

#include <vector>
#include <glm/glm.hpp>

class LorenzAttractor {
public:
    LorenzAttractor(float dt = 0.01f, int maxPoints = 10000);
    void update();
    const std::vector<glm::vec3>& getPoints() const;

private:
    float sigma, rho, beta, dt;
    int maxPoints;
    glm::vec3 point;
    std::vector<glm::vec3> points;
};

#endif // LORENZ_ATTRACTOR_H
