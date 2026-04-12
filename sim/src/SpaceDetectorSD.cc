#include "SpaceDetectorSD.hh"

SpaceDetectorSD::SpaceDetectorSD(G4String name) : G4VSensitiveDetector(name) {
  G4cout << "your sensitive detector's name is " << name << G4endl;
  G4SDManager::GetSDMpointer()->AddNewDetector(this);
}

SpaceDetectorSD::~SpaceDetectorSD() {}

G4bool SpaceDetectorSD::ProcessHits(G4Step *aStep, G4TouchableHistory *) {
  return true;
}

void SpaceDetectorSD::processReactionInfo(const G4Step *aStep) {}

void SpaceDetectorSD::Initialize() {}

void SpaceDetectorSD::EndOfEvent() {}
