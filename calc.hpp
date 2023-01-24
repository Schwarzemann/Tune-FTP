#ifndef EULER_CALC_HPP_
#define EULER_CALC_HPP_

#include <vector>

namespace my_lib {

class Calc {
  double p;
  double r;
  double b;
  bool lorenz(const double[], double(&)[3]);  

public:
  Calc(double p, double r, double b) : p(p), r(r), b(b) {}     
  bool lorenz_runge_kutta(std::vector<std::vector<double>>&);  
};

}  // namespace my_lib

#endif
