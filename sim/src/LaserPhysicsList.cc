#include "LaserPhysicsList.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LaserPhysicsList::LaserPhysicsList() : G4VModularPhysicsList() {
  G4int verb = 2;
  SetVerboseLevel(verb);

  // Register EM physics from FTFP_BERT manually

  RegisterPhysics(
      new G4EmStandardPhysics_option4());          // This replaces default EM
  RegisterPhysics(new G4DecayPhysics());           // Needed for particle decays
  RegisterPhysics(new G4HadronElasticPhysics());   // Hadronic processes
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT()); // Hadronic model
  RegisterPhysics(new G4StoppingPhysics());        // For stopped particles
  RegisterPhysics(new G4IonPhysics());             // Ions
  RegisterPhysics(new G4NeutronTrackingCut());     // Neutron cuts

  // Optical physics
  auto *opticalPhysics = new G4OpticalPhysics();
  opticalPhysics->SetVerboseLevel(2); // Optional: prints OpBoundary info
  RegisterPhysics(opticalPhysics);

  auto opticalParams = G4OpticalParameters::Instance();
  opticalParams->SetWLSTimeProfile("exponential");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LaserPhysicsList::~LaserPhysicsList() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LaserPhysicsList::ConstructParticle() {
  G4VModularPhysicsList::ConstructParticle();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LaserPhysicsList::ConstructProcess() {
  G4VModularPhysicsList::ConstructProcess();
}

void LaserPhysicsList::SetCuts() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
