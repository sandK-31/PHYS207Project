#include "SpaceActionManager.hh"

SpaceActionManager::SpaceActionManager()
    : fTotalPathLength(0.0), fRayleighCount(0), fAbsorbed(false) {}

SpaceActionManager::~SpaceActionManager() {}

void SpaceActionManager::Reset() {
  fTotalPathLength = 0.0;
  fRayleighCount = 0;
  fAbsorbed = false;
}

void SpaceActionManager::AddToPathLength(G4double length) {
  fTotalPathLength += length;
}

void SpaceActionManager::IncrementRayleighCount() { fRayleighCount++; }

G4double SpaceActionManager::GetTotalPathLength() const {
  return fTotalPathLength;
}

G4int SpaceActionManager::GetRayleighCount() const { return fRayleighCount; }

void SpaceActionManager::SetAbsorbed(G4bool absorbed) { fAbsorbed = absorbed; }

G4bool SpaceActionManager::WasAbsorbed() const { return fAbsorbed; }
