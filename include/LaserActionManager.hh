// ActionManager.hh
#ifndef LaserACTIONMANAGER_h
#define LaserACTIONMANAGER_h 1

#include "globals.hh"

class LaserActionManager {
public:
  LaserActionManager();
  ~LaserActionManager();

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
