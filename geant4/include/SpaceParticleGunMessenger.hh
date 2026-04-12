#ifndef SpaceParticleGunMessenger_h
#define SpaceParticleGunMessenger_h 1

#include "G4GenericMessenger.hh"
#include <iostream>
#include <string>
#include <vector>

class SpaceParticleGunMessenger {
public:
  SpaceParticleGunMessenger();
  ~SpaceParticleGunMessenger();

  inline G4double GetParticleGunAngle() { return Angle; };
  inline G4double GetPolarizationX() { return PolX; };
  inline G4double GetPolarizationY() { return PolY; };
  inline G4double GetPolarizationZ() { return PolZ; };
  inline G4double GetWavelength() { return Wave; };

  void GunUpdate();

private:
  G4GenericMessenger *fMessenger;
  double fAngle = -9999;
  double fPolX;
  double fPolY;
  double fPolZ;
  double fWave; // nm

  double Angle;
  double PolX;
  double PolY;
  double PolZ;
  double Wave; // nm
};

#endif
