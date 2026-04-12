#ifndef SpaceOpticalPropertiesMessenger_h
#define SpaceOpticalPropertiesMessenger_h 1

#include "G4GenericMessenger.hh"
#include "G4Material.hh"
#include "SpaceMaterialsDefinition.hh"
#include "globals.hh"
#include <iostream>
#include <string>
#include <vector>

class SpaceOpticalPropertiesMessenger {
public:
  SpaceOpticalPropertiesMessenger();
  ~SpaceOpticalPropertiesMessenger();
  inline G4double GetIntralipidRayleighScatlength() {
    return intralipidRayleighScatlength;
  };
  inline G4double GetIntralipidAbslength() { return intralipidAbslength; };
  inline G4double GetIntralipidRindex() { return intralipidRindex; };
  inline G4double GetIntralipidintralipidMieScatlength() {
    return intralipidMieScatlength;
  };
  inline G4double GetIntralipidintralipidMieForwardG() {
    return intralipidMieForwardG;
  };
  inline G4double GetIntralipidintralipidMieBackwardG() {
    return intralipidMieBackwardG;
  };
  inline G4double GetIntralipidintralipidMieFBRatioG() {
    return intralipidMieFBRatioG;
  };

  void OpticsUpdate();

private:
  G4double intralipidRayleighScatlength;
  G4double intralipidAbslength;
  G4double intralipidRindex;
  G4double intralipidMieScatlength;
  G4double intralipidMieForwardG;
  G4double intralipidMieBackwardG;
  G4double intralipidMieFBRatioG;

  G4double intralipidRayleighScatlengthHolder;
  G4double intralipidAbslengthHolder;
  G4double intralipidRindexHolder;
  G4double intralipidMieScatlengthHolder;
  G4double intralipidMieForwardGHolder;
  G4double intralipidMieBackwardGHolder;
  G4double intralipidMieFBRatioGHolder;

  G4GenericMessenger *fMessenger;
};

#endif
