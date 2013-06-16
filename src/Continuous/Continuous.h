#ifndef CONTINUOUS_H
#define CONTINUOUS_H
#include "../Options.h"
#include "../Data.h"
#include "../Estimators/Estimator.h"
#include "../Ensemble.h"
#include "../Parameters.h"
#include "../Probabilistic.h"

//! Represents the continuous part of the probability distribution
class Continuous : public Probabilistic {
   public:
      //! Options
      //! - invTol (1e-4): When retriving an inverse value, how close must the cdf of that value be to the
      //!   requested cdf? Note this only applies when a Continuous scheme does not implement its
      //!   own inverter.
      Continuous(const Options& iOptions, const Data& iData);

      // Accessors
      float getLikelihood(float iObs, const Ensemble& iEnsemble, const Parameters& iParameters) const;
      float getCdf(float iX, const Ensemble& iEnsemble, const Parameters& iParameters) const;
      float getPdf(float iX, const Ensemble& iEnsemble, const Parameters& iParameters) const;
      float getInv(float iCdf, const Ensemble& iEnsemble, const Parameters& iParameters) const;
      float getMoment(int iMoment, const Ensemble& iEnsemble, const Parameters& iParameters) const;

      void  getDefaultParameters(Parameters& iParameters) const;
      void  updateParameters(const Ensemble& iEnsemble, const Obs& iObs, Parameters& iParameters) const;

      static Continuous* getScheme(const Options& iOptions, const Data& iData);
      static Continuous* getScheme(const std::string& iTag, const Data& iData);
      bool  isMemberDependent() const {return false;};
      virtual bool needsConstantEnsembleSize() const {return false;};
   protected:
      virtual float getCdfCore(float iX, const Ensemble& iEnsemble, const Parameters& iParameters) const = 0;
      //! Default uses numerical differentiation
      virtual float getPdfCore(float iX, const Ensemble& iEnsemble, const Parameters& iParameters) const;
      //! Default uses numerical differentiation
      virtual float getInvCore(float iCdf, const Ensemble& iEnsemble, const Parameters& iParameters) const;
      virtual float getMomentCore(int iMoment, const Ensemble& iEnsemble, const Parameters& iParameters) const;
      virtual void updateParametersCore( const Ensemble& iEnsemble, const Obs& iObs, Parameters& iParameters) const;
      virtual void getDefaultParametersCore(Parameters& iParameters) const {return;}
      static const float mMinPdf = 1e-20;
      float mInvTol;
};
#endif

