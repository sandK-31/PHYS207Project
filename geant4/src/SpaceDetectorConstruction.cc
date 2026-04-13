#include "SpaceDetectorConstruction.hh"

#ifndef PI
#define PI 3.14159265358979312
#endif

SpaceDetectorConstruction::SpaceDetectorConstruction(
    SpaceOpticalPropertiesMessenger *props)
    : G4VUserDetectorConstruction(), fVisAttributes(), matDef(nullptr) {
  std::cout << "detector constructor\n";
  matDef = new SpaceMaterialsDefinition(props);
}

SpaceDetectorConstruction::~SpaceDetectorConstruction() {
  delete matDef;

  for (G4int i = 0; i < G4int(fVisAttributes.size()); ++i) {
    delete fVisAttributes[i];
  }
}

G4VPhysicalVolume *SpaceDetectorConstruction::Construct() {
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  checkOverlaps = true;
  BuildCloud();
  return cloud_physV;
}

void SpaceDetectorConstruction::BuildCloud() {
  auto *cloud_solid =
      new G4Sphere("cloud_solid", 0.0 * m, 400.0 * m, 0, 2 * PI, 0, PI);

  cloud_logical =
      new G4LogicalVolume(cloud_solid, matDef->GetVacuum(), "cloud_logical");

  // Smaller step = better accuracy for your manual interaction model
  G4double maxStep = 0.01 * m;
  auto *stepLimit = new G4UserLimits(maxStep);
  cloud_logical->SetUserLimits(stepLimit);

  cloud_physV =
      new G4PVPlacement(nullptr, G4ThreeVector(), cloud_logical, "cloud_physV",
                        nullptr, false, 0, checkOverlaps);
}
