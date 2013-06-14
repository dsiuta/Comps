#include "Default.h"
#include "../Location.h"
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include "../Configurations/Configuration.h"

LoggerDefault::LoggerDefault(Logger::Level rMaxLevel) : Logger(rMaxLevel), mStream(std::cout) {
}
LoggerDefault::LoggerDefault(std::ofstream& rStream, Logger::Level rMaxLevel) : Logger(rMaxLevel), mStream(rStream) {
}
LoggerDefault::~LoggerDefault() {
   //mStream.close();
}

void LoggerDefault::write(const std::string& rMessage, Logger::Level rLevel, Logger::Type iType) {
   if(rLevel <= mMaxLevel) {
      time_t timer;
      time(&timer);
      char buffer[80];

      std::string startCol = getStartColor(rLevel);
      std::string endCol   = getEndColor(rLevel);
      strftime(buffer, 80, "%c", gmtime(&timer));
      mStream << startCol << buffer << " (" << rLevel << ") " << rMessage << endCol << std::endl;
      if(rLevel == LoggerDefault::error) {
         mStream << "Stack trace:" << std::endl;
         void *array[10];
         size_t size = backtrace(array, 10);

         // print out all the frames to stderr
         backtrace_symbols_fd(array, size, 2);
         abort();
      }
   }
   for(int i = 0; i < (int) mCacheNames.size(); i++) {
      //std::cout << "Cache size: " << mCacheNames[i] << " = " << *mCacheSizes[i] << std::endl;
   }

}
std::string LoggerDefault::getStartColor(Logger::Level iLevel) const {
   std::map<Level,int>::const_iterator it = mColourMap.find(iLevel);
   if(it == mColourMap.end()) {
      return "";
   }
   else {
      std::stringstream ss;
      ss << "\033[" << it->second << "m";
      return ss.str();
   }
}
std::string LoggerDefault::getEndColor(Logger::Level iLevel) const {
   std::map<Level,int>::const_iterator it = mColourMap.find(iLevel);
   if(it == mColourMap.end()) {
      return "";
   }
   else {
      return "\033[0m";
   }
}

void LoggerDefault::setLocationCore() {
   mStream << "Location: " << mCurrLocation->getId() << " (" << mCurrLocationIndex << "/" << mNumLocations << ")" << std::endl;
}
void LoggerDefault::setDateCore() {
   mStream << "Date: " << mCurrDate << " (" << mCurrDateIndex << "/" << mNumDates << ")" << std::endl;
}

void LoggerDefault::setConfigurationsCore() {
   mStream << "Configuration: " << mConfigurations[mCurrConfiguration]->getName() << std::endl;
   std::string options = mConfigurations[mCurrConfiguration]->toString();
   mStream << options << std::endl;

}
