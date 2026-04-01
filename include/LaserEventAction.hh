#ifndef LaserEventAction_h
#define LaserEventAction_h 1
////////////////////////
// My
#include "G4UserEventAction.hh"
#include "LaserActionManager.hh"
#include "LaserRunAction.hh"
#include "globals.hh"
// C++
#include "G4AccumulableManager.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4WorkerThread.hh"
#include "LaserAnalysis.hh"
#include "LaserRun.hh"
#include <array>
#include <numeric>
#include <set>
#include <vector>

// EventAction.hh

class LaserEventAction : public G4UserEventAction {
private:
  // data members
  G4int fdetHCID;
  LaserActionManager *fActionManager; // Add this line

public:
  LaserEventAction(LaserActionManager *manager); // constructor
  virtual ~LaserEventAction();

  virtual void BeginOfEventAction(const G4Event *event);
  virtual void EndOfEventAction(const G4Event *event);

  // my methods
};

#endif
