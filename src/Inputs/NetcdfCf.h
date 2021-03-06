#ifndef INPUT_NETCDF_CF_H
#define INPUT_NETCDF_CF_H
#include "Input.h"
#include "../Cache.h"
#include "../Key.h"
#include <netcdf.hh>

//! Datasets conforming to the CF standard
//! Recognizes the following attributes (assigned to a variable):
//!   scale_factor, add_offset, _FillValue
//! Lat, lon, and elevation variables may have different dimensions than the
//! variables in the file, however they must have the X, and Y dimensions in the
//! same order.
class InputNetcdfCf : public Input {
   public:
      InputNetcdfCf(const Options& iOptions);
   private:
      void   getLocationsCore(std::vector<Location>& iLocations) const;
      void   getOffsetsCore(std::vector<float>& iOffsets) const;
      void   getMembersCore(std::vector<Member>& iMembers) const;
      std::string getDefaultFileExtension() const;

      float  getValueCore(const Key::Input& iKey) const;
      void   optimizeCacheOptions();
      // Variable names
      std::string mLatVar;
      std::string mLonVar;
      std::string mLandUseVar;
      std::string mLandFractionVar;
      std::string mTimeVar;
      std::string mTimeRef;
      std::string mElevVar;
      float       mElevScale;
      bool mComputeGradient;
      // Dimension names
      std::string mTimeDim;
      std::string mEnsDim;
      std::vector<std::string> mHorizDims;
      std::vector<std::string> mVertDims;
      //! Throws an error if the dimension does not exist
      NcDim* getDim(NcFile* iFile, std::string iName) const;
      //! Throws an error if the variable does not exist
      NcVar* getRequiredVar(NcFile* iFile, std::string iName) const;
      //! Returns NULL if the variable does not exist
      NcVar* getVar(NcFile* iFile, std::string iName) const;
      float mTimeDivisor;
      //! Convert linear index 'i' to vector 'iInidices'. 'iCount' specifies the size of the data
      //! Using row-major ordering (last index varies fastest)
      void getIndices(int i, const std::vector<int>& iCount, std::vector<int>& iIndices) const;

      int getIndex(int i, int j, int jSize) const;
      long* getCount(const NcVar* iVar, std::vector<NcDim*> iHorizDims) const;

};
#endif
