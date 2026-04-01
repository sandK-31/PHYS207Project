#ifndef LaserSteppingAction_h
#define LaserSteppingAction_h 1

#include "G4SteppingManager.hh"
#include "G4UserSteppingAction.hh"
#include "LaserActionManager.hh"
#include "LaserDetectorConstruction.hh"
#include "LaserEventAction.hh"
#include "LaserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include <array>
#include <iostream>
#include <set>
#include <vector>

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
#include "LaserActionManager.hh"
#include "LaserAnalysis.hh"
#include "LaserRunAction.hh"
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

class LaserSteppingAction : public G4UserSteppingAction {
public:
  LaserSteppingAction(LaserActionManager *manager); // constructor
  virtual ~LaserSteppingAction();

  // method from the base class
  virtual void UserSteppingAction(const G4Step *);
  int count = 0;
  int polCount = 0;

private:
  LaserActionManager *fActionManager;
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
