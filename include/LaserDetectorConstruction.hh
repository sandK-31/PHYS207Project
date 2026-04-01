#ifndef LaserDetectorConstruction_H
#define LaserDetectorConstruction_H 1

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4VSensitiveDetector;
class G4VisAttributes;
class G4Material;
class LaserOpticalPropertiesMessenger;
class LaserOpticalSurfaceMessenger;

#include "LaserDetectorSD.hh"

#include "G4GenericMessenger.hh"
#include "G4PVPlacement.hh"
#include "G4VUserDetectorConstruction.hh"
#include "LaserMaterialsDefinition.hh"
#include "globals.hh"

#include <vector>

#include "LaserDetectorSD.hh"
#include "LaserMaterialsDefinition.hh"

#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4NistManager.hh"
#include "G4Orb.hh"
#include "G4Polycone.hh"
#include "G4Polyhedra.hh"
#include "G4RunManager.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"

#include "G4Colour.hh"
#include "G4Element.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4OpticalSurface.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RotationMatrix.hh"
#include "G4SDManager.hh"
#include "G4Scintillation.hh"
#include "G4SolidStore.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4VSensitiveDetector.hh"
#include "G4VisAttributes.hh"
#include "G4ios.hh"

#include "G4MTRunManager.hh"
#include "LaserOpticalSurfaceMessenger.hh"

#include "G4UImanager.hh"
#include "G4VVisManager.hh"

#include <fstream>
#include <iostream>
#include <iostream> // std::cout
#include <sstream>  // std::stringstream
#include <string>   // std::string

class G4LogicalVolume;
class G4Material;
class G4GenericMessenger;

class LaserDetectorConstruction : public G4VUserDetectorConstruction {
public:
  LaserDetectorConstruction(LaserOpticalPropertiesMessenger *props,
                            LaserOpticalSurfaceMessenger *surf);
  ~LaserDetectorConstruction();
  G4VPhysicalVolume *Construct();
  LaserMaterialsDefinition *matDef;
  LaserOpticalSurfaceMessenger *surface;

  // LaserDetectorSD* fiberSD;

private:
  void BuildWorld();
  void BuildBeaker();
  void BuildInnerHollow();
  void BuildLiquid();
  void BuildDetectorAir();
  void MakeOpticalSurfaces();

  G4VPhysicalVolume *worldVol_physV;
  G4VPhysicalVolume *beaker_physV;
  G4VPhysicalVolume *innerHollow_physV;
  G4VPhysicalVolume *liquid_physV;
  G4VPhysicalVolume *detector_physV;

  G4Material *air;
  G4Material *glass;
  G4Material *WbLS;
  G4Material *water;

  G4LogicalVolume *worldLogical;
  G4LogicalVolume *outerBeakerLog;
  G4LogicalVolume *innerHollowLog;

  G4PVPlacement *testPV;
  G4double worldSizeX;
  G4double worldSizeY;
  G4double worldSizeZ;
  G4bool checkOverlaps;

protected:
  std::vector<G4VisAttributes *> fVisAttributes;
};

#endif
