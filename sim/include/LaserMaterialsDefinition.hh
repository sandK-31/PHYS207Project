#ifndef LaserMaterialsDefinition_H
#define LaserMaterialsDefinition_H 1

#include "G4Element.hh"
#include "G4Isotope.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"

#include "globals.hh"
#include <iostream>

#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4OpticalSurface.hh"

#include "G4OpticalSurface.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "LaserOpticalSurfaceMessenger.hh"

#include "G4LogicalBorderSurface.hh"

#include "LaserOpticalPropertiesMessenger.hh"
class LaserOpticalPropertiesMessenger;
enum materialName { MATAIR, MATGLASS, MATVAC }; // END of enum materialName

class LaserMaterialsDefinition {
public:
  LaserMaterialsDefinition(LaserOpticalPropertiesMessenger *);
  ~LaserMaterialsDefinition();
  void CreateBorderSurface(G4String name, G4OpticalSurfaceModel model,
                           G4SurfaceType type, G4OpticalSurfaceFinish finish,
                           G4VPhysicalVolume *, G4VPhysicalVolume *);
  inline G4Material *GetAir() { return matAir; };
  inline G4Material *GetAl() { return matAl; };
  inline G4Material *GetVac() { return matVac; };
  inline G4Material *GetGlass() { return matGlass; };
  inline G4Material *GetWater() { return matWater; };

  LaserOpticalPropertiesMessenger *optics;
  LaserOpticalSurfaceMessenger *surface;
  void DefineMaterials();

private:
  G4NistManager *manager;

  // materials for the world
  G4Material *matAir;
  G4Material *matAl;
  G4Material *matVac;
  G4Material *matGlass;
  G4Material *matWater;

}; // END of class MaterialsDefinition

#endif
