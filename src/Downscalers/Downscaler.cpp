#include "SchemesHeader.inc"
#include "Downscaler.h"
#include "../Options.h"
#include "../Field.h"
#include "../Parameters.h"

Downscaler::Downscaler(const Options& iOptions, const Data& iData) : Processor(iOptions, iData) {}
#include "Schemes.inc"
float Downscaler::downscale(const Input* iInput, 
      int iDate, int iInit, float iOffset,
      const Location& iLocation,
      const Member& iMember,
      std::string iVariable) const {
   Field slice(iDate, iInit, iOffset, iMember);
   float value = downscale(slice, iVariable, iLocation, Parameters());
   return value;
}
