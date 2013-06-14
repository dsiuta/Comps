#include "WindSpeed.h"
#include "../Data.h"

VariableWindSpeed::VariableWindSpeed() : Variable("WindSpeed") {}

float VariableWindSpeed::computeCore(const Data& iData,
      int iDate,
      int iInit,
      float iOffset,
      const Location& iLocation,
      const Member& iMember,
      Input::Type iType) const {
   float U  = iData.getValue(iDate, iInit, iOffset, iLocation, iMember, "U");
   float V  = iData.getValue(iDate, iInit, iOffset, iLocation, iMember, "V");
   if(!Global::isValid(U) || !Global::isValid(V))
      return Global::MV;
   float WindSpeed = std::sqrt(U*U + V*V);
   return WindSpeed;
}
