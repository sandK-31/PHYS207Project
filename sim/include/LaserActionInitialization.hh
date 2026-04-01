//
#ifndef LaserActionInitialization_h
#define LaserActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "LaserActionManager.hh"
#include "LaserDetectorSD.hh"
#include "LaserEventAction.hh"
#include "LaserParticleGunMessenger.hh"
#include "LaserPrimaryGeneratorAction.hh"
#include "LaserRunAction.hh"
#include "LaserSteppingAction.hh"

class DetectorConstruction;
class LaserParticleGunMessenger;

/// Action initialization class.

class LaserActionInitialization : public G4VUserActionInitialization {
public:
  LaserActionInitialization(G4String, G4int, long, long,
                            LaserParticleGunMessenger *gunProps, std::string,
                            std::string);
  virtual ~LaserActionInitialization();

  virtual void BuildForMaster() const;
  virtual void Build() const;

private:
  G4int fileNumber;
  G4String fileName;
  G4int seed1;
  G4int seed2;
  LaserParticleGunMessenger *gunMessenger;
  std::string macroContent;
  std::string specContent;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
