#ifndef CORRECTOR_K_MEANS_H
#define CORRECTOR_K_MEANS_H
#include "Corrector.h"

//! Date: 20130321
//! Author: Thomas Nipen
//! Creates an ensemble with 'K' members, using the Kmeans algorithm
//! Means that have no closest ensemble members are set to missing
class CorrectorKmeans : public Corrector {
   public:
      CorrectorKmeans(const Options& iOptions, const Data& iData);
   protected:
      void correctCore(const Parameters& iParameters, Ensemble& iUnCorrected) const;
   private:
      int mNumMeans;
      const static float mTol = 1e-3;
};
#endif
