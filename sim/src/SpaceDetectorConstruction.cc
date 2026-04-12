#include "SpaceDetectorConstruction.hh"

#ifndef PI
#define PI 3.14159265358979312
#endif

SpaceDetectorConstruction::SpaceDetectorConstruction(
    SpaceOpticalPropertiesMessenger *props)
    : G4VUserDetectorConstruction(), fVisAttributes() {
  std::cout << "detector constructor\n";

  // Materials
  ///////////////////////////////////////

  matDef = new SpaceMaterialsDefinition(props);
}

SpaceDetectorConstruction::~SpaceDetectorConstruction() {
  for (G4int i = 0; i < G4int(fVisAttributes.size()); ++i)
    delete fVisAttributes[i];
}

G4VPhysicalVolume *SpaceDetectorConstruction::Construct() {
  // Cleanup old geometry
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  checkOverlaps = true;

  BuildCloud();
  return cloud_physV;
}

void SpaceDetectorConstruction::BuildCloud() {
  auto cloud_solid =
      new G4Sphere("cloud_solid", 0.0 * m, 200 * m, 0, 2 * PI, 0, PI);
  // Create a World logical volume
  cloud_logical =
      new G4LogicalVolume(cloud_solid, matDef->GetAir(), "cloud_logical");

  // 1. Create the user limits object (1 meter in your scaled simulation)
  G4double maxStep = 19.0 * m;
  G4UserLimits *stepLimit = new G4UserLimits(maxStep);

  // 2. Assign it to your logical volume
  // (Replace 'logicSpaceCloud' with the actual name of your logical volume)
  cloud_logical->SetUserLimits(stepLimit);

  cloud_physV = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), cloud_logical,
                                  "cloud_physV", 0, false, 0, checkOverlaps);
}
