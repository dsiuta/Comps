#include "NearestElevation.h"
#include "../Field.h"
#include "../Data.h"
#include "../Location.h"
DownscalerNearestElevation::DownscalerNearestElevation(const Options& iOptions, const Data& iData) : Downscaler(iOptions, iData),
      mMinElevDiff(0) {
   //! Search for nearest elevation-neighbour within this radius (in m)
   iOptions.getRequiredValue("searchRadius", mSearchRadius);
   //! Only use the nearest neighbour if its elevation difference is less than this amount (in m)
   iOptions.getValue("minElevDiff", mMinElevDiff);
}

float DownscalerNearestElevation::downscale(const Field& iField,
      const std::string& iVariable,
      const Location& iLocation,
      const Parameters& iParameters) const {

   std::string sliceDataset = iField.getMember().getDataset();
   Input* input = mData.getInput(sliceDataset); //mData.getInput(iField.getMember().getDataset());

   // Determine which location to use
   Location useLocation;
   if(iLocation.getDataset() == sliceDataset) {
      // This should not happen, because there is no point downscaling a grid
      // to itself using this method
      useLocation = iLocation;
   }
   else {
      useLocation = getBestLocation(input, iLocation);
   }

   float value = mData.getValue(iField.getDate(), iField.getInit(), iField.getOffset(),
         useLocation, iField.getMember(), iVariable);
   return value;
}

Location DownscalerNearestElevation::getBestLocation(Input* iInput, const Location& iLocation) const {
   // Get the nearest neighbour, in case we need it
   std::vector<Location> temp;
   iInput->getSurroundingLocations(iLocation, temp);
   Location nearestNeighbour = temp[0];

   float thisElev = iLocation.getElev();
   // Cannot downscale if we do not have elevation information
   if(!Global::isValid(thisElev)) {
      return nearestNeighbour;
   }

   // Check if the nearest neighbour is close enough in elevation
   float elevNearestNeighbour = nearestNeighbour.getElev();
   if(!Global::isValid(elevNearestNeighbour) || fabs(elevNearestNeighbour - thisElev) < mMinElevDiff) {
      // The true nearest neighbour is close enough in elevation to the desired point.
      return nearestNeighbour;
   }

   // Check that there are locations within the search radius
   std::vector<Location> locations;
   iInput->getSurroundingLocationsByRadius(iLocation, locations, mSearchRadius);
   if(locations.size() == 0) {
      return nearestNeighbour;
   }

   // Find location with closest elevation
   int nearestIndex = Global::MV;
   float diff = Global::MV;
   for(int i = 0; i < locations.size(); i++) {
      float elev = locations[i].getElev();
      if(Global::isValid(elev)) {
         float currDiff = fabs(elev - thisElev);
         if(diff == Global::MV || currDiff < diff) {
            nearestIndex = i;
            diff = currDiff;
         }
      }
   }
   // Check that we found a point with an elevation
   if(!Global::isValid(nearestIndex)) {
      return nearestNeighbour;
   }

   // Use nearest elevation
   return locations[nearestIndex];
}
