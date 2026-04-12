#include "SpaceActionManager.hh"

// Constructor
SpaceActionManager::SpaceActionManager()
    : fTotalPathLength(0.0), fRayleighCount(0), fAbsorbed(false) {}

// Destructor
SpaceActionManager::~SpaceActionManager() {}

// Reset variables
void SpaceActionManager::Reset() {
  fTotalPathLength = 0.0;
  fRayleighCount = 0;
  fAbsorbed = false;
}

// Add to the total path length
void SpaceActionManager::AddToPathLength(G4double length) {
  fTotalPathLength += length;
}

// Increment the Rayleigh scattering count
void SpaceActionManager::IncrementRayleighCount() { fRayleighCount++; }

// Get the total path length
G4double SpaceActionManager::GetTotalPathLength() const {
  return fTotalPathLength;
}

// Get the Rayleigh scattering count
G4int SpaceActionManager::GetRayleighCount() const { return fRayleighCount; }
