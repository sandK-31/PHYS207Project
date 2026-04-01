#ifndef LaserDetectorSD_h
#define LaserDetectorSD_h 1

#include "G4VSensitiveDetector.hh"
// #include "TotalHit.hh"
#include "LaserAnalysis.hh"
#include "LaserEventAction.hh"
#include "LaserRun.hh"
#include "LaserRunAction.hh"
// Geant
#include "G4AccumulableManager.hh"
#include "G4HadronicProcess.hh"
#include "G4ParticleTypes.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4ios.hh"
// ROOT
#include <algorithm>
#include <vector>

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

/// Hodoscope sensitive detector

class LaserDetectorSD : public G4VSensitiveDetector {
public:
  LaserDetectorSD(G4String name);
  virtual ~LaserDetectorSD();
  // This is called by RunManager at the beginning of each event. Here we can
  // clear out variables for counters and accumulators
  virtual void Initialize(/*G4HCofThisEvent *HCE*/);

  // This is called at each step of the particle crosssing the sensitive
  // detector. Allows access to all information about particle properties and
  // its interaction with material. It is in ProcessHits that we create hits
  virtual G4bool ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist);

  void processReactionInfo(const G4Step *aStep);

  // This is called at the end of an event
  virtual void EndOfEvent(/*G4HCofThisEvent *HCE*/);

  void printInfo();

private:
  // TotalHitsCollection* hitsCollection;
  // G4int HCID;

private:
  std::map<G4ParticleDefinition *, G4int> fParticleFlag;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
