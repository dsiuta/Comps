#ifndef INPUT_H
#define INPUT_H
#include "../Global.h"
#include "../Component.h"
#include "../Cache.h"
#include "../Key.h"

#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

class Variable;
class Global;
class Options;
class Location;
class Member;
class Logger;
class Obs;
class Ensemble;

//! Interface to retrieving forecast/obs data
class Input : public Component {
   public:
      static        Input* getScheme(const Options& iOptions, const Data& iData);
      static        Input* getScheme(const std::string& iTag, const Data& iData);
      virtual       ~Input();

      enum Type {typeObservation = 0, typeForecast = 10, typeBest = 20, typeUnspecified = 30};
      static std::string getTypeDescription(Input::Type iType);
      ////////////////////////
      // Dataset dimensions //
      ////////////////////////
      std::vector<Member>      getMembers() const;
      std::vector<Location>    getLocations() const;
      std::vector<float>       getOffsets() const;
      std::vector<std::string> getVariables() const;
      void                     getDates(std::vector<int>& iDates) const;
      int                      getNumLocations() const;
      int                      getNumOffsets() const;
      int                      getNumVariables() const;
      int                      getNumMembers() const;

      ////////////
      // Values //
      ////////////
      //! iCalibrate: Should the value be calibrated before being returned?
      float         getValue(int rDate, int rInit, float iOffset, int rLocationNum, int rMemberId, std::string rVariable, bool iCalibrate=true) const;
      void          getValues(int rDate, int rInit, float iOffset, int rLocationId, std::string rVariable, std::vector<float>& iValues) const;
      void          getValues(int rDate, int rInit, float iOffset, int rLocationId, std::string rVariable, Ensemble& iEnsemble) const;
      void          getSurroundingLocations(const Location& iTarget, std::vector<Location>& iLocations, int iNumPoints=1) const;
      //! In metres
      void          getSurroundingLocationsByRadius(const Location& rTarget, std::vector<Location>& rLocations, float iRadius) const;

      std::string   getName() const;
      Type          getType() const;
      //! Get description of iType
      static        std::string getInputDirectory();
      //! Checks if variable is available
      bool          hasVariable(std::string iVariable) const;
      bool          hasOffset(float iOffset) const;
      int           getOffsetIndex(float iOffset) const;
      int           getLocationIndex(float iLocationId) const;
      int           getNearestOffsetIndex(float iOffset) const;
      std::string   getSampleFilename() const;
      bool          needsTraining() const {return false;};

      ///////////////////////
      // Write this format //
      ///////////////////////
      //! Writes data of this type, using data from iData and dimensions from iDimensions
      void          write(const Input& iData, const Input& iDimensions, int iDate) const;
      //! Writes data of this type, using data and dimensinos from iInput
      void          write(const Input& iInput, int iDate) const;
      //! Get the filename of the sample file
   protected:
      Input(const Options& iOptions, const Data& iData);
      ////////////////////////////
      // Subclass can implement //
      ////////////////////////////
      virtual float getValueCore(const Key::Input& key) const = 0;
      virtual void  getMembersCore(std::vector<Member>& iMembers) const;
      virtual void  getLocationsCore(std::vector<Location>& rLocations) const;
      virtual void  getOffsetsCore(std::vector<float>& iOffsets) const;
      //! Retrives all available dates. No need to sort or cache.
      virtual bool  getDatesCore(std::vector<int>& iDates) const;
      //! Set the caching booleans to optimial values for this dataset
      virtual void  optimizeCacheOptions();
      virtual void  writeCore(const Input& iData, const Input& iDimensions, int iDate) const;
      virtual std::string getDefaultFileExtension() const;
      virtual std::string getSampleFilenameCore() const;


      //! Loads offsets/members/etc. Must be called by inheriting classes.
      void          init();

      std::string   getFileExtension() const;
      //! Base directory where namelists are located
      std::string   getDirectory() const;
      //! Directory where data is located
      std::string   getDataDirectory() const;
      std::string   getNamelistFilename(std::string type) const;

      // Variables
      //std::string   getLocalVariableName(std::string iVariable) const;
      //std::string   getLocalVariableName(int iVariableId) const;
      //std::string   getVariableName(std::string iLocalVariable) const;
      //std::string   getVariableName(int iVariableId) const;
      //int           getVariableIdFromVariable(std::string iVariable) const;
      //int           getVariableIdFromLocalVariable(std::string iLocalVariable) const;

      // Returns true if found
      bool getLocalVariableName(std::string iVariable, std::string& iLocalVariable) const;
      bool getLocalVariableName(int iVariableId, std::string& iLocalVariable) const;
      bool getVariableName(std::string iLocalVariable, std::string& iVariable) const;
      bool getVariableName(int iVariableId, std::string& iVariable) const;
      bool getVariableIdFromVariable(std::string iVariable, int& iVariableId) const;
      bool getVariableIdFromLocalVariable(std::string iLocalVariable, int& iVariableId) const;

     // Caching
      void addToCache(const Key::Input& iKey, float iValue) const;
      bool mCacheOtherOffsets;
      bool mCacheOtherLocations;
      bool mCacheOtherMembers;
      bool mCacheOtherVariables;

      //! Tells the user that the caching options are invalid for this dataset
      void notifyInvalidCacheOptions() const;

   private:
      std::vector<int> mAllowLocations;
      bool mAllowTimeInterpolation;

      float getVariableOffset(const std::string& iVariable) const;
      float getVariableScale(const std::string& iVariable) const;

      bool         mHasInit;
      mutable bool mHasWarnedCacheMiss; //> Only warn about cache misses once

      mutable Cache<Location, std::vector<int> > mCacheSurroundingLocations; // location Id, closest Ids
      mutable Cache<Location, std::vector<int> > mCacheNearestLocation; // location Id, closest Id
      mutable std::vector<float>* mLastCachedVector;
      mutable Key::Input          mLastCachedKey;
      mutable Cache<Key::Input, std::vector<float> > mCache;
      int getCacheIndex(const Key::Input& iKey) const;
      int getCacheVectorSize() const;
      bool mOptimizeCache;

      // TODO: Not implemented. Allow filenames to have junk at the beginning before the date.
      int mFilenameDateStartIndex;
      std::string mFileExtension;
      Type mType;
      std::string mName;
      mutable bool mIsDatesCached;
      mutable std::vector<int> mDates;

      std::string   mDataDirectory;
      std::string   mDirectory;
      std::string   mFolder;

      // Dataset dimensions
      // Never access these, use the accessor functions
      mutable std::vector<Location> mLocations;
      mutable std::map<int, int> mLocationMap; // Id, index
      mutable std::vector<Member> mMembers;
      mutable std::vector<float> mOffsets;
      mutable std::map<float, int> mOffsetMap; // Offset, index
      mutable std::vector<std::string>   mVariables;

      // Variable conversions
      mutable std::map<std::string,std::string> mVariable2LocalVariable; // Comps name, local name
      mutable std::map<std::string,std::string> mLocalVariable2Variable; // Local name, Comps name
      mutable std::map<std::string, int> mVariable2Id;
      mutable std::vector<std::string> mId2Variable;
      mutable std::map<std::string, int> mLocalVariable2Id;
      mutable std::vector<std::string> mId2LocalVariable;

      mutable std::map<std::string,float> mVariableScale;
      mutable std::map<std::string,float> mVariableOffset;

      mutable int mNumLocations;
      mutable int mNumOffsets;
      mutable int mNumMembers;
      mutable int mNumVariables;
};
#endif
