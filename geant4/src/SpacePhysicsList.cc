#include "SpacePhysicsList.hh"

#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4HadronPhysicsFTFP_BERT.hh"
#include "G4IonPhysics.hh"
#include "G4NeutronTrackingCut.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhysics.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4StoppingPhysics.hh"

SpacePhysicsList::SpacePhysicsList() : G4VModularPhysicsList() {
  SetVerboseLevel(1);

  RegisterPhysics(new G4EmStandardPhysics_option4());
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4HadronElasticPhysics());
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT());
  RegisterPhysics(new G4StoppingPhysics());
  RegisterPhysics(new G4IonPhysics());
  RegisterPhysics(new G4NeutronTrackingCut());

  auto *opticalPhysics = new G4OpticalPhysics();
  opticalPhysics->SetVerboseLevel(0);
  RegisterPhysics(opticalPhysics);

  auto *stepLimiterPhysics = new G4StepLimiterPhysics();
  stepLimiterPhysics->SetApplyToAll(true);
  RegisterPhysics(stepLimiterPhysics);

  auto *opticalParams = G4OpticalParameters::Instance();
  opticalParams->SetWLSTimeProfile("exponential");
}

SpacePhysicsList::~SpacePhysicsList() {}

void SpacePhysicsList::ConstructParticle() {
  G4VModularPhysicsList::ConstructParticle();
}

void SpacePhysicsList::ConstructProcess() {
  G4VModularPhysicsList::ConstructProcess();
}

void SpacePhysicsList::SetCuts() { SetCutsWithDefault(); }
