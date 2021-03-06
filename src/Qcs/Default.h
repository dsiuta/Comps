#ifndef QC_DEFAULT_H
#define QC_DEFAULT_H
#include "Qc.h"

//! Checks that the value is between the min and max defined by the variable
class QcDefault : public Qc {
   public:
      QcDefault(const Options& iOptions, const Data& iData);
   protected:
      bool checkCore(const Value& iValue) const;
};
#endif

