#include "LaserDetectorConstruction.hh"

#ifndef PI
#define PI 3.14159265358979312
#endif

LaserDetectorConstruction::LaserDetectorConstruction(
    LaserOpticalPropertiesMessenger *props, LaserOpticalSurfaceMessenger *surf)
    : G4VUserDetectorConstruction(), fVisAttributes() {
  std::cout << "detector constructor\n";
  ///////////////////////////////////////
  // Variables
  ///////////////////////////////////////
  worldSizeX = 200.0 * cm;
  worldSizeY = 200.0 * cm;
  worldSizeZ = 200.0 * cm;

  G4double angle_target, detSize, caseThickness, fiberLength, fibSize,
      fibCaseThickness, fiberPitch;
  detSize = 100 * mm;
  fiberPitch = 20 * mm;
  caseThickness = 3.175 * mm; // 1/8 inch thick aluminum
  fiberLength = 0.5 * m;
  fibSize = .5 * mm; // radius
  fibCaseThickness =
      .02 *
      fibSize; // 2% according to Kuraray, 2% per
               // https://cds.cern.ch/record/1662547/files/LHCb-PUB-2014-023.pdf

  G4Colour red(1, 0, 0);
  G4Colour blue(0, 0, 1);
  G4Colour yellow(1, 1, 0);

  // Materials
  ///////////////////////////////////////
  // G4Material *theMaterial;

  surface = surf;
  matDef = new LaserMaterialsDefinition(props);
}

LaserDetectorConstruction::~LaserDetectorConstruction() {
  for (G4int i = 0; i < G4int(fVisAttributes.size()); ++i)
    delete fVisAttributes[i];
}

G4VPhysicalVolume *LaserDetectorConstruction::Construct() {
  // Cleanup old geometry
  surface->FinishUpdate();
  surface->TypeUpdate();
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  checkOverlaps = true;

  BuildWorld();
  BuildLiquid();
  BuildDetectorAir();
  MakeOpticalSurfaces();
  return worldVol_physV;
}

void LaserDetectorConstruction::BuildWorld() {
  auto worldVol_solid =
      new G4Box("worldVol_solid", worldSizeX, worldSizeY, worldSizeZ);
  // Create a World logical volume
  worldLogical =
      new G4LogicalVolume(worldVol_solid, matDef->GetAir(), "worldLogical");
  worldLogical->SetVisAttributes(G4VisAttributes::GetInvisible());
  // Create World physical volume

  worldVol_physV =
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), worldLogical,
                        "worldVol_physV", 0, false, 0, checkOverlaps);
}

void LaserDetectorConstruction::BuildBeaker() {
  G4Tubs *outerBeaker =
      new G4Tubs("outerBeaker", 0, 2 * cm, 2 * cm, 0, 2 * M_PI);
  G4Tubs *innerBeaker =
      new G4Tubs("innerBeaker", 0, 1.9 * cm, 2 * cm, 0, 2 * M_PI);

  G4VSolid *beaker =
      new G4SubtractionSolid("Cylinder-Cylinder", outerBeaker, innerBeaker, 0,
                             G4ThreeVector(0, 0., .1 * cm));

  outerBeakerLog =
      new G4LogicalVolume(beaker, matDef->GetGlass(), "outerBeakerLog");
  beaker_physV =
      new G4PVPlacement(0, G4ThreeVector(), outerBeakerLog, "outerBeakerPhys",
                        worldLogical, false, 0, checkOverlaps);
}

void LaserDetectorConstruction::BuildLiquid() {
  G4Tubs *Liquid = new G4Tubs("Liquid", 0, 199.5 * cm, 198 * cm, 0, 2 * M_PI);
  G4LogicalVolume *LiquidLog =
      new G4LogicalVolume(Liquid, matDef->GetWater(), "LiquidLog");
  liquid_physV = new G4PVPlacement(
      0, G4ThreeVector(0, 0, -1.9 * cm), LiquidLog, "LiquidPhys", worldLogical,
      false, 0, checkOverlaps); // Offset by -2 cm to center it at 11 cm
}

void LaserDetectorConstruction::BuildDetectorAir() {
  auto detector_solid = new G4Box("detector_solid", 200 * cm, 200 * cm, 1 * cm);
  // Create a World logical volume
  G4LogicalVolume *detectorLog =
      new G4LogicalVolume(detector_solid, matDef->GetAir(), "detectorLogical");
  detector_physV = new G4PVPlacement(0, G4ThreeVector(0, 0, 198.1 * cm),
                                     detectorLog, "detector_physV",
                                     worldLogical, false, 0, checkOverlaps);
}

void LaserDetectorConstruction::MakeOpticalSurfaces() {
  matDef->CreateBorderSurface("LiquidtoWorld", unified, surface->WaterAirType(),
                              surface->WaterAirFinish(), liquid_physV,
                              worldVol_physV);
  // matDef->CreateBorderSurface("GlassLiquid", unified,
  // surface->GlassWaterType(),surface->GlassWaterFinish(), liquid_physV,
  // beaker_physV); matDef->CreateBorderSurface("AirGlass", unified,
  // surface->AirGlassType(),surface->AirGlassFinish(), beaker_physV,
  // worldLogical); matDef->CreateBorderSurface("GlassWorld", unified,
  // surface->GlassWorldType(),surface->GlassWorldFinish(), beaker_physV,
  // worldVol_physV);
}
