#include "SpaceEventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

SpaceEventAction::SpaceEventAction(SpaceActionManager *manager)
    : G4UserEventAction(), fdetHCID(-1), fActionManager(manager) {}

SpaceEventAction::~SpaceEventAction() {
  delete fActionManager;
  G4cout << "Deleting EventAction...done" << G4endl;
}

void SpaceEventAction::BeginOfEventAction(const G4Event *) {
  if (fActionManager) {
    fActionManager->Reset();
  }
}

void SpaceEventAction::EndOfEventAction(const G4Event *) {
  if (!fActionManager)
    return;

  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();

  G4double totalPathLength = fActionManager->GetTotalPathLength();
  G4int rayleighCount = fActionManager->GetRayleighCount();
  G4bool absorbed = fActionManager->WasAbsorbed();

  // If later you book histograms/ntuples for per-event summaries,
  // fill them here. Example:
  //
  // analysisManager->FillH1(0, totalPathLength / m);
  // if (rayleighCount > 0) {
  //   analysisManager->FillH1(1, (totalPathLength / rayleighCount) / m);
  // }

  (void)analysisManager;
  (void)totalPathLength;
  (void)rayleighCount;
  (void)absorbed;
}
