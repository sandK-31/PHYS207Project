#ifndef LaserRun_h
#define LaserRun_h 1

#include "G4AutoLock.hh"
#include "G4Run.hh"
#include "G4Threading.hh"
#include "globals.hh"
// ROOT

class G4Event;

class LaserRun : public G4Run {
public:
  LaserRun();
  virtual ~LaserRun();
  virtual void RecordEvent(const G4Event *);
  virtual void Merge(const G4Run *);

private:
};

#endif
