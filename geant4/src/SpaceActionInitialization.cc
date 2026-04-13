#include "SpaceActionInitialization.hh"
#include "SpaceEventAction.hh"
#include "SpacePrimaryGeneratorAction.hh"
#include "SpaceRunAction.hh"
#include "SpaceSteppingAction.hh"

SpaceActionInitialization::SpaceActionInitialization(G4String n, G4int f,
                                                     long s1, long s2,
                                                     std::string macroContent,
                                                     std::string specContent)
    : G4VUserActionInitialization(), fileName(n), fileNumber(f), seed1(s1),
      seed2(s2), macroContent(macroContent), specContent(specContent) {}

SpaceActionInitialization::~SpaceActionInitialization() {}

void SpaceActionInitialization::BuildForMaster() const {
  SetUserAction(new SpaceRunAction(fileName, fileNumber, seed1, seed2,
                                   specContent, macroContent));
}

void SpaceActionInitialization::Build() const {
  SetUserAction(new SpacePrimaryGeneratorAction());

  auto *actionManager = new SpaceActionManager;
  auto *eventAction = new SpaceEventAction(actionManager);
  auto *steppingAction = new SpaceSteppingAction(actionManager);

  SetUserAction(eventAction);
  SetUserAction(steppingAction);
  SetUserAction(new SpaceRunAction(fileName, fileNumber, seed1, seed2,
                                   specContent, macroContent));
}
