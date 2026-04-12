#ifndef SpaceRun_h
#define SpaceRun_h 1

#include "G4AutoLock.hh"
#include "G4Run.hh"
#include "G4Threading.hh"
#include "globals.hh"
// ROOT

class G4Event;

class SpaceRun : public G4Run {
public:
  SpaceRun();
  virtual ~SpaceRun();
  virtual void RecordEvent(const G4Event *);
  virtual void Merge(const G4Run *);

private:
};

#endif
