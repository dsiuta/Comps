#include "Anomaly.h"
#include "../Data.h"
#include "../Ensemble.h"

MeasureAnomaly::MeasureAnomaly(const Options& iOptions, const Data& iData) : Measure(iOptions, iData) {
}

float MeasureAnomaly::measureCore(const Ensemble& iEnsemble, const Parameters& iParameters) const {
   float clim = mData.getClim(iEnsemble.getDate(), 0, iEnsemble.getOffset(), iEnsemble.getLocation(), iEnsemble.getVariable());
   float mean = iEnsemble.getMoment(1);
   return mean - clim;
}
