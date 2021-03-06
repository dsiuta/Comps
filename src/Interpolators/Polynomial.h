#ifndef INTERPOLATOR_POLYNOMIAL_H
#define INTERPOLATOR_POLYNOMIAL_H
#include "../Global.h"
#include "Interpolator.h"

//! Fits a polynomial of specified order using least-squared error. Under development.
class InterpolatorPolynomial: public Interpolator {
   public:
      InterpolatorPolynomial(const Options& iOptions);
      bool needsSorted() const {return true;};
      bool isMonotonic() const {return false;};
   private:
      float interpolateCore(float x, const std::vector<float>& iX, const std::vector<float>& iY) const;
      int mOrder;
};
#endif
