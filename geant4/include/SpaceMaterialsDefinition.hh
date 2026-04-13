#ifndef SpaceMaterialsDefinition_H
#define SpaceMaterialsDefinition_H 1

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

#include "G4LogicalBorderSurface.hh"

#include "SpaceOpticalPropertiesMessenger.hh"
class SpaceOpticalPropertiesMessenger;
enum materialName { MATAIR, MATGLASS, MATVAC }; // END of enum materialName

class SpaceMaterialsDefinition {
public:
  SpaceMaterialsDefinition(SpaceOpticalPropertiesMessenger *);
  ~SpaceMaterialsDefinition();
  inline G4Material *GetAir() { return matAir; };
  inline G4Material *GetWater() { return matWater; };
  inline G4Material *GetVacuum() { return matVacuum; };

  SpaceOpticalPropertiesMessenger *optics;
  void DefineMaterials();

private:
  G4NistManager *manager;

  // materials for the world
  G4Material *matAir;
  G4Material *matWater;
  G4Material *matVacuum;

}; // END of class MaterialsDefinition

#endif
