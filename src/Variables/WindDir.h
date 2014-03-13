#ifndef VARIABLE_WINDDIR_H
#define VARIABLE_WINDDIR_H
#include "Variable.h"

//! Using the meteorological definition of direction:
//! * i.e. where the wind is coming from
//! * From north is 0, from east is 90
class VariableWindDir : public Variable {
   public:
      VariableWindDir();
      float computeCore(const Data& iData,
                    int iDate,
                    int iInit,
                    float iOffset,
                    const Location& iLocation,
                    const Member& iMember,
                    Input::Type iType) const;
};
#endif
