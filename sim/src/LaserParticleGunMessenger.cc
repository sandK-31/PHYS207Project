#include "LaserParticleGunMessenger.hh"

LaserParticleGunMessenger::LaserParticleGunMessenger() {

  fMessenger =
      new G4GenericMessenger(this, "/generator/", "Particle Gun Properties");

  fMessenger->DeclareProperty("Angle", fAngle, "Beam Angle in degrees");
  fMessenger->DeclareProperty("PolarizationX", fPolX,
                              "Polarization X component");
  fMessenger->DeclareProperty("PolarizationY", fPolY,
                              "Polarization Y component");
  fMessenger->DeclareProperty("PolarizationZ", fPolZ,
                              "Polarization Z component");
  fMessenger->DeclareProperty("Wavelength", fWave, "Photon Wavelength in nm");
}

LaserParticleGunMessenger::~LaserParticleGunMessenger() {}

void LaserParticleGunMessenger::GunUpdate() {
  if (fAngle == -9999) {
    throw std::invalid_argument("No Angle Provided ParticleGunMessenger");
  } else {
    Angle = fAngle;
  }

  if (fPolX == 0 && fPolY == 0 && fPolZ == 0) {
    G4cout << "No Polarization Provided: ParticleGunMessenger" << G4endl;
    PolX = 10.0;
    PolY = 10.0;
    PolZ = 10.0;
  } else {
    PolX = fPolX;
    PolY = fPolY;
    PolZ = fPolZ;
  }

  if (fWave == 0) {
    throw std::invalid_argument("No Wavelength Provided: ParticleGunMessenger");
  } else {
    Wave = fWave;
  }
}
