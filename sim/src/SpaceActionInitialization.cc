#include "SpaceActionInitialization.hh"
#include "SpaceEventAction.hh"
#include "SpacePrimaryGeneratorAction.hh"
#include "SpaceRunAction.hh"
#include "SpaceSteppingAction.hh"

SpaceActionInitialization::SpaceActionInitialization(
    G4String n, G4int f, long s1, long s2, SpaceParticleGunMessenger *gunProps,
    std::string macroContent, std::string specContent)
    : G4VUserActionInitialization(), fileName(n), fileNumber(f), seed1(s1),
      seed2(s2), gunMessenger(gunProps), macroContent(macroContent),
      specContent(specContent) {}

SpaceActionInitialization::~SpaceActionInitialization() {}

void SpaceActionInitialization::BuildForMaster() const {
  SpaceActionManager *actionManager = new SpaceActionManager;
  SpaceEventAction *myEventAction = new SpaceEventAction(actionManager);
  SetUserAction(new SpaceRunAction(myEventAction, fileName, fileNumber, seed1,
                                   seed2, specContent, macroContent));
}

void SpaceActionInitialization::Build() const {
  SetUserAction(new SpacePrimaryGeneratorAction(gunMessenger));
  SpaceActionManager *actionManager = new SpaceActionManager;

  SpaceEventAction *myEventAction = new SpaceEventAction(actionManager);
  SpaceSteppingAction *mySteppingAction =
      new SpaceSteppingAction(actionManager);
  SetUserAction(myEventAction);
  SetUserAction(mySteppingAction);
  SetUserAction(new SpaceRunAction(myEventAction, fileName, fileNumber, seed1,
                                   seed2, specContent, macroContent));
}
