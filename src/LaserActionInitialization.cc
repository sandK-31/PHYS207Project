#include "LaserActionInitialization.hh"
#include "LaserEventAction.hh"
#include "LaserPrimaryGeneratorAction.hh"
#include "LaserRunAction.hh"
#include "LaserSteppingAction.hh"

LaserActionInitialization::LaserActionInitialization(
    G4String n, G4int f, long s1, long s2, LaserParticleGunMessenger *gunProps,
    std::string macroContent, std::string specContent)
    : G4VUserActionInitialization(), fileName(n), fileNumber(f), seed1(s1),
      seed2(s2), gunMessenger(gunProps), macroContent(macroContent),
      specContent(specContent) {}

LaserActionInitialization::~LaserActionInitialization() {}

void LaserActionInitialization::BuildForMaster() const {
  LaserActionManager *actionManager = new LaserActionManager;
  LaserEventAction *myEventAction = new LaserEventAction(actionManager);
  SetUserAction(new LaserRunAction(myEventAction, fileName, fileNumber, seed1,
                                   seed2, specContent, macroContent));
}

void LaserActionInitialization::Build() const {
  SetUserAction(new LaserPrimaryGeneratorAction(gunMessenger));
  LaserActionManager *actionManager = new LaserActionManager;

  LaserEventAction *myEventAction = new LaserEventAction(actionManager);
  LaserSteppingAction *mySteppingAction =
      new LaserSteppingAction(actionManager);
  SetUserAction(myEventAction);
  SetUserAction(mySteppingAction);
  SetUserAction(new LaserRunAction(myEventAction, fileName, fileNumber, seed1,
                                   seed2, specContent, macroContent));
}
