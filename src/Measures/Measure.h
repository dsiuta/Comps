#ifndef MEASURE_H
#define MEASURE_H
#include "../Component.h"

class Data;
class Parameters;
class Ensemble;
class Obs;
class Options;

//! Computes some measure over an ensemble
class Measure : public Component {
   public:
      Measure(const Options& iOptions, const Data& iData);

      float measure(const Ensemble& iEnsemble, const Parameters& iParameters) const;
      static Measure* getScheme(const Options& rOptions, const Data& iData);
      static Measure* getScheme(const std::string& iTag, const Data& iData);
      //! Does this measure guarantee a positive value?
      bool isPositive() const;
      //! Does this measure guarantee a non-zero value?
      virtual bool isDefinite() const {return false;};
      //! Is this measure positive definite?
      virtual bool isPositiveDefinite() const;
   protected:
      virtual float measureCore(const Ensemble& iEnsemble, const Parameters& iParameters) const = 0;
      virtual bool isPositiveCore() const {return false;};
   private:
      bool mAbsolute;
      mutable float mLastMeasure;
      mutable std::vector<float> mLastEnsemble;
      float mPostPower;
      float mPrePower;
};
#endif
