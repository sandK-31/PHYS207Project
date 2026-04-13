#include "SpaceMaterialsDefinition.hh"

#define _tostr(a) #a
#define tostr(a) _tostr(a)

SpaceMaterialsDefinition::SpaceMaterialsDefinition(
    SpaceOpticalPropertiesMessenger *input) {
  manager = G4NistManager::Instance();
  // materials
  matWater = manager->FindOrBuildMaterial("G4_He");
  matVacuum = manager->FindOrBuildMaterial("G4_Galactic");
  matAir = manager->FindOrBuildMaterial("G4_AIR");

  optics = input;
  DefineMaterials();

} // END of constructor

SpaceMaterialsDefinition::~SpaceMaterialsDefinition() {

  G4cout << "Deleting MaterialsDefinition...";

  G4cout << "done" << G4endl;
}

void SpaceMaterialsDefinition::DefineMaterials() {
  G4double density;
  G4int ncomponents, natoms;
  optics->OpticsUpdate();

  G4NistManager *nistManager = G4NistManager::Instance();
  ///////////
  // Elements
  ///////////
  G4Element *H = nistManager->FindOrBuildElement("H");   // Z=1
  G4Element *C = nistManager->FindOrBuildElement("C");   // Z=6
  G4Element *O = nistManager->FindOrBuildElement("O");   // Z=8
  G4Element *F = nistManager->FindOrBuildElement("F");   // Z=9
  G4Element *Pb = nistManager->FindOrBuildElement("Pb"); // Z=9

  // making wavelength independent optical properties
  const G4int nEntries = 2;
  G4double PhotonEnergy[nEntries] = {0.0001 * eV, 100 * eV};

  G4double refractiveIndexAir[nEntries] = {1.001, 1.001};
  G4MaterialPropertiesTable *airMPT = new G4MaterialPropertiesTable();
  airMPT->AddProperty("RINDEX", PhotonEnergy, refractiveIndexAir, nEntries);
  matAir->SetMaterialPropertiesTable(airMPT);

  auto rayleigh_scattering_length = optics->GetIntralipidRayleighScatlength();
  auto absorption_length = optics->GetIntralipidAbslength();
  auto refractive_index = optics->GetIntralipidRindex();
  auto mie_scattering_length = optics->GetIntralipidintralipidMieScatlength();
  auto mie_forward_G = optics->GetIntralipidintralipidMieForwardG();
  auto mie_backward_G = optics->GetIntralipidintralipidMieBackwardG();
  auto mie_forward_backward_ratio_G =
      optics->GetIntralipidintralipidMieFBRatioG();

  //////////////////
  G4MaterialPropertiesTable *waterMPT = new G4MaterialPropertiesTable();
  G4double refractiveIndexWater[nEntries] = {refractive_index,
                                             refractive_index};
  G4double waterAbsorptionLength[nEntries] = {absorption_length * mm,
                                              absorption_length * mm};
  G4double rayleighScatLength[nEntries] = {rayleigh_scattering_length * mm,
                                           rayleigh_scattering_length * mm};
  G4double mieScatLength[nEntries] = {mie_scattering_length * mm,
                                      mie_scattering_length * mm};

  waterMPT->AddProperty("ABSLENGTH", PhotonEnergy, waterAbsorptionLength,
                        nEntries);

  if (rayleigh_scattering_length == -1.0 and mie_scattering_length == -1.0) {
    throw std::invalid_argument(
        "No Scattering Length Values Provided: OpPropsMessenger");
  }

  if (rayleigh_scattering_length != -1.0) {
    waterMPT->AddProperty("RAYLEIGH", PhotonEnergy, rayleighScatLength,
                          nEntries);
    G4cout << "USING RAYLEIGH SCATTERING" << G4endl;
  } else {
    G4cout << "No Rayleigh Scattering Length Provided, Using Mie Scattering"
           << G4endl;
  }

  waterMPT->AddProperty("RINDEX", PhotonEnergy, refractiveIndexWater, nEntries);

  if (mie_scattering_length != -1.0) {
    waterMPT->AddProperty("MIEHG", PhotonEnergy, mieScatLength, nEntries);
    waterMPT->AddConstProperty("MIEHG_FORWARD", mie_forward_G);
    waterMPT->AddConstProperty("MIEHG_BACKWARD", mie_backward_G);
    waterMPT->AddConstProperty("MIEHG_FORWARD_RATIO",
                               mie_forward_backward_ratio_G);
    G4cout << "USING MIE SCATTERING" << G4endl;
  } else {
    G4cout << "No Mie Scattering Length Provided, Using Rayleigh Scattering"
           << G4endl;
  }

  matWater->SetMaterialPropertiesTable(waterMPT);
}
