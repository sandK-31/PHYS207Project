#include "SpacePrimaryGeneratorAction.hh"

SpacePrimaryGeneratorAction::SpacePrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction() {
  const G4double wavelength_nm = 1000.0;
  const G4double energy = 1239.84193 / wavelength_nm;

  fParticleGun = new G4ParticleGun(1);

  auto *particleDefinition =
      G4ParticleTable::GetParticleTable()->FindParticle("opticalphoton");

  fParticleGun->SetParticleDefinition(particleDefinition);
  fParticleGun->SetParticleEnergy(energy * eV);
}

SpacePrimaryGeneratorAction::~SpacePrimaryGeneratorAction() {
  delete fParticleGun;
}

void SpacePrimaryGeneratorAction::GeneratePrimaries(G4Event *anEvent) {
  fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));
  fParticleGun->SetParticleMomentumDirection(G4RandomDirection());
  fParticleGun->GeneratePrimaryVertex(anEvent);
}
