#include "Averager.h"
#include "Weighted.h"

float AveragerWeighted::errorOffset = 0.1;

AveragerWeighted::AveragerWeighted(const Options& iOptions, const Data& iData) : Averager(iOptions, iData),
      mUseSelectorSkill(false) {
   //! Use the skill of the member determined by the Selection component. Otherwise, compute the
   //! skill adaptively.
   iOptions.getValue("useSelectorSkill", mUseSelectorSkill);
   Component::underDevelopment();
}

float AveragerWeighted::average(const Ensemble& iValues, const Parameters& iParameters) const {
   float mean = 0;
   float total = 0;
   std::vector<float> skills;
   if(mUseSelectorSkill) { 
      skills = iValues.getSkills();
   }
   else {
      skills = iParameters.getAllParameters();
   }
   for(int i = 0; i < (int) iValues.size(); i++) {
      if(Global::isValid(iValues[i])) {
         float skill;
         if(!Global::isValid(skills[i])) {
            skill = 0;
         }
         else {
            skill = 1/(skills[i] + errorOffset);
         }
         mean += iValues[i] * skill;
         total += skill;
      }
   }
   if(total != 0) {
      mean = mean / total;
   }
   else  {
      mean = Global::getMoment(iValues.getValues(),1);
   }
   return mean;
}
void AveragerWeighted::resize(Parameters& iParameters, int iSize) const {

}
void AveragerWeighted::getDefaultParameters(Parameters& iParameters) const {
   if(!mUseSelectorSkill) {
      iParameters[0] = 1;
   }
}
void AveragerWeighted::updateParameters(const std::vector<Ensemble>& iValues,
      const std::vector<Obs>& iObs,
      Parameters& iParameters) const {
   if(!mUseSelectorSkill) {
      /*
      if(iValues.size() != iParameters.size()) {
         resize(iParameters, iValues.size());
      }
      if(iObs.size() > 0) {
         float obs = iObs[0].getValue();
         if(!Global::isMissing(obs)) {
            for(int i = 0; i < (int) iValues.size(); i++) {
               if(!Global::isMissing(iValues[i])) {
                  float error = fabs(obs - iValues[i]);
                  iParameters[i] = Processor::combine(iParameters[i], error);
               }
            }
         }
      }
      */
   }
}
bool AveragerWeighted::needsTraining() const {
   return !mUseSelectorSkill;
}
int AveragerWeighted::numParametersCore() const {
   return !mUseSelectorSkill;
}
