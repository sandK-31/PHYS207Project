#ifndef SpaceSteppingAction_h
#define SpaceSteppingAction_h 1

#include "G4SteppingManager.hh"
#include "G4UserSteppingAction.hh"
#include "SpaceActionManager.hh"
#include "SpaceDetectorConstruction.hh"
#include "SpaceEventAction.hh"
#include "SpacePrimaryGeneratorAction.hh"
#include "globals.hh"
#include <array>
#include <cmath>
#include <iostream>
#include <set>
#include <vector>

#include "G4RandomDirection.hh"
#include "Randomize.hh"

#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4UnitsTable.hh"

#include "G4AccumulableManager.hh"
#include "G4DynamicParticle.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4LogicalVolume.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpticalPhoton.hh"
#include "G4ProcessManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4VSolid.hh"
#include "G4ios.hh"
#include "SpaceActionManager.hh"
#include "SpaceAnalysis.hh"
#include "SpaceRunAction.hh"
#include <G4String.hh>
#include <G4VProcess.hh>
#include <G4VSteppingVerbose.hh>
#include <iostream>
#include <string>
#include <typeinfo>

#include <unordered_map>

class EventAction;

class G4LogicalVolume;

/// Stepping action.hh
///

class SpaceSteppingAction : public G4UserSteppingAction {
public:
  // Define a struct to hold our two lengths
  struct OpticalLengths {
    G4double absorptionLength;
    G4double scatteringLength;
  };
  SpaceSteppingAction(SpaceActionManager *manager); // constructor
  virtual ~SpaceSteppingAction();

  // method from the base class
  virtual void UserSteppingAction(const G4Step *);
  int count = 0;
  int polCount = 0;

private:
  OpticalLengths GetOpticalLengthsAtPosition(G4ThreeVector position);

  SpaceActionManager *fActionManager;
  std::set<G4int> eventIDList;

  struct stepInfo {
    int stepID;
    G4ThreeVector position;
    G4String volume;
    G4String process;
  };

  std::vector<stepInfo> stepData;

  std::unordered_map<int, int> rayleighCount;
  std::unordered_map<int, int> mieCount;
};

#endif
