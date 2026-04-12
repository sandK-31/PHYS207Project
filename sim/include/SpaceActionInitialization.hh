//
#ifndef SpaceActionInitialization_h
#define SpaceActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "SpaceActionManager.hh"
#include "SpaceDetectorSD.hh"
#include "SpaceEventAction.hh"
#include "SpaceParticleGunMessenger.hh"
#include "SpacePrimaryGeneratorAction.hh"
#include "SpaceRunAction.hh"
#include "SpaceSteppingAction.hh"

class DetectorConstruction;
class SpaceParticleGunMessenger;

/// Action initialization class.

class SpaceActionInitialization : public G4VUserActionInitialization {
public:
  SpaceActionInitialization(G4String, G4int, long, long,
                            SpaceParticleGunMessenger *gunProps, std::string,
                            std::string);
  virtual ~SpaceActionInitialization();

  virtual void BuildForMaster() const;
  virtual void Build() const;

private:
  G4int fileNumber;
  G4String fileName;
  G4int seed1;
  G4int seed2;
  SpaceParticleGunMessenger *gunMessenger;
  std::string macroContent;
  std::string specContent;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
