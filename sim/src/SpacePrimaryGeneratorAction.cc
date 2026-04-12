#include "SpacePrimaryGeneratorAction.hh"

namespace {
G4Mutex myHEPPrimGenMutex = G4MUTEX_INITIALIZER;
}

SpacePrimaryGeneratorAction::SpacePrimaryGeneratorAction(
    SpaceParticleGunMessenger *gunProps)
    : G4VUserPrimaryGeneratorAction(), fParticleGun(nullptr),
      randGen(new TRandom3(0)) {
  gunProps->GunUpdate(); // Update the gun properties from the messenger

  angle = gunProps->GetParticleGunAngle();
  double PolX = gunProps->GetPolarizationX();
  double PolY = gunProps->GetPolarizationY();
  double PolZ = gunProps->GetPolarizationZ();
  double wavelength = gunProps->GetWavelength(); // in nm

  double energy =
      1.23984193e-6 /
      (wavelength * 1e-9); // Convert wavelength in nm to energy in eV

  fParticleGun = new G4ParticleGun(1); // Create new instance for each thread
  auto particleDefinition =
      G4ParticleTable::GetParticleTable()->FindParticle("opticalphoton");
  fParticleGun->SetParticleDefinition(particleDefinition);
  fParticleGun->SetParticleEnergy(energy * eV); // 633nm HeNe
  /*
  if(PolX!=10 && PolY!=10 && PolZ!=10){
    G4cout << "Setting polarization to: " << PolX << " " << PolY << " " << PolZ
  << G4endl; G4ThreeVector kphoton(0, yMomentum, zMomentum); kphoton =
  kphoton.unit();
    // Initial polarization guess (not necessarily orthogonal)
    G4ThreeVector polarization(PolX, PolY, PolZ);
    // Make polarization perpendicular to momentum
    G4ThreeVector pol_perp = polarization - (polarization.dot(kphoton)) *
  kphoton;
    // Normalize polarization
    pol_perp = pol_perp.unit();
    G4cout << "Setting polarization to: " << pol_perp.x() << " " << pol_perp.y()
  << " " << pol_perp.z() << G4endl;
    fParticleGun->SetParticlePolarization(pol_perp);
  }
  */
}

SpacePrimaryGeneratorAction::~SpacePrimaryGeneratorAction() {
  delete fParticleGun;
}

void SpacePrimaryGeneratorAction::GeneratePrimaries(G4Event *anEvent) {

  // Making gaussian space pulse for MILK
  double sigma =
      4.04 * 0.07486029798533422 * mm; // Standard deviation of Gaussian profile

  // Generate two independent Gaussian variables with mean 0 and standard
  // deviation sigma
  double x_r = randGen->Gaus(0, sigma);
  double y_r = randGen->Gaus(0, sigma);

  // Compute the Rayleigh-distributed variable
  double r = sqrt(x_r * x_r + y_r * y_r);

  // Sample angle uniformly
  double theta = randGen->Uniform(0, 2 * M_PI);

  // Convert to Cartesian coordinates
  double x = r * cos(theta);
  double y = r * sin(theta);

  G4ThreeVector position = G4ThreeVector(x, y, 197 * cm); // Set beam position
  fParticleGun->SetParticlePosition(position);

  G4ThreeVector dir = G4ThreeVector(0, 0, -1); // Propagate along -z
  fParticleGun->SetParticleMomentumDirection(dir);

  fParticleGun->GeneratePrimaryVertex(anEvent);
}
