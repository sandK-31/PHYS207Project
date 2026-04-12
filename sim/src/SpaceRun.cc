#include "SpaceRun.hh"

namespace {
G4Mutex myRunMutex = G4MUTEX_INITIALIZER;
}

SpaceRun::SpaceRun() : G4Run() {
  G4cout << "Creating Run in thread " << G4Threading::G4GetThreadId();
  G4cout << "...done." << G4endl;
}

SpaceRun::~SpaceRun() {}

void SpaceRun::RecordEvent(const G4Event *evt) {
  G4Run::RecordEvent(evt);
  return;
}

void SpaceRun::Merge(const G4Run *run) {
  G4Run::Merge(run);
  return;
}
