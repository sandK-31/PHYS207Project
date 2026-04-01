#include "LaserOpticalSurfaceMessenger.hh"

int LaserOpticalSurfaceMessenger::OptionsFinish(G4String input) {
  if (input == "polished")
    return 1;
  else if (input == "ground")
    return 2;
  else if (input == "polishedfrontpainted")
    return 3;
  else if (input == "groundfrontpainted")
    return 4;
  else if (input == "polishedbackpainted")
    return 5;
  else if (input == "groundbackpainted")
    return 6;
  else
    return 0;
}

int LaserOpticalSurfaceMessenger::OptionsType(G4String input) {
  if (input == "dieclectric")
    return 1;
  else if (input == "metal")
    return 2;
  else
    return 0;
}

LaserOpticalSurfaceMessenger::LaserOpticalSurfaceMessenger() {

  finishMessenger = new G4GenericMessenger(this, "/finish/", "Material Finish");

  finishMessenger->DeclareProperty("WaterAir", WaterAirf, "WaterAir Finish");
  finishMessenger->DeclareProperty("AirGlass", AirGlassf, "AirGlass Finish");
  finishMessenger->DeclareProperty("GlassWater", GlassWaterf,
                                   "GlassWater Finish");
  finishMessenger->DeclareProperty("GlassWorld", GlassWorldf,
                                   "GlassWorld Finish");

  typeMessenger = new G4GenericMessenger(this, "/type/", "Material Type");

  typeMessenger->DeclareProperty("WaterAir", WaterAirt, "WaterAir Type");
  typeMessenger->DeclareProperty("AirGlass", AirGlasst, "AirGlass Type");
  typeMessenger->DeclareProperty("GlassWater", GlassWatert, "GlassWater Type");
  typeMessenger->DeclareProperty("GlassWorld", GlassWorldt, "GlassWorld Type");
}

LaserOpticalSurfaceMessenger::~LaserOpticalSurfaceMessenger() {}

void LaserOpticalSurfaceMessenger::FinishUpdate() {
  switch (OptionsFinish(WaterAirf)) {
  case 1:
    WaterAirFinishHolder = polished;
    break;
  case 2:
    WaterAirFinishHolder = ground;
    break;
  case 3:
    WaterAirFinishHolder = polishedfrontpainted;
    break;
  case 4:
    WaterAirFinishHolder = groundfrontpainted;
    break;
  case 5:
    WaterAirFinishHolder = polishedbackpainted;
    break;
  case 6:
    WaterAirFinishHolder = groundbackpainted;
    break;
  default:
    WaterAirFinishHolder = polished;
    G4cout << "!Warning!: Using default WaterAir finish" << G4endl;
  }
  switch (OptionsFinish(AirGlassf)) {
  case 1:
    AirGlassFinishHolder = polished;
    break;
  case 2:
    AirGlassFinishHolder = ground;
    break;
  case 3:
    AirGlassFinishHolder = polishedfrontpainted;
    break;
  case 4:
    AirGlassFinishHolder = groundfrontpainted;
    break;
  case 5:
    AirGlassFinishHolder = polishedbackpainted;
    break;
  case 6:
    AirGlassFinishHolder = groundbackpainted;
    break;
  default:
    AirGlassFinishHolder = polished;
    G4cout << "!Warning!: Using default AirGlass finish" << G4endl;
  }
  switch (OptionsFinish(GlassWaterf)) {
  case 1:
    GlassWaterFinishHolder = polished;
    break;
  case 2:
    GlassWaterFinishHolder = ground;
    break;
  case 3:
    GlassWaterFinishHolder = polishedfrontpainted;
    break;
  case 4:
    GlassWaterFinishHolder = groundfrontpainted;
    break;
  case 5:
    GlassWaterFinishHolder = polishedbackpainted;
    break;
  case 6:
    GlassWaterFinishHolder = groundbackpainted;
    break;
  default:
    GlassWaterFinishHolder = polished;
    G4cout << "!Warning!: Using default GlassWater finish" << G4endl;
  }
  switch (OptionsFinish(GlassWorldf)) {
  case 1:
    GlassWorldFinishHolder = polished;
    break;
  case 2:
    GlassWorldFinishHolder = ground;
    break;
  case 3:
    GlassWorldFinishHolder = polishedfrontpainted;
    break;
  case 4:
    GlassWorldFinishHolder = groundfrontpainted;
    break;
  case 5:
    GlassWorldFinishHolder = polishedbackpainted;
    break;
  case 6:
    GlassWorldFinishHolder = groundbackpainted;
    break;
  default:
    GlassWorldFinishHolder = polished;
    G4cout << "!Warning!: Using default GlassWorld finish" << G4endl;
  }
}

void LaserOpticalSurfaceMessenger::TypeUpdate() {
  switch (OptionsType(WaterAirt)) {
  case 1:
    WaterAirTypeHolder = dielectric_dielectric;
    break;
  case 2:
    WaterAirTypeHolder = dielectric_metal;
    break;
  default:
    WaterAirTypeHolder = dielectric_dielectric;
    G4cout << "!Warning!: Using default WaterAir type" << G4endl;
  }
  switch (OptionsType(AirGlasst)) {
  case 1:
    AirGlassTypeHolder = dielectric_dielectric;
    break;
  case 2:
    AirGlassTypeHolder = dielectric_metal;
    break;
  default:
    AirGlassTypeHolder = dielectric_dielectric;
    G4cout << "!Warning!: Using default AirGlass type" << G4endl;
  }
  switch (OptionsType(GlassWatert)) {
  case 1:
    GlassWaterTypeHolder = dielectric_dielectric;
    break;
  case 2:
    GlassWaterTypeHolder = dielectric_metal;
    break;
  default:
    GlassWaterTypeHolder = dielectric_dielectric;
    G4cout << "!Warning!: Using default GlassWater type" << G4endl;
  }
  switch (OptionsType(GlassWorldt)) {
  case 1:
    GlassWorldTypeHolder = dielectric_dielectric;
    G4cout << "!Warning!: GlassWorld type is dielectric_dielectric" << G4endl;
    break;
  case 2:
    GlassWorldTypeHolder = dielectric_metal;
    G4cout << "!Warning!: GlassWorld type is dielectric_metal" << G4endl;
    break;
  default:
    GlassWorldTypeHolder = dielectric_dielectric;
    G4cout << "!Warning!: Using default GlassWorld type dielectric_dielectric"
           << G4endl;
  }
}
