#include "LorenzAttractor.h"

LorenzAttractor::LorenzAttractor(float dt, int maxPoints)
    : sigma(10.0f), rho(28.0f), beta(8.0f / 3.0f), dt(dt), maxPoints(maxPoints), point(0.1f, 0.0f, 0.0f) {
    points.push_back(point);
}

void LorenzAttractor::update() {
    float dx = sigma * (point.y - point.x);
    float dy = point.x * (rho - point.z) - point.y;
    float dz = point.x * point.y - beta * point.z;

    point.x += dx * dt;
    point.y += dy * dt;
    point.z += dz * dt;

    points.push_back(point);
    if (points.size() > maxPoints) {
        points.erase(points.begin());
    }
}

const std::vector<glm::vec3>& LorenzAttractor::getPoints() const {
    return points;
}
