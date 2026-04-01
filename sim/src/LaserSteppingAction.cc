#include "LaserSteppingAction.hh"

using namespace std;

// SteppingAction.cc

LaserSteppingAction::LaserSteppingAction(LaserActionManager *manager)
    : G4UserSteppingAction(), fActionManager(manager) {}

LaserSteppingAction::~LaserSteppingAction() {}

void LaserSteppingAction::UserSteppingAction(const G4Step *aStep) {
  EventAction *myEventAction =
      (EventAction *)(G4EventManager::GetEventManager()->GetUserEventAction());
  auto analysisManager = G4AnalysisManager::Instance();

  int eventID =
      G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();

  G4double upperLimit = 197 * cm;
  G4double sideLimit = 200 * cm;
  G4ThreeVector position = aStep->GetPostStepPoint()->GetPosition();
  G4ThreeVector direction = aStep->GetPostStepPoint()->GetMomentumDirection();
  G4double timeOfArrival = aStep->GetPreStepPoint()->GetGlobalTime();

  G4double preTime = aStep->GetPreStepPoint()->GetGlobalTime();
  G4ThreeVector prePosition = aStep->GetPreStepPoint()->GetPosition();

  if (position.z() > upperLimit && std::abs(position.x()) < sideLimit &&
      std::abs(position.y()) < sideLimit && direction.z() > 0 &&
      prePosition.z() > 1959) {

    G4ThreeVector postPolarization =
        aStep->GetPostStepPoint()->GetPolarization();
    G4ThreeVector prePolarization = aStep->GetPreStepPoint()->GetPolarization();
    G4ThreeVector preMomentum =
        aStep->GetPreStepPoint()->GetMomentumDirection();

    analysisManager->FillNtupleDColumn(0, 0,
                                       prePosition.x() / mm); // x position
    analysisManager->FillNtupleDColumn(0, 1,
                                       prePosition.y() / mm); // y position
    analysisManager->FillNtupleDColumn(0, 2, preTime);        // time
    analysisManager->AddNtupleRow(0);

    analysisManager->FillH2(0, prePosition.x(),
                            prePosition.y()); // Fill the 2D histogram.

    aStep->GetTrack()->SetTrackStatus(fStopAndKill);
  }
}
