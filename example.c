#include <stdio.h>
#include "lorenz.h"

int main() {
    double x = 0.1;
    double y = 0.0;
    double z = 0.0;
    double s = 10.0;
    double r = 28.0;
    double b = 8.0/3.0;
    int n = 10000;
    double xout[n], yout[n], zout[n];
    lorenz(x, y, z, s, r, b, n, xout, yout, zout);
    for (int i = 0; i < n; i++) {
        printf("%f, %f, %f\n", xout[i], yout[i], zout[i]);
    }
    return 0;
}
