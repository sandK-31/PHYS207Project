#ifndef LaserPrimaryGeneratorAction_h
#define LaserPrimaryGeneratorAction_h 1

#include "G4DataVector.hh"
#include "G4ThreeVector.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "Randomize.hh"
#include "vector"
#include <cmath>

#include "G4ParticleGun.hh"
#include "globals.hh"

#include "LaserRunAction.hh"

#include "G4AutoLock.hh"
#include "G4BosonConstructor.hh"
#include "G4Box.hh"
#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4RandomDirection.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "TRandom3.h"

#include "LaserParticleGunMessenger.hh"
#include "Randomize.hh"
class G4Event;
class G4ParticleGun;
class G4Box;
class LaserParticleGunMessenger;

class LaserPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
  LaserPrimaryGeneratorAction(LaserParticleGunMessenger *gunProps);
  virtual ~LaserPrimaryGeneratorAction();

  virtual void GeneratePrimaries(G4Event *anEvent);
  const G4ParticleGun *GetParticleGun() const { return fParticleGun; }

private:
  G4ParticleGun *fParticleGun;
  TRandom3 *randGen;
  double maxEnergy; // for distribution from file
  double maxProb;   // for distribution from file
  std::vector<double> distr_prob;
  std::vector<double> distr_energy;
  double totalTime;

  double angle; // degrees
  double surfaceLevel;
  double hypotenuse; // Hypotenuse of the triangle formed by the laser beam

  float yPos;
  float zPos;

  float yMomentum;
  float zMomentum;
};

#endif
