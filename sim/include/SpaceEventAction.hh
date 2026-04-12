#ifndef SpaceEventAction_h
#define SpaceEventAction_h 1
////////////////////////
// My
#include "G4UserEventAction.hh"
#include "SpaceActionManager.hh"
#include "SpaceRunAction.hh"
#include "globals.hh"
// C++
#include "G4AccumulableManager.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4WorkerThread.hh"
#include "SpaceAnalysis.hh"
#include "SpaceRun.hh"
#include <array>
#include <numeric>
#include <set>
#include <vector>

// EventAction.hh

class SpaceEventAction : public G4UserEventAction {
private:
  // data members
  G4int fdetHCID;
  SpaceActionManager *fActionManager; // Add this line

public:
  SpaceEventAction(SpaceActionManager *manager); // constructor
  virtual ~SpaceEventAction();

  virtual void BeginOfEventAction(const G4Event *event);
  virtual void EndOfEventAction(const G4Event *event);

  // my methods
};

#endif
