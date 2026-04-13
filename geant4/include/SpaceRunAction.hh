//
#ifndef SpaceRunAction_h
#define SpaceRunAction_h 1

#include "G4Accumulable.hh"
#include "G4ThreeVector.hh"
#include "G4UserRunAction.hh"
#include "globals.hh"
// ROOT

// c++ classes
#include "G4UserRunAction.hh"
#include <cstdlib>
#include <string>

#include <mutex>

#include "SpaceAnalysis.hh"
#include "SpaceDetectorConstruction.hh"
#include "SpaceEventAction.hh"
#include "SpacePrimaryGeneratorAction.hh"
#include "SpaceRun.hh"
// Geant
#include "G4AccumulableManager.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"
#include "G4UnitsTable.hh"

// ROOT

// C++
#include "TObjString.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class G4Run;
class SpaceRun;
class G4LogicalVolume;
class SpaceEventAction;

/// Run action class

class SpaceRunAction : public G4UserRunAction {
public:
  SpaceRunAction(G4String n, G4int f, long s1, long s2, std::string sContent,
                 std::string mContent);
  ~SpaceRunAction();

  virtual void BeginOfRunAction(const G4Run *);
  virtual void EndOfRunAction(const G4Run *);

private:
  SpaceEventAction *fEventAction = nullptr;
  G4int fileNumber;
  G4String fileName;
  G4int seed1;
  G4int seed2;
  std::string specContent;
  std::string macroContent;
};

#endif
