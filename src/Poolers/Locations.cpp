#include "Locations.h"
#include "../Global.h"
#include "../Options.h"
#include "../Location.h"
#include "../Inputs/Input.h"
#include "../Data.h"
#include "../Location.h"

PoolerLocations::PoolerLocations(const Options& iOptions, const Data& iData) :
      Pooler(iOptions, iData),
      mDataset("") {
   if(iOptions.hasValue("lats") || iOptions.hasValue("lons")) {
      std::vector<float> lats;
      std::vector<float> lons;
      //! Place parameters at latitude/longitude pairs
      iOptions.getValues("lats", lats);
      iOptions.getValues("lons", lons);
      if(lats.size() != lons.size()) {
         std::stringstream ss;
         std::string tag;
         iOptions.getValue("tag", tag);
         ss << "Pooler '" << tag << "' must have the same number of latitudes and longitudes specified";
         Global::logger->write(ss.str(), Logger::error);
      }
      for(int i = 0; i < lats.size(); i++) {
         int id = i;
         float lat = lats[i];
         float lon = lons[i];
         std::string dataset = "";
         Location location(dataset, id, lat, lon);
         mLocations.push_back(location);
      }
   }
   //! Use one parameter set for each location in this dataset
   else if(iOptions.getValue("dataset", mDataset)) {
      Input* input = iData.getInput(mDataset);
      assert(input != NULL);
      if(input == NULL) {
         std::stringstream ss;
         ss << "PoolerLocations: Dataset " << mDataset << " does not exist";
         Global::logger->write(ss.str(), Logger::error);
      }
      mLocations = input->getLocations();
   }
   else {
      Input* input = iData.getObsInput();
      if(input != NULL) {
         mDataset = input->getName();
         mLocations = input->getLocations();
      }
   }
   if(mLocations.size() == 0) {
      std::stringstream ss;
      ss << "PoolerLocations: No locations available. Parameters can never be estimated, since observations"
         << " are never pooled";
      Global::logger->write(ss.str(), Logger::warning);
   }
   //Pooler::setSize(mLocations.size());
   iOptions.check();
}

int PoolerLocations::findCore(const Location& iLocation) const {
   if(iLocation.getDataset() == mDataset)
      return iLocation.getId();

   double minDist = Global::INF;
   int minI = Global::MV;
   for(int i = 0; i < mLocations.size(); i++) {
      double dist = mLocations[i].getDistance(iLocation);
      if(dist < minDist) {
         minDist = dist;
         minI = i;
      }
   }
   if(!Global::isValid(minI))
      return Global::MV;
   else {
      assert(minI < mLocations.size());
      return mLocations[minI].getId();
   }
}
