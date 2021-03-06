#include "Elevation.h"
#include "../Field.h"
#include "../Data.h"
#include "../Location.h"
#include "../Variables/Variable.h"
DownscalerElevation::DownscalerElevation(const Options& iOptions) : Downscaler(iOptions),
      mNumPoints(1),
      mLapseRate(6.5),
      mComputeLapseRate(false),
      mShowLapseRate(false),
      mType(typeTemperature),
      mMaxLapseRate(Global::MV) {
   std::string neighbourhoodTag;
   //! Which neighbourhood scheme should be used to define the neighbourhood? Defaults to 1 nearest
   //! neighbour.
   if(iOptions.getValue("neighbourhood", neighbourhoodTag)) {
      mNeighbourhood = Neighbourhood::getScheme(neighbourhoodTag);
   }
   else {
      // Default to nearest neighbour
      mNeighbourhood = Neighbourhood::getScheme(Options("class=NeighbourhoodNearest num=1"));
   }

   //! Use this lapse rate (degrees/km). Positive means decreasing temperature with height.
   iOptions.getValue("lapseRate", mLapseRate);
   //! Should the lapse rate be computed from neighbouring points?
   iOptions.getValue("computeLapseRate", mComputeLapseRate);
   //! Output the lapse rate instead of temperature (only used for debugging)
   iOptions.getValue("showLapseRate", mShowLapseRate);

   //! What kind of elevation correction?
   //! * temperature (using lapse rate)
   //! * pressure
   std::string type = "temperature";
   iOptions.getValue("type", type);
   if(type == "temperature")
      mType = typeTemperature;
   else if(type == "pressure")
      mType = typePressure;
   else if(type == "precipitation")
      mType = typePressure;
   else if(type == "windSpeed")
      mType = typeWindSpeed;
   else {
      std::stringstream ss;
      ss << "type must be one of: 'temperature', 'pressure', 'precipitation', 'windSpeed";
      Global::logger->write(ss.str(), Logger::error);
   }

   iOptions.getValue("maxLapseRate", mMaxLapseRate);
   iOptions.check();
}

float DownscalerElevation::downscale(const Input* iInput,
      int iDate, int iInit, float iOffset,
      const Location& iLocation,
      int iMemberId,
      const std::string& iVariable) const {

   std::vector<Location> locations = mNeighbourhood->select(iInput, iLocation);
   std::vector<Location> nns;
   iInput->getSurroundingLocations(iLocation, nns,1);
   Location neighbour = nns[0];

   float value = Global::MV;
   float desiredElevation = iLocation.getElev();
   float nearestT    = iInput->getValue(iDate, iInit, iOffset, neighbour.getId(), iMemberId, iVariable);
   float nearestElev = neighbour.getElev();

   if(!Global::isValid(desiredElevation))
      return Global::MV;

   if(mType == typePressure) {
      int counter = 0;
      float total = 0;
      for(int i = 0; i < locations.size(); i++) {
         float currElevation = locations[i].getElev();
         float currValue     = iInput->getValue(iDate, iInit, iOffset, locations[i].getId(), iMemberId, iVariable);
         if(Global::isValid(currValue)) {
            counter++;
            float adjustedValue = currValue * exp(-1.21e-4 * (desiredElevation - currElevation));
            total += adjustedValue;
         }
      }
      if(counter > 0)
         value = total / counter;
   }
   else {
      // Find temperature by using linear regression on the elevation and temperatures
      // of neighbouring points
      if(mComputeLapseRate) {
         float meanXY  = 0; // elev*T
         float meanX   = 0; // elev
         float meanY   = 0; // T
         float meanXX  = 0; // elev*elev
         int   counter = 0;
         for(int i = 0; i < locations.size(); i++) {
            float x = locations[i].getElev();
            float y = iInput->getValue(iDate, iInit, iOffset, locations[i].getId(), iMemberId, iVariable);
            if(Global::isValid(x) && Global::isValid(y)) {
               meanXY += x*y;
               meanX  += x;
               meanY  += y;
               meanXX += x*x;
               counter++;
            }
         }
         if(counter == 0) {
            // No valid elevations or forecasts
            value = Global::MV;
         }
         else {
            float lapseRate; // In units / km
            // If all points are at the same elevation, use the standard lapse rate
            if(fabs(meanXX - meanX*meanX) < 1) {
               lapseRate = mLapseRate;
            }
            else {
               // Estimate lapse rate
               meanXY /= counter;
               meanX  /= counter;
               meanY  /= counter;
               meanXX /= counter;
               lapseRate = -(meanXY - meanX*meanY)/(meanXX - meanX*meanX)*1000;
               // Check against unreasonable lapse rates
               if(lapseRate < -mMaxLapseRate)
                  lapseRate = -mMaxLapseRate; // extreme unstable lapse rates
               if(lapseRate > mMaxLapseRate)
                  lapseRate = mMaxLapseRate; // extreme stable lapse rates
            }

            if(mShowLapseRate) {
               // For debugging purposes only
               value = lapseRate;
            }
            else {
               value = moveParcel(nearestT, nearestElev, desiredElevation, lapseRate);
            }
            assert(Global::isValid(value));
         }
      }
      // Bring each neighbouring point up/down to desired location and take the average
      else {
         if(mShowLapseRate) {
            // For debugging purposes only
            value = mLapseRate;
         }
         else {
            float total   = 0;
            int   counter = 0;
            for(int i = 0; i < locations.size(); i++) {
               float currElevation = locations[i].getElev();
               float currValue = iInput->getValue(iDate, iInit, iOffset, locations[i].getId(), iMemberId, iVariable);
               float adjustedValue = moveParcel(currValue, currElevation, desiredElevation, mLapseRate);
               if(Global::isValid(adjustedValue)) {
                  total += adjustedValue;
                  counter++;
               }
            }
            if(counter > 0)
               value = total / counter;
         }
      }
   }
   return value;
}
float DownscalerElevation::moveParcel(float iTemperatureStart, float iZStart, float iZEnd, float iLapseRate)  const {
   float temperatureEnd = Global::MV;
   if(Global::isValid(iZStart) && Global::isValid(iZEnd) && Global::isValid(iTemperatureStart) && Global::isValid(iLapseRate)) {
      temperatureEnd = iTemperatureStart - iLapseRate/1000 * (iZEnd - iZStart);
   }
   return temperatureEnd;
}
