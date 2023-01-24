#include "calc.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace my_lib {

// 定数
constexpr double kDt   = 1.0e-3;  // Differential interval
constexpr int    kStep = 100000;  // Time step count
constexpr double kX0   = 1.0;     // Initial value of x
constexpr double kY0   = 1.0;     // Initial value of y
constexpr double kZ0   = 1.0;     // Initial value of z

bool Calc::lorenz_runge_kutta(std::vector<std::vector<double>>& res) {
  double xyz[] = {kX0, kY0, kZ0};
  double xyz_l_0[3];  
  double xyz_l_1[3];  
  double xyz_l_2[3];  
  double xyz_l_3[3];  
  double xyz_w[3];   
  unsigned int i;    
  unsigned int j;    

  try {
    for (i = 0; i < kStep; i++) {
      if (!lorenz(xyz, xyz_l_0)) return false;
      xyz_w[0] = xyz[0] + xyz_l_0[0] * kDt / 2.0;
      xyz_w[1] = xyz[1] + xyz_l_0[1] * kDt / 2.0;
      xyz_w[2] = xyz[2] + xyz_l_0[2] * kDt / 2.0;
      if (!lorenz(xyz_w, xyz_l_1)) return false;
      xyz_w[0] = xyz[0] + xyz_l_1[0] * kDt / 2.0;
      xyz_w[1] = xyz[1] + xyz_l_1[1] * kDt / 2.0;
      xyz_w[2] = xyz[2] + xyz_l_1[2] * kDt / 2.0;
      if (!lorenz(xyz_w, xyz_l_2)) return false;
      xyz_w[0] = xyz[0] + xyz_l_2[0] * kDt;
      xyz_w[1] = xyz[1] + xyz_l_2[1] * kDt;
      xyz_w[2] = xyz[2] + xyz_l_2[2] * kDt;
      if (!lorenz(xyz_w, xyz_l_3)) return false;
      for (j = 0; j < 3; ++j) {
        xyz[j] += (xyz_l_0[j] + 2 * xyz_l_1[j] + 2 * xyz_l_2[j] + xyz_l_3[j])
                * kDt / 6.0;
      }
      res.push_back({xyz[0], xyz[1], xyz[2]});
    }
  } catch (...) {
    return false;  
  }

  return true; 
}

bool Calc::lorenz(const double xyz[], double(&xyz_l)[3]) {
  try {
    xyz_l[0] = -p * xyz[0] + p * xyz[1];
    xyz_l[1] = -xyz[0] * xyz[2] + r * xyz[0] - xyz[1];
    xyz_l[2] = xyz[0] * xyz[1] - b * xyz[2];
  } catch (...) {
    return false;
  }

  return true;
}

}  
