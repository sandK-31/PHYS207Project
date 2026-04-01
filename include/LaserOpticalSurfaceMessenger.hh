#ifndef LaserOpticalSurfaceMessenger_h
#define LaserOpticalSurfaceMessenger_h 1

#include "G4GenericMessenger.hh"
#include "G4Material.hh"
#include "G4SurfaceProperty.hh"
#include "globals.hh"
#include <G4OpticalSurface.hh>
#include <iostream>
#include <string>
#include <vector>

class LaserOpticalSurfaceMessenger {
public:
  LaserOpticalSurfaceMessenger();
  ~LaserOpticalSurfaceMessenger();
  inline G4OpticalSurfaceFinish WaterAirFinish() {
    return WaterAirFinishHolder;
  };
  inline G4OpticalSurfaceFinish AirGlassFinish() {
    return AirGlassFinishHolder;
  };
  inline G4OpticalSurfaceFinish GlassWaterFinish() {
    return GlassWaterFinishHolder;
  };
  inline G4OpticalSurfaceFinish GlassWorldFinish() {
    return GlassWorldFinishHolder;
  };

  inline G4SurfaceType WaterAirType() { return WaterAirTypeHolder; };
  inline G4SurfaceType AirGlassType() { return AirGlassTypeHolder; };
  inline G4SurfaceType GlassWaterType() { return GlassWaterTypeHolder; };
  inline G4SurfaceType GlassWorldType() { return GlassWorldTypeHolder; };

  void TypeUpdate();
  void FinishUpdate();
  int OptionsType(G4String);
  int OptionsFinish(G4String);

private:
  G4SurfaceType WaterAirTypeHolder;
  G4SurfaceType AirGlassTypeHolder;
  G4SurfaceType GlassWaterTypeHolder;
  G4SurfaceType GlassWorldTypeHolder;

  G4OpticalSurfaceFinish WaterAirFinishHolder;
  G4OpticalSurfaceFinish AirGlassFinishHolder;
  G4OpticalSurfaceFinish GlassWaterFinishHolder;
  G4OpticalSurfaceFinish GlassWorldFinishHolder;

  G4String WaterAirf;
  G4String AirGlassf;
  G4String GlassWaterf;
  G4String GlassWorldf;

  G4String WaterAirt;
  G4String AirGlasst;
  G4String GlassWatert;
  G4String GlassWorldt;

  G4GenericMessenger *finishMessenger;
  G4GenericMessenger *typeMessenger;
};

#endif
