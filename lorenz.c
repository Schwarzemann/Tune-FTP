#include "lorenz.h"

void lorenz(double x, double y, double z, double s, double r, double b, int n, double *xout, double *yout, double *zout) {
    double dt = 0.01;
    for (int i = 0; i < n; i++) {
        double dx = s*(y-x);
        double dy = r*x - y - x*z;
        double dz = x*y - b*z;
        x += dt*dx;
        y += dt*dy;
        z += dt*dz;
        xout[i] = x;
        yout[i] = y;
        zout[i] = z;
    }
}
