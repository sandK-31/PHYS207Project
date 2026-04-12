#include "SpaceSteppingAction.hh"

using namespace std;

// SteppingAction.cc

SpaceSteppingAction::SpaceSteppingAction(SpaceActionManager *manager)
    : G4UserSteppingAction(), fActionManager(manager) {}

SpaceSteppingAction::~SpaceSteppingAction() {}

void SpaceSteppingAction::UserSteppingAction(const G4Step *aStep) {
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

  // --- NEW CODE TO VERIFY STEP LENGTH ---

  // Get the distance the particle traveled during this specific step
  G4double stepLength = aStep->GetStepLength();

  // Get the track to identify the particle (optional, but good for debugging)
  G4Track *track = aStep->GetTrack();
  G4String particleName = track->GetDefinition()->GetParticleName();

  // Print the step length to the console.
  // Dividing by 'm' converts Geant4's internal units back to meters for easy
  // verification. We only print if the step length is greater than 0 to avoid
  // printing boundary crossings.
  if (stepLength > 0.0) {
    G4cout << "Event ID: " << eventID << " | Particle: " << particleName
           << " | Step Length: " << stepLength / m << " m" << G4endl;
  }
}
