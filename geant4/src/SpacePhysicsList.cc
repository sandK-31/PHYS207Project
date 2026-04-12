#include "SpacePhysicsList.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SpacePhysicsList::SpacePhysicsList() : G4VModularPhysicsList() {
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

SpacePhysicsList::~SpacePhysicsList() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SpacePhysicsList::ConstructParticle() {
  G4VModularPhysicsList::ConstructParticle();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SpacePhysicsList::ConstructProcess() {
  G4VModularPhysicsList::ConstructProcess();
  // Create the step limiter process
  G4StepLimiter *stepLimiter = new G4StepLimiter();

  // Get the process manager for your specific particle
  // (Change "G4OpticalPhoton" to whatever particle you are using for the radio
  // waves)
  G4ProcessManager *pmanager =
      G4OpticalPhoton::OpticalPhoton()->GetProcessManager();

  // Add the step limiter to the particle's physics processes
  pmanager->AddDiscreteProcess(stepLimiter);
}

void SpacePhysicsList::SetCuts() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
