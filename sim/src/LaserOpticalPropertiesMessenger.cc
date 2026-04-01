#include "LaserOpticalPropertiesMessenger.hh"

LaserOpticalPropertiesMessenger::LaserOpticalPropertiesMessenger() {

  fMessenger =
      new G4GenericMessenger(this, "/opticprops/", "Optical Properties");

  fMessenger->DeclareProperty("IntralipidRayleighScatlength",
                              intralipidRayleighScatlengthHolder,
                              "Intralipid Rayleigh Scatlength");
  fMessenger->DeclareProperty("IntralipidAbslength", intralipidAbslengthHolder,
                              "Intralipid Abslength");
  fMessenger->DeclareProperty("IntralipidRindex", intralipidRindexHolder,
                              "Intralipid Rindex");
  fMessenger->DeclareProperty("IntralipidMieScatlength",
                              intralipidMieScatlengthHolder,
                              "Intralipid Mie Scatlength");
  fMessenger->DeclareProperty("IntralipidMieForwardG",
                              intralipidMieForwardGHolder,
                              "Intralipid Mie ForwardG");
  fMessenger->DeclareProperty("IntralipidMieBackwardG",
                              intralipidMieBackwardGHolder,
                              "Intralipid Mie BackwardG");
  fMessenger->DeclareProperty("IntralipidMieFBRatioG",
                              intralipidMieFBRatioGHolder,
                              "Intralipid Mie FBRatioG");
}

LaserOpticalPropertiesMessenger::~LaserOpticalPropertiesMessenger() {}

void LaserOpticalPropertiesMessenger::OpticsUpdate() {
  if (intralipidMieScatlengthHolder == 0.0 and
      intralipidRayleighScatlengthHolder == 0.0) {
    throw std::invalid_argument(
        "No Scattering Length Values Provided: OpPropsMessenger");
  }

  if (intralipidRayleighScatlengthHolder != 0.0) {
    intralipidRayleighScatlength = intralipidRayleighScatlengthHolder;
  } else {
    intralipidRayleighScatlength = -1;
    G4cout << "WARNING: No Rayleigh Scattering Length Provided, Using Mie "
              "Scattering"
           << G4endl;
  }

  if (intralipidRindexHolder != 0.0) {
    intralipidRindex = intralipidRindexHolder;
  } else {
    throw std::invalid_argument(
        "No Value For Intralipid Refractive Iindex: OpPropsMessenger");
  }

  if (intralipidAbslengthHolder != 0.0) {
    intralipidAbslength = intralipidAbslengthHolder;
  } else {
    throw std::invalid_argument(
        "No Value For Intralipid Absorption Length: OpPropsMessenger");
  }

  if (intralipidMieScatlengthHolder != 0.0) {
    intralipidMieScatlength = intralipidMieScatlengthHolder;
  } else {
    intralipidMieScatlength = -1;
    G4cout << "WARNING: No Mie Scattering Length Provided, Using Rayleign "
              "Scattering"
           << G4endl;
  }

  if (intralipidMieForwardGHolder != 0.0) {
    intralipidMieForwardG = intralipidMieForwardGHolder;
  } else {
    intralipidMieForwardG = -1;
    G4cout << "WARNING: No Mie Forward G Provided, Using Rayleigh Scattering"
           << G4endl;
  }

  if (intralipidMieBackwardGHolder != 0.0) {
    intralipidMieBackwardG = intralipidMieBackwardGHolder;
  } else {
    intralipidMieBackwardG = -1;
    G4cout << "WARNING: No Mie Backward G Provided, Using Rayleigh Scattering"
           << G4endl;
  }

  if (intralipidMieFBRatioGHolder != 0.0) {
    intralipidMieFBRatioG = intralipidMieFBRatioGHolder;
  } else {
    intralipidMieFBRatioG = -1;
    G4cout << "WARNING: No Mie Forward-Backward Ratio G Provided, Using "
              "Rayleigh Scattering"
           << G4endl;
  }
}
