#include "LaserRun.hh"

namespace {
G4Mutex myRunMutex = G4MUTEX_INITIALIZER;
}

LaserRun::LaserRun() : G4Run() {
  G4cout << "Creating Run in thread " << G4Threading::G4GetThreadId();
  G4cout << "...done." << G4endl;
}

LaserRun::~LaserRun() {}

void LaserRun::RecordEvent(const G4Event *evt) {
  G4Run::RecordEvent(evt);
  return;
}

void LaserRun::Merge(const G4Run *run) {
  G4Run::Merge(run);
  return;
}
