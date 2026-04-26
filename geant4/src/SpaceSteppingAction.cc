#include "SpaceSteppingAction.hh"

#include "G4AnalysisManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <algorithm>
#include <cmath>
#include <limits>

SpaceSteppingAction::SpaceSteppingAction(SpaceActionManager *manager)
    : G4UserSteppingAction(), fActionManager(manager) {}

SpaceSteppingAction::~SpaceSteppingAction() {}

void SpaceSteppingAction::UserSteppingAction(const G4Step *aStep) {
  G4Track *track = aStep->GetTrack();

  if (track->GetDefinition() != G4OpticalPhoton::Definition())
    return;

  G4ThreeVector pre = aStep->GetPreStepPoint()->GetPosition();
  G4ThreeVector post = aStep->GetPostStepPoint()->GetPosition();
  G4ThreeVector position = 0.5 * (pre + post);
  const G4double stepLength = aStep->GetStepLength();

  OpticalLengths lengths = GetOpticalLengthsAtPosition(position);

  if (!std::isfinite(lengths.absorptionLength) ||
      !std::isfinite(lengths.scatteringLength)) {
    return;
  }

  if (lengths.absorptionLength <= 0.0 || lengths.scatteringLength <= 0.0) {
    return;
  }

  if (fActionManager) {
    fActionManager->AddToPathLength(stepLength);
  }

  const G4double invAbs = 1.0 / lengths.absorptionLength;
  const G4double invScat = 1.0 / lengths.scatteringLength;
  const G4double invTotal = invAbs + invScat;

  if (invTotal <= 0.0)
    return;

  const G4double lambdaTotal = 1.0 / invTotal;
  const G4double pInteraction = 1.0 - std::exp(-stepLength / lambdaTotal);

  if (G4UniformRand() > pInteraction)
    return;

  const G4double pScat = invScat / invTotal;

  if (G4UniformRand() < pScat) {
    // Scatter
    const G4double g = 8.611102e-02;

    G4double cosTheta;
    if (std::abs(g) < 1e-12) {
      cosTheta = 2.0 * G4UniformRand() - 1.0;
    } else {
      const G4double u = G4UniformRand();
      const G4double term = (1.0 - g * g) / (1.0 - g + 2.0 * g * u);
      cosTheta = (1.0 + g * g - term * term) / (2.0 * g);
    }

    cosTheta = std::max(-1.0, std::min(1.0, cosTheta));
    const G4double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
    const G4double phi = 2.0 * CLHEP::pi * G4UniformRand();

    G4ThreeVector newDirection(sinTheta * std::cos(phi),
                               sinTheta * std::sin(phi), cosTheta);

    const G4ThreeVector oldDirection = track->GetMomentumDirection();
    newDirection.rotateUz(oldDirection);
    track->SetMomentumDirection(newDirection);

    if (fActionManager) {
      fActionManager->IncrementRayleighCount();
    }

    auto *analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleDColumn(0, 0, position.x() / m);
    analysisManager->FillNtupleDColumn(0, 1, position.y() / m);
    analysisManager->FillNtupleDColumn(0, 2, position.z() / m);
    G4int eventID =
        G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    analysisManager->FillNtupleIColumn(0, 3, eventID);
    analysisManager->AddNtupleRow(0);

    // Histograms
    analysisManager->FillH2(0, position.x() / m,
                            position.z() / m);    // scatter_xy
    analysisManager->FillH1(0, position.y() / m); // scatter_z
  } else {
    // Absorb
    if (fActionManager) {
      fActionManager->SetAbsorbed(true);
    }
    track->SetTrackStatus(fStopAndKill);
  }
}

SpaceSteppingAction::OpticalLengths
SpaceSteppingAction::GetOpticalLengthsAtPosition(G4ThreeVector position) {
  const double M_star = 1.98892e33;
  const double L_star = 3.8525e33;
  const double G = 6.6743e-8;
  const double k_B = 1.380649e-16;
  const double m_p = 1.67262e-24;
  const double sigma_SB = 5.670374e-5;
  const double mu = 2.3;
  const double alpha = 0.05;
  const double Sigma_d0 = 0.1;
  const double AU_to_cm = 1.495978707e13;

  const double kappa_abs = 1.897725e+03;
  const double kappa_scat = 1.424672e+03;

  const double r_AU = position.mag() / m;

  if (r_AU < 10.0 || r_AU > 100.0) {
    return {std::numeric_limits<G4double>::infinity(),
            std::numeric_limits<G4double>::infinity()};
  }

  const double r_cm = r_AU * AU_to_cm;
  const double z_cm = (position.z() / m) * AU_to_cm;

  const double theta = std::acos(z_cm / r_cm);

  const double Sigma_d = Sigma_d0 * std::pow(r_AU, -1.0);

  const double F_irr = (alpha * L_star) / (4.0 * CLHEP::pi * r_cm * r_cm);
  const double T_mid = std::pow(F_irr / sigma_SB, 0.25);

  const double c_s = std::sqrt((k_B * T_mid) / (mu * m_p));
  const double Omega_k = std::sqrt((G * M_star) / std::pow(r_cm, 3));
  const double h = c_s / Omega_k;

  const double exponent = -1.0 * std::pow((CLHEP::pi / 2.0 - theta), 2) /
                          (2.0 * std::pow(h / r_cm, 2));

  const double rho_d =
      (Sigma_d / (std::sqrt(2.0 * CLHEP::pi) * h)) * std::exp(exponent);

  if (rho_d < 1e-30) {
    return {std::numeric_limits<G4double>::infinity(),
            std::numeric_limits<G4double>::infinity()};
  }

  const double lambda_abs_cm = 1.0 / (rho_d * kappa_abs);
  const double lambda_scat_cm = 1.0 / (rho_d * kappa_scat);

  const double lambda_abs_geant4 = (lambda_abs_cm / AU_to_cm) * m;
  const double lambda_scat_geant4 = (lambda_scat_cm / AU_to_cm) * m;

  return {lambda_abs_geant4, lambda_scat_geant4};
}
