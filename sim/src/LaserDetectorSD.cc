#include "LaserDetectorSD.hh"

LaserDetectorSD::LaserDetectorSD(G4String name) : G4VSensitiveDetector(name) {
  G4cout << "your sensitive detector's name is " << name << G4endl;
  G4SDManager::GetSDMpointer()->AddNewDetector(this);
}

LaserDetectorSD::~LaserDetectorSD() {}

G4bool LaserDetectorSD::ProcessHits(G4Step *aStep, G4TouchableHistory *) {
  return true;
}

void LaserDetectorSD::processReactionInfo(const G4Step *aStep) {}

void LaserDetectorSD::Initialize() {}

void LaserDetectorSD::EndOfEvent() {}
