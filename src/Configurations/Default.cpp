#include "Default.h"
#include "../Uncertainties/Uncertainty.h"
#include "../Selectors/Selector.h"
#include "../Downscalers/Downscaler.h"
#include "../Correctors/Corrector.h"
#include "../Continuous/Continuous.h"
#include "../Discretes/Discrete.h"
#include "../Averagers/Averager.h"
#include "../Calibrators/Calibrator.h"
#include "../Updaters/Updater.h"
#include "../Estimators/Estimator.h"
#include "../Estimators/MaximumLikelihood.h"
#include "../Smoothers/Smoother.h"
#include "../Field.h"
#include "../Deterministic.h"
#include "../ParameterIos/ParameterIo.h"
#include "../Regions/Region.h"

ConfigurationDefault::ConfigurationDefault(const Options& iOptions, const Data& iData) : Configuration(iOptions, iData) {

   mSelectorCache.setName("selectorCache");
   mCorrectorCache.setName("correctorCache");
   // Selector
   {
      std::string tag;
      iOptions.getRequiredValue("selector", tag);
      mSelector = Selector::getScheme(tag, mData);
      addComponent(mSelector, Component::TypeSelector);
   }
   // Downscaler
   {
      std::string tag;
      iOptions.getRequiredValue("downscaler", tag);
      mDownscaler = Downscaler::getScheme(tag, mData);
      addComponent(mDownscaler, Component::TypeDownscaler);
   }
   // Correctors
   {
      std::vector<std::string> tags;
      iOptions.getValues("correctors", tags);
      for(int i = 0; i < (int) tags.size(); i++) {
         const Corrector* corrector = Corrector::getScheme(tags[i], mData);
         mCorrectors.push_back(corrector);
         addComponent(corrector, Component::TypeCorrector);
      }
   }
   // Averager
   {
      std::string tag;
      iOptions.getRequiredValue("averager", tag);
      mAverager = Averager::getScheme(tag, mData);
      addComponent(mAverager, Component::TypeAverager);
   }
   // Updaters
   {
      std::vector<std::string> tags;
      iOptions.getValues("updaters", tags);
      for(int i = 0; i < (int) tags.size(); i++) {
         Updater* updater = Updater::getScheme(tags[i], mData);
         mUpdaters.push_back(updater);
         addComponent(updater, Component::TypeUpdater);
      }
   }
   // Smoother
   {
      std::vector<std::string> tags;
      iOptions.getValues("smoothers", tags);
      for(int i = 0; i < (int) tags.size(); i++) {
         Smoother* smoother = Smoother::getScheme(tags[i], mData);
         mSmoothers.push_back(smoother);
         addComponent(smoother, Component::TypeSmoother);
      }
   }
   // Set up uncertainty object. In the future different ways of combining continuous and discretes
   // can be supported, but for now use UncertaintyCombine.
   {
      std::stringstream ss;
      ss << "tag=unc class=UncertaintyCombine ";
      std::string tag;
      bool continuous = false;
      bool lower      = false;
      bool upper      = false;
      bool discrete   = false;

      if(iOptions.getValue("continuous", tag)) {
         ss << "continuous=" << tag << " ";
         continuous = true;
      }
      if(iOptions.getValue("discreteLower", tag)) {
         ss << "discreteLower=" << tag << " ";
         lower = true;
      }
      if(iOptions.getValue("discreteUpper", tag)) {
         ss << "discreteUpper=" << tag << " ";
         upper = true;
      }
      if(iOptions.getValue("discrete", tag)) {
         ss << "discrete=" << tag << " ";
         discrete = true;
      }

      if(!continuous && !lower && !upper && !discrete) {
         std::stringstream ss;
         ss << "Configuration " << mName << " has no continuous or discrete models specified";
         Global::logger->write(ss.str(), Logger::error);
      }
      if(!continuous && (lower || upper)) {
         std::stringstream ss;
         ss << "Configuration " << mName << " has lower/upperDiscrete defined but no continuous ";
         ss << "model defined. Either provide a) 'continuous' and ('lowerDiscrete' and/or ";
         ss << "'upperDiscrete') or b) 'discrete'.";
      }
      if(continuous && discrete) {
         std::stringstream ss;
         ss << "Configuration " << mName << " has 'continuous' and 'discrete' specified'. ";
         ss << "Use 'lowerDiscrete' and/or 'upperDiscrete' instead of 'discrete'";
         Global::logger->write(ss.str(), Logger::error);
      }

      mUncertainty = Uncertainty::getScheme(Options(ss.str()), mData);
      addComponent(mUncertainty, Component::TypeUncertainty);
   }

   // Calibrator
   {
      std::vector<std::string> tags;
      iOptions.getValues("calibrators", tags);
      for(int i = 0; i < (int) tags.size(); i++) {
         const Calibrator* calibrator = Calibrator::getScheme(tags[i], mData);
         mCalibrators.push_back(calibrator);
         addComponent(calibrator, Component::TypeCalibrator);
      }
   }

   init();
}
ConfigurationDefault::~ConfigurationDefault() {
   /*
   std::map<Component::Type, const EstimatorProbabilistic*>::iterator it;
   for(it = mEstimators.begin(); it != mEstimators.end(); it++) {
      delete it->second;
   }
  */
}

void ConfigurationDefault::getEnsemble(int iDate,
            int iInit,
            float iOffset,
            const Location& iLocation,
            std::string iVariable,
            Ensemble& iEnsemble,
            ProcTypeEns iType) const {

   int region = mRegion->find(iLocation);
   std::vector<Field> slices;
   getSelectorIndicies(iDate, iInit, iOffset, iLocation, iVariable, slices);
   std::vector<float> ensArray;
   std::vector<float> skillArray;
   Ensemble ensDownscaled;

   ///////////////
   // Downscale //
   ///////////////
   if(slices.size() == 0) {
      ensArray.push_back(Global::MV);
      skillArray.push_back(Global::MV);
   }
   else {
      Parameters parDownscaler;
      int downscalerIndex = 0; // Only one downscaler
      getParameters(Component::TypeDownscaler, iDate, iInit, iOffset, region, iVariable, downscalerIndex, parDownscaler);
      for(int i = 0; i < (int) slices.size(); i++) {
         Field slice = slices[i];
         float value = mDownscaler->downscale(slice, iVariable, iLocation, parDownscaler);
         ensArray.push_back(value);
         skillArray.push_back(slice.getSkill());
      }
   }
   ensDownscaled.setValues(ensArray);
   ensDownscaled.setSkills(skillArray);
   ensDownscaled.setInfo(iDate, iInit, iOffset, iLocation, iVariable);

   if(iType == typeUnCorrected) {
      iEnsemble = ensDownscaled;
      return;
   }

   /////////////
   // Correct //
   /////////////
   Key::Ensemble key(iDate, iInit, iOffset, iLocation.getId(), iVariable);
   if(mCorrectorCache.isCached(key)) {
      const Ensemble& ens = mCorrectorCache.get(key);
      iEnsemble = ens;
   }
   else {
      Ensemble ensCorrected = ensDownscaled;
      // Do all correctors in sequence
      for(int i = 0; i < (int) mCorrectors.size(); i++) {
         Parameters parCorrector;
         getParameters(Component::TypeCorrector, iDate, iInit, iOffset, region, iVariable, i, parCorrector);
         mCorrectors[i]->correct(parCorrector, ensCorrected);

         // TODO: Remove old dates from cache that won't be read again (to save space)
      }
      iEnsemble = ensCorrected;
      mCorrectorCache.add(key, iEnsemble);
   }
}

Distribution::ptr ConfigurationDefault::getDistribution(int iDate,
      int iInit,
      float iOffset,
      const Location& iLocation,
      std::string iVariable,
      ProcTypeDist iType) const {
   int region = mRegion->find(iLocation);
   Ensemble ens;
   getEnsemble(iDate, iInit, iOffset, iLocation, iVariable, ens);

   /////////////////
   // Uncertainty //
   /////////////////
   Parameters parUnc;
   getParameters(Component::TypeUncertainty, iDate, iInit, iOffset, region, iVariable, 0, parUnc);
   Distribution::ptr uncD = mUncertainty->getDistribution(ens, parUnc);

   if(mCalibrators.size() == 0 && mUpdaters.size() == 0)
      return uncD;

   if(iType == typeUnCalibrated)
      return uncD;
   
   /////////////////
   // Calibration //
   /////////////////
   std::vector<Distribution::ptr> cal;
   cal.push_back(uncD);

   // Chain calibrators together
   for(int i = 0; i < (int) mCalibrators.size(); i++) {
      Parameters parCal;
      getParameters(Component::TypeCalibrator, iDate, iInit, iOffset, region, iVariable, i, parCal);

      cal.push_back(mCalibrators[i]->getDistribution(cal[i], parCal));
   }

   if(iType == typeUnUpdated)
      return cal.back();

   //////////////
   // Updating //
   //////////////
   for(int i = 0; i < (int) mUpdaters.size(); i++) {
      Parameters par;
      Obs recentObs;
      mData.getObs(iDate, iInit, 0, iLocation, iVariable, recentObs);
      Distribution::ptr recentDist = getDistribution(iDate, iInit, 0, iLocation, iVariable, typeUnUpdated);
      getParameters(Component::TypeUpdater, iDate, iInit, iOffset, region, iVariable, i, par);

      int Iupstream = cal.size()-1;
      assert(Iupstream >= 0);

      Distribution::ptr dist = mUpdaters[i]->getDistribution(cal[Iupstream], recentObs, recentDist, par);
      cal.push_back(dist);
   }
   return cal.back();
}

std::string ConfigurationDefault::toString() const {
   std::stringstream ss;
   ss << "      Selector:    " << mSelector->getSchemeName() << std::endl;
   ss << "      Downscaler:  " << mDownscaler->getSchemeName() << std::endl;
   ss << "      Correctors:  ";
   for(int i = 0; i < (int) mCorrectors.size(); i++) {
      ss << mCorrectors[i]->getSchemeName() << "+";
   }
   ss << std::endl;
   // TODO:
   ss << "      Uncertainty: " << mUncertainty->getSchemeName() << std::endl;
   ss << "      Calibrators: ";
   for(int i = 0; i < (int) mCalibrators.size(); i++) {
      ss << mCalibrators[i]->getSchemeName() << "+";
   }
   ss << std::endl;
   ss << "      Updaters:    ";
   for(int i = 0; i < (int) mUpdaters.size(); i++) {
      ss << mUpdaters[i]->getSchemeName();
   }
   ss << std::endl;
   ss << "      Smoother:    ";
   for(int i = 0; i < (int) mSmoothers.size(); i++) {
      ss << mSmoothers[i]->getSchemeName();
   }
   ss << std::endl;
   return ss.str();
}

void ConfigurationDefault::getDeterministic(int iDate,
      int iInit,
      float iOffset,
      const Location& iLocation,
      std::string iVariable,
      Deterministic& iDeterministic) const {
   int region = mRegion->find(iLocation);
   Ensemble ens;
   getEnsemble(iDate, iInit, iOffset, iLocation, iVariable, ens);

   /////////////
   // Average //
   /////////////
   Parameters par;
   getParameters(Component::TypeAverager, iDate, iInit, iOffset, region, iVariable, 0, par);

   float value = mAverager->average(ens, par);
   iDeterministic = Deterministic(value, iDate, iInit, iOffset, iLocation, iVariable);
}


bool ConfigurationDefault::isValid(std::string& iMessage) const {
   iMessage = "";
   // Does selector always give the same sized ensemble
   if(!mSelector->isConstSize()) {
      // Selector ensemble changes in size. This means that correctors with parameters operating on
      // each member cannot be used
      for(int i = 0; i < (int) mCorrectors.size(); i++) {
         if(mCorrectors[i]->needsConstantEnsembleSize()) {
            iMessage = "Selector gives varying ensemble sizes. One or more corrector requires a constant ensemble size to estimate parameters";
            return false;
         }
      }
      if(mUncertainty->needsConstantEnsembleSize()) {
         iMessage = "Selector gives varying ensemble sizes. One or more corrector requires a constant ensemble size to estimate parameters";
         return false;
      }
   }

   return true;
}

void ConfigurationDefault::getSelectorIndicies(int iDate,
      int iInit,
      float iOffset, 
      const Location& iLocation,
      std::string iVariable,
      std::vector<Field>& iFields) const {
   int region = mRegion->find(iLocation);

   // If selector gives the same results regardless of location or offset
   // only compute these once
   int locationId = mSelector->isLocationDependent() ? iLocation.getId() : 0;
   float offset   = mSelector->isOffsetDependent()   ? iOffset : 0;

   Key::Ensemble key(iDate, iInit, offset, locationId, iVariable);

   // Get selector indices
   if(mSelectorCache.isCached(key)) {
      const std::vector<Field>& slices = mSelectorCache.get(key);
      iFields = slices;
   }
   else {
      Parameters parSelector;
      int selectorIndex = 0; // Only one selector
      getParameters(Component::TypeSelector, iDate, iInit, offset, region, iVariable, selectorIndex, parSelector);
      mSelector->select(iDate, iInit, offset, iLocation, iVariable, parSelector, iFields);
      mSelectorCache.add(key, iFields);
   }

   // TODO: Is this the best way to interpret isOffsetDependent? Because it could also
   // mean that selector returns the same offset in the slice regardless of the search offset
   if(!mSelector->isOffsetDependent()) {
      // Adjust offset back
      for(int i = 0; i < (int) iFields.size(); i++) {
         iFields[i].setOffset(iOffset);
      }
   }

   // Check that selector doesn't violate by selecting future indices
   // Perhaps this slows it down?
   if(0 && !mSelector->allowedToCheat()) {
      for(int i = 0; i < (int) iFields.size(); i++) {
         // TODO:
         // Its ok to pick future dates if forecasts
         //if(iFields[i].getMember().getDataset()) {
         //}

         float timeDiff = Global::getTimeDiff(iDate, iInit, iInit, iFields[i].getDate(), iFields[i].getInit(), iFields[i].getOffset());
         if(timeDiff < 0) {
            std::stringstream ss;
            ss << "ConfigurationDefault: Selector picked a date/offset ("
               << iFields[i].getDate() << " " << iFields[i].getOffset() << ") "
               << "which would not be available operationally for date "
               << iDate;
            Global::logger->write(ss.str(), Logger::error);
         }
      }
   }
}

void ConfigurationDefault::updateParameters(int iDate, int iInit, const std::string& iVariable) {
   std::vector<float> offsets;
   mData.getOutputOffsets(offsets);
   std::vector<Location> obsLocations;
   mData.getObsLocations(obsLocations);

   int numValid = 0;

   // Get all observations
   std::set<float> allOffsetsSet;
   for(int o = 0; o < offsets.size(); o++) {
      float offset = fmod(offsets[o],24);
      allOffsetsSet.insert(offset);
   }
   std::vector<float> allOffsets(allOffsetsSet.begin(), allOffsetsSet.end());

   std::vector<Obs> allObs;
   for(int o = 0; o < allOffsets.size(); o++) {
      float offset = allOffsets[o];
      for(int i = 0; i < obsLocations.size(); i++) {
          Obs obs;
          mData.getObs(iDate, iInit, offset, obsLocations[i], iVariable, obs);
          allObs.push_back(obs);
          if(Global::isValid(obs.getValue()))
              numValid++;
      }
   }

   // Create a vector of all region ids
   std::set<int> regionsSet;
   for(int i = 0; i < obsLocations.size(); i++) {
      int region = mRegion->find(obsLocations[i]);
      regionsSet.insert(region);
   }
   std::vector<int> regions(regionsSet.begin(), regionsSet.end());

   bool needEnsemble = getNeedEnsemble();

   // Loop over all parameter regions
   for(int r = 0; r < regions.size(); r++) {
      int region = regions[r];
      // Loop over all output offsets
      for(int o = 0; o < offsets.size(); o++) {
         float offset = offsets[o];
         //std::cout << "Region: " << region << " offset: " << offset<< std::endl;
         float offsetObs = fmod(offsets[o],24);
         int   dateFcst = Global::getDate(iDate, 0, -(offset - offsetObs));
         // Select obs for this location/offset
         std::vector<Obs> useObs;
         for(int i = 0; i < allObs.size(); i++) {
            Obs obs = allObs[i];
            int currRegion = mRegion->find(obs.getLocation());
            if(currRegion == regions[r] && obs.getOffset() == offsetObs)
               useObs.push_back(obs);
         }
         //std::cout << "   Processing " << useObs.size() << " observations: " << std::endl;

         // Check that we are updating the right forecast
         for(int k = 0; k < useObs.size(); k++) {
            Obs obs = useObs[k];
            assert(Global::getDate(obs.getDate(), 0, obs.getOffset()) == Global::getDate(dateFcst, 0, offset) &&
                   Global::getTime(obs.getDate(), 0, obs.getOffset()) == Global::getTime(dateFcst, 0, offset));
         }
         if(useObs.size() > 0) {
            // Selector
            if(mSelector->needsTraining()) {
               Parameters par;
               getParameters(Component::TypeSelector, iDate, iInit, offset, region, iVariable, 0, par);
               // Loop over locations
               mSelector->updateParameters(dateFcst, iInit, offset, useObs, par);
               setParameters(Component::TypeSelector, iDate, iInit, offset, region, iVariable, 0, par);
            }

            if(needEnsemble) {
               // Correctors
               std::vector<Ensemble> ensembles;
               for(int n = 0; n < useObs.size(); n++) {
                  Location obsLocation = useObs[n].getLocation();
                  // Get the raw ensemble
                  Ensemble ensemble;
                  getEnsemble(dateFcst, iInit, offset, obsLocation, iVariable, ensemble, typeUnCorrected);
                  ensembles.push_back(ensemble);
               }
               for(int k = 0; k < (int) mCorrectors.size(); k++) {
                  Parameters parCorrector;
                  getParameters(Component::TypeCorrector, iDate, iInit, offset, region, iVariable, k, parCorrector);
                  Parameters parOrig = parCorrector;
                  if(mCorrectors[k]->needsTraining()) {
                     mCorrectors[k]->updateParameters(ensembles, useObs, parCorrector);
                     setParameters(Component::TypeCorrector, iDate, iInit, offset, region, iVariable, k, parCorrector);
                  }

                  // Correct ensemble for next corrector
                  for(int n = 0; n < ensembles.size(); n++) {
                     mCorrectors[k]->correct(parOrig, ensembles[n]);
                  }
               }

               // Averager
               if(mAverager->needsTraining()) {
                  Parameters parAverager;
                  getParameters(Component::TypeAverager, iDate, iInit, offset, region, iVariable, 0, parAverager);
                  mAverager->updateParameters(ensembles, useObs, parAverager);
                  setParameters(Component::TypeAverager, iDate, iInit, offset, region, iVariable, 0, parAverager);
               }

               // Uncertainty
               Parameters parUncertainty;
               getParameters(Component::TypeUncertainty, iDate, iInit, offset, region, iVariable, 0, parUncertainty);
               if(mUncertainty->needsTraining()) {
                  mUncertainty->updateParameters(ensembles, useObs, parUncertainty);
                  setParameters(Component::TypeUncertainty, iDate, iInit, offset, region, iVariable, 0, parUncertainty);
               }

               // Calibrators
               std::vector<std::vector<Distribution::ptr> > upstreams;
               upstreams.resize(1 + mCalibrators.size() + mUpdaters.size());
               upstreams[0].resize(useObs.size());
               for(int n = 0; n < useObs.size(); n++) {
                  Distribution::ptr uncD = mUncertainty->getDistribution(ensembles[n], parUncertainty);
                  upstreams[0][n] = uncD;
               }

               // Start with the uncertainty distributions. Then iteratively calibrate the previous
               // distirbution (upstream).
               int Iupstream = 0;
               for(int k = 0; k < (int) mCalibrators.size(); k++) {
                  upstreams[Iupstream+1].resize(useObs.size());
                  Parameters parCalibrator;
                  getParameters(Component::TypeCalibrator, iDate, iInit, offset, region, iVariable, k, parCalibrator);
                  if(mCalibrators[k]->needsTraining()) {
                     mCalibrators[k]->updateParameters(upstreams[Iupstream], useObs, parCalibrator);
                     setParameters(Component::TypeCalibrator, iDate, iInit, offset, region, iVariable, k, parCalibrator);
                  }
                  // Calibrate all distributions for the next calibrator
                  for(int n = 0; n < useObs.size(); n++) {
                     upstreams[Iupstream+1][n] = mCalibrators[k]->getDistribution(upstreams[Iupstream][n], parCalibrator);
                  }
                  Iupstream++;
               }
               // Updaters
               assert(mUpdaters.size() <= 1);
               std::vector<Distribution::ptr> recentDists(useObs.size());
               std::vector<Obs> recentObs;
               for(int n = 0; n < useObs.size(); n++) {
                  Distribution::ptr recentDist = getDistribution(dateFcst, iInit, 0, useObs[n].getLocation(), iVariable, typeUnUpdated);
                  recentDists[n] = recentDist;
                  // Get the recent most observation
                  Obs obs;
                  mData.getObs(dateFcst, iInit, 0, useObs[n].getLocation(), iVariable, obs);
                  recentObs.push_back(obs);
               }
               for(int k = 0; k < mUpdaters.size(); k++) {
                  upstreams[Iupstream+1].resize(useObs.size());
                  Parameters par;
                  getParameters(Component::TypeUpdater, iDate, iInit, offset, region, iVariable, k, par);
                  if(mUpdaters[k]->needsTraining()) {
                     mUpdaters[k]->updateParameters(upstreams[Iupstream], useObs, recentDists, recentObs, par);
                     setParameters(Component::TypeUpdater, iDate, iInit, offset, region, iVariable, k, par);
                  }
                  Iupstream++;
               }
            }
         }
         else {
            std::stringstream ss;
            ss << "ConfigurationDefault: No obs available to region " << region << " offset " << offset;
            Global::logger->write(ss.str(), Logger::warning);
         }
      }
   }

   if(numValid == 0) {
      std::stringstream ss;
      ss << "ConfigurationDefault: No observations available for updating for date " << iDate;
      Global::logger->write(ss.str(), Logger::warning);
   }
   else {
      std::stringstream ss;
      ss << "ConfigurationDefault: Updating using " << numValid << " valid observations for date " << iDate;
      Global::logger->write(ss.str(), Logger::status);
   }

   mParameters->write();
}

bool ConfigurationDefault::getNeedEnsemble() const {
   bool needEnsemble = false;
   for(int k = 0; k < (int) mCorrectors.size(); k++) {
      needEnsemble = mCorrectors[k]->needsTraining() ? true : needEnsemble;
   }
   needEnsemble = mAverager->needsTraining() ? true : needEnsemble;
   needEnsemble = mUncertainty->needsTraining() ? true : needEnsemble;
   for(int k = 0; k < (int) mCalibrators.size(); k++) {
      needEnsemble = mCalibrators[k]->needsTraining() ? true : needEnsemble;
   }
   return needEnsemble;
}
