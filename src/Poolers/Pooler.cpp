#include "Pooler.h"
#include "SchemesHeader.inc"
#include "../Global.h"
#include "../Options.h"
#include "../Data.h"
#include "../Location.h"

Pooler::Pooler(const Options& iOptions, const Data& iData) : Component(iOptions), mData(iData),
      mWindowLength(0) {
   //! Each bin should be this many hours wide
   iOptions.getValue("windowLength", mWindowLength);
   mLocationCache.setName("Pooler");
   std::string tag;
   iOptions.getValue("tag", tag);
}
#include "Schemes.inc"

int Pooler::find(const Location& iLocation) const {
   if(mData.getObsInput() == NULL) {
      return Global::MV;
   }
   if(mLocationCache.isCached(iLocation)) {
      // Use cached index
      int i = mLocationCache.get(iLocation);
      return i;
   }
   else {
      // Compute and cache
      int i = findCore(iLocation);
      mLocationCache.add(iLocation, i);
      return i;
   }
}
float Pooler::find(float iOffset) const {
   if(mData.getObsInput() == NULL) {
      return Global::MV;
   }
   if(mWindowLength == 0)
      return iOffset;
   else
      // Round to nearest bin edge
      return floor(iOffset / mWindowLength) * mWindowLength;
   // Opportunity to cache index
}
