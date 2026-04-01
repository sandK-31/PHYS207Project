#include "LaserEventAction.hh"

LaserEventAction::LaserEventAction(LaserActionManager *manager)
    : G4UserEventAction(), fdetHCID(-1), fActionManager(manager) {}

LaserEventAction::~LaserEventAction() {
  G4cout << "Deleting EventAction";
  G4cout << "...done" << G4endl;
}

void LaserEventAction::BeginOfEventAction(const G4Event *event) {}

void LaserEventAction::EndOfEventAction(const G4Event *event) {

  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();

  G4double totalPathLength = fActionManager->GetTotalPathLength();
  G4int rayleighCount = fActionManager->GetRayleighCount();
  /*
  if(fActionManager->fAbsorbed == true){
    analysisManager->FillH1(0, totalPathLength);
  }

  if (rayleighCount > 0)
  {
    analysisManager->FillH1(1, totalPathLength / rayleighCount);
  }
  */
  // Reset the stepping action counters for the next event
  fActionManager->Reset();
}
