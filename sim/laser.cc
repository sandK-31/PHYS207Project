#include "G4ios.hh"
#include <cstdlib>
#include <fstream>
#include <math.h>

// package includes
//
#include "G4RunManager.hh"
#include "LaserDetectorConstruction.hh"
#include "LaserPhysicsList.hh"
#include "LaserPrimaryGeneratorAction.hh"
// #include "PrimaryGeneratorActionGPS.hh"
#include "LaserActionInitialization.hh"
#include "LaserParticleGunMessenger.hh"
// geant4 includes
#include "G4RunManagerFactory.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4UItcsh.hh"
#include "G4UIterminal.hh"
#include "G4VisExecutive.hh"
#include "G4ios.hh"

#include <fstream>
#include <string>
#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif
// Physics lists
#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhysics.hh"
#include "G4VModularPhysicsList.hh"
#include "QGSP_BERT.hh"
#include "QGSP_BERT_HP.hh"
#include "QGSP_BIC_HP.hh"
#include "QGSP_INCLXX_HP.hh"

#include "TRandom3.h"

// run ./main -help to see input commands
// (0) executable
// (1) macro
void help();

namespace {
void PrintUsage() {
  G4cerr << " Usage: " << G4endl;
  G4cerr << " laser [-r macro ] [-u UIsession] [-t nThreads] [-s SpecMacro]"
         << G4endl;
  G4cerr << " [-a RandSeed1] [-b RandSeed2] [-o fileName] [-n fileNumber]"
         << G4endl;
  G4cerr << "   note: -t option is available only for multi-threaded mode."
         << G4endl;
}
} // namespace

int numEventsRecorded = 0;

//------------------------------------------------------------------------------
int main(int argc, char **argv) {

  auto t_start = std::chrono::high_resolution_clock::now();

  // Choose the Random engine
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  // Construct the default run manager
  // for single processor
  // G4RunManager* runManager = new G4RunManager;
  G4int nThreads = 4;
  G4String macro;
  G4String specmac;
  G4String session;
  G4int fileNumber = -1;
  G4String fileName;
  G4UIExecutive *ui = 0;

  long seeds[2];

  TRandom3 rootRandom;

  // Optionally, seed the generator (e.g., with system time)
  rootRandom.SetSeed(0); // 0 uses a time-based seed

  // Generate two random seeds
  seeds[0] = rootRandom.Integer(1000000000);
  seeds[1] = rootRandom.Integer(1000000000);
  for (G4int i = 1; i < argc; i = i + 2) {
    if (G4String(argv[i]) == "-r")
      macro = argv[i + 1];
    else if (G4String(argv[i]) == "-s")
      specmac = argv[i + 1]; // SPECS for geometry
    else if (G4String(argv[i]) == "-u")
      session = argv[i + 1]; // can use different gui's dont use this just use
                             // qt, can visual more particles in x11
    else if (G4String(argv[i]) == "-n")
      fileNumber = G4UIcommand::ConvertToInt(argv[i + 1]);
    else if (G4String(argv[i]) == "-o")
      fileName = argv[i + 1];
    else if (G4String(argv[i]) == "-a")
      seeds[0] = (long)G4UIcommand::ConvertToInt(argv[i + 1]);
    else if (G4String(argv[i]) == "-b")
      seeds[1] = (long)G4UIcommand::ConvertToInt(argv[i + 1]);
#ifdef G4MULTITHREADED
    else if (G4String(argv[i]) == "-t") {
      nThreads = G4UIcommand::ConvertToInt(argv[i + 1]);
    }
#endif
    else {
      PrintUsage();
      return 1;
    }
  }
  G4Random::setTheSeeds(seeds);

#ifdef G4MULTITHREADED
  auto *runManager = G4RunManagerFactory::CreateRunManager();

  // Set number of threads if applicable
  if (nThreads > 0) {
    runManager->SetNumberOfThreads(nThreads);
  }

  // Check thread IDs
#else
  G4RunManager *runManager = new G4RunManager;
#endif
  // Set mandatory initialization classes
  // Detector construction

  auto opProps = new LaserOpticalPropertiesMessenger();
  auto surf = new LaserOpticalSurfaceMessenger();
  auto gunProps = new LaserParticleGunMessenger();
  G4UImanager *UImanager = G4UImanager::GetUIpointer();
  G4UImanager::GetUIpointer()->ApplyCommand("/tracking/verbose 0");

  if (specmac.size()) {
    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command + specmac);
  }

  std::ifstream file(specmac);
  if (!file.is_open()) {
    G4cerr << "Error: Cannot open file " << specmac << G4endl;
    return 1;
  }

  std::string line;
  std::string specContent;

  while (std::getline(file, line)) {
    specContent += line + "\n";
  }
  file.close();

  std::string macroContent;
  if (macro.size()) {
    std::ifstream file(macro);
    std::string line;
    if (!file.is_open()) {
      G4cerr << "Error: Cannot open file " << macro << G4endl;
      return 1;
    }

    while (std::getline(file, line)) {
      macroContent += line + "\n";
    }
    file.close();
  }

  //////////////////////
  auto dc = new LaserDetectorConstruction(opProps, surf);

  runManager->SetUserInitialization(dc);
  // Physics list

  LaserPhysicsList *physicsList = new LaserPhysicsList();
  runManager->SetUserInitialization(physicsList);
  // User action initialization

  runManager->SetUserInitialization(
      new LaserActionInitialization(fileName, fileNumber, seeds[0], seeds[1],
                                    gunProps, macroContent, specContent));

  runManager->Initialize();

#ifdef G4VIS_USE
  // Initialize visualization
  G4VisManager *visManager = new G4VisExecutive;
  // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
  // G4VisManager* visManager = new G4VisExecutive("Quiet");
  visManager->Initialize();
#endif

  // Get the pointer to the User Interface manager
  // G4UImanager* UImanager = G4UImanager::GetUIpointer();
  auto t_end1 = std::chrono::high_resolution_clock::now();
  G4cout << "Wall clock time passed: ";
  G4cout
      << std::chrono::duration<double, std::milli>(t_end1 - t_start).count() *
             0.001
      << G4endl;
  if (macro.size()) {
    // batch mode

    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command + macro);
  } else {
    ui = new G4UIExecutive(argc, argv);

    G4VisManager *visManager = new G4VisExecutive();
    visManager->Initialize();

    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    if (ui) {
      UImanager->ApplyCommand("/control/execute macros/vis.mac");
      ui->SessionStart();
      delete ui;
    } else {
      G4String command = "/control/execute ";
      G4String fileName = argv[1]; // see if this is correct******
      UImanager->ApplyCommand(command + fileName); // use this!!!!!!!
    }
  }
  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted
  // in the main() program !
#ifdef G4VIS_USE
  delete visManager;
#endif
  delete runManager;
  // t2 = std::clock();
  // float diff = ((float)t2-(float)t1);
  // diff /= CLOCKS_PER_SEC;
  // G4cout << diff << " s" << G4endl;
  auto t_end = std::chrono::high_resolution_clock::now();
  G4cout << "Wall clock time passed: ";
  G4cout << std::chrono::duration<double, std::milli>(t_end - t_start).count() *
                0.001
         << G4endl;

  G4cout << G4endl;
  G4cout << "No-Pro Pro Industries LLC" << G4endl;
  G4cout << "All Rights Reserved" << G4endl;
  G4cout << "Glow Blue" << G4endl;
  return 0;
}

void help() { cout << "-h for help" << endl; }
