#ifndef PARAMETER_IO_H
#define PARAMETER_IO_H
#include "../Processor.h"
#include "../Location.h"
#include "../Parameters.h"
#include "../Key.h"
#include "../Cache.h"

class Data;
class Pooler;
class ParameterIo : public Component {
   public:
      ParameterIo(const Options& iOptions, std::string iConfiguration, const Data& iData);
      static ParameterIo* getScheme(const Options& iOptions, std::string iConfiguration, const Data& iData);
      static ParameterIo* getScheme(const std::string& iTag, std::string iConfiguration, const Data& iData);
      ~ParameterIo();

      //! Returns true if parameters found, otherwise false
      bool read(Processor::Type iType,
                        int iDate,
                        int iInit,
                        float iOffset,
                        int iPoolId,
                        const std::string iVariable,
                        int iIndex,
                        Parameters& iParameters) const;
      void add(Processor::Type iType,
                         int iDate,
                         int iInit,
                         float iOffset,
                         int iPoolId,
                         const std::string iVariable,
                         int iIndex,
                         Parameters iParameters);
      //! Write all queued parameters to file. Clear parameters afterwards.
      void write();

   protected:
      virtual void writeCore(const std::map<Key::Par,Parameters>& iParametersWrite) = 0;
      virtual bool readCore(const Key::Par& iKey, Parameters& iParameters) const = 0;

      static const std::string mBaseOutputDirectory;
      std::string mRunDirectory;
      mutable std::map<Processor::Type, int> mComponentMap; // Type, index

      std::vector<Processor::Type> mComponents;

      // Cannot be private, because some schemes pull extra data than whats being asked for
      // and need to add to cache
      mutable Cache<Key::Par,Parameters> mCache;
      std::string mConfiguration;
      const Data& mData;
   private:
      mutable std::map<Key::Par,Parameters> mParametersWrite;
};
#endif
