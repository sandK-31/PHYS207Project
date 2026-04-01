#ifndef LaserPhysicsList_h
#define LaserPhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronElasticPhysicsXS.hh"

#include "G4HadronInelasticQBBC.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4HadronPhysicsINCLXX.hh"
#include "G4HadronPhysicsQGSP_BIC_AllHP.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"

#include "G4DecayPhysics.hh"
#include "G4IonElasticPhysics.hh"
#include "G4IonINCLXXPhysics.hh"
#include "G4IonPhysicsPHP.hh"
#include "G4IonPhysicsXS.hh"
#include "G4IonQMDPhysics.hh"
#include "G4NeutronTrackingCut.hh"
#include "G4StoppingPhysics.hh"

// particles

#include "G4BaryonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

#include "G4OpBoundaryProcess.hh"
#include "G4OpRayleigh.hh"
#include "G4OpticalPhysics.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class LaserPhysicsList : public G4VModularPhysicsList {
public:
  LaserPhysicsList();
  ~LaserPhysicsList();

public:
  virtual void ConstructParticle();
  virtual void SetCuts();
  virtual void ConstructProcess();
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
