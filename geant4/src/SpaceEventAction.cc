#include "SpaceEventAction.hh"

SpaceEventAction::SpaceEventAction(SpaceActionManager *manager)
    : G4UserEventAction(), fdetHCID(-1), fActionManager(manager) {}

SpaceEventAction::~SpaceEventAction() {
  G4cout << "Deleting EventAction";
  G4cout << "...done" << G4endl;
}

void SpaceEventAction::BeginOfEventAction(const G4Event *event) {}

void SpaceEventAction::EndOfEventAction(const G4Event *event) {

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
