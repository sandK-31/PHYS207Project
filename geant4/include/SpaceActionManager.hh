// ActionManager.hh
#ifndef SpaceACTIONMANAGER_h
#define SpaceACTIONMANAGER_h 1

#include "globals.hh"

class SpaceActionManager {
public:
  SpaceActionManager();
  ~SpaceActionManager();

  void Reset();
  void AddToPathLength(G4double length);
  void IncrementRayleighCount();

  G4double GetTotalPathLength() const;
  G4int GetRayleighCount() const;

  G4double fTotalPathLength;
  G4int fRayleighCount;
  G4bool fAbsorbed;

private:
};
#endif
