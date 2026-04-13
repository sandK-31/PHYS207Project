#include "G4ios.hh"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

// package includes
#include "SpaceActionInitialization.hh"
#include "SpaceDetectorConstruction.hh"
#include "SpacePhysicsList.hh"

// geant4 includes
#include "G4RunManagerFactory.hh"
#include "G4UIExecutive.hh"
#include "G4UIcommand.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "TRandom3.h"

void help();

namespace {
void PrintUsage() {
  G4cerr << "Usage:\n";
  G4cerr << "  space [-r macro] [-u UIsession] [-t nThreads] [-s SpecMacro]\n";
  G4cerr << "        [-a RandSeed1] [-b RandSeed2] [-o fileName] [-n "
            "fileNumber]\n";
  G4cerr << "  note: -t option is available only for multi-threaded mode.\n";
}
} // namespace

int main(int argc, char **argv) {
  auto t_start = std::chrono::high_resolution_clock::now();

  G4Random::setTheEngine(new CLHEP::RanecuEngine);

  G4int nThreads = 4;
  G4String macro;
  G4String specmac;
  G4String session;
  G4int fileNumber = -1;
  G4String fileName;

  long seeds[2];

  TRandom3 rootRandom;
  rootRandom.SetSeed(0);
  seeds[0] = rootRandom.Integer(1000000000);
  seeds[1] = rootRandom.Integer(1000000000);

  for (G4int i = 1; i < argc; i += 2) {
    if (i + 1 >= argc) {
      PrintUsage();
      return 1;
    }

    if (G4String(argv[i]) == "-r")
      macro = argv[i + 1];
    else if (G4String(argv[i]) == "-s")
      specmac = argv[i + 1];
    else if (G4String(argv[i]) == "-u")
      session = argv[i + 1];
    else if (G4String(argv[i]) == "-n")
      fileNumber = G4UIcommand::ConvertToInt(argv[i + 1]);
    else if (G4String(argv[i]) == "-o")
      fileName = argv[i + 1];
    else if (G4String(argv[i]) == "-a")
      seeds[0] = static_cast<long>(G4UIcommand::ConvertToInt(argv[i + 1]));
    else if (G4String(argv[i]) == "-b")
      seeds[1] = static_cast<long>(G4UIcommand::ConvertToInt(argv[i + 1]));
#ifdef G4MULTITHREADED
    else if (G4String(argv[i]) == "-t")
      nThreads = G4UIcommand::ConvertToInt(argv[i + 1]);
#endif
    else {
      PrintUsage();
      return 1;
    }
  }

  G4Random::setTheSeeds(seeds);

#ifdef G4MULTITHREADED
  auto *runManager = G4RunManagerFactory::CreateRunManager();
  if (nThreads > 0) {
    runManager->SetNumberOfThreads(nThreads);
  }
#else
  auto *runManager = new G4RunManager;
#endif

  auto *opProps = new SpaceOpticalPropertiesMessenger();
  auto *UImanager = G4UImanager::GetUIpointer();
  UImanager->ApplyCommand("/tracking/verbose 0");

  std::string specContent;
  if (!specmac.empty()) {
    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command + specmac);

    std::ifstream file(specmac);
    if (!file.is_open()) {
      G4cerr << "Error: Cannot open file " << specmac << G4endl;
      delete runManager;
      delete opProps;
      return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
      specContent += line + "\n";
    }
  }

  std::string macroContent;
  if (!macro.empty()) {
    std::ifstream file(macro);
    if (!file.is_open()) {
      G4cerr << "Error: Cannot open file " << macro << G4endl;
      delete runManager;
      delete opProps;
      return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
      macroContent += line + "\n";
    }
  }

  runManager->SetUserInitialization(new SpaceDetectorConstruction(opProps));
  runManager->SetUserInitialization(new SpacePhysicsList());
  runManager->SetUserInitialization(new SpaceActionInitialization(
      fileName, fileNumber, seeds[0], seeds[1], macroContent, specContent));

  runManager->Initialize();

  auto t_end1 = std::chrono::high_resolution_clock::now();
  G4cout << "Wall clock time passed: "
         << std::chrono::duration<double>(t_end1 - t_start).count() << G4endl;

  G4VisManager *visManager = nullptr;

  if (!macro.empty()) {
    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command + macro);
  } else {
    visManager = new G4VisExecutive();
    visManager->Initialize();

    G4UIExecutive *ui = nullptr;
    if (!session.empty()) {
      ui = new G4UIExecutive(argc, argv, session);
    } else {
      ui = new G4UIExecutive(argc, argv);
    }

    UImanager->ApplyCommand("/control/execute macros/vis.mac");
    ui->SessionStart();
    delete ui;
  }

  delete visManager;
  delete runManager;
  delete opProps;

  auto t_end = std::chrono::high_resolution_clock::now();
  G4cout << "Wall clock time passed: "
         << std::chrono::duration<double>(t_end - t_start).count() << G4endl;

  G4cout << "\nNo-Pro Pro Industries LLC\n";
  G4cout << "All Rights Reserved\n";
  G4cout << "Glow Blue\n";

  return 0;
}

void help() { std::cout << "-h for help" << std::endl; }
