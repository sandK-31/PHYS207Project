#include "LaserActionManager.hh"

// Constructor
LaserActionManager::LaserActionManager()
    : fTotalPathLength(0.0), fRayleighCount(0), fAbsorbed(false) {}

// Destructor
LaserActionManager::~LaserActionManager() {}

// Reset variables
void LaserActionManager::Reset() {
  fTotalPathLength = 0.0;
  fRayleighCount = 0;
  fAbsorbed = false;
}

// Add to the total path length
void LaserActionManager::AddToPathLength(G4double length) {
  fTotalPathLength += length;
}

// Increment the Rayleigh scattering count
void LaserActionManager::IncrementRayleighCount() { fRayleighCount++; }

// Get the total path length
G4double LaserActionManager::GetTotalPathLength() const {
  return fTotalPathLength;
}

// Get the Rayleigh scattering count
G4int LaserActionManager::GetRayleighCount() const { return fRayleighCount; }
