#include <ctime>
#if defined (WIN32)
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#else
#include <signal.h>
#endif

// Geant4 includes
#include "globals.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif
#include "G4Types.hh"
#include "G4StateManager.hh"

#include "G4RunManagerFactory.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4UImanager.hh"

#include "G4PhysListFactory.hh"
#include "G4EmStandardPhysics_option3.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4EmLivermorePhysics.hh"

#include "NPParamReader.hh"

int main(int argc, char** argv) {
  clock_t beginAt = clock();
  NPLibrary::NPParamReaderMain *cmdParam = NPLibrary::NPParamReaderMain::Instance();
  cmdParam->setVerbose(2);
  cmdParam->setCmdInput(argc, argv);
  cmdParam->fillAppJson();

  CLHEP::Ranlux64Engine defaultEngine(1234567, 4);
  G4Random::setTheEngine(&defaultEngine);
  int seedType = cmdParam->getRESeedType();
  G4long seed;
  if (1 == seedType) {
    seed = time(NULL);
    std::cout << "RE seed set to: " << seed << std::endl;
  }
  else if (0 == seedType) {
    seed = 1029384756;
  }
  else {
    seed = boost::lexical_cast<G4long>(cmdParam->getPriorityParamAsG4String("RESeedValue", "123654"));
  }
  G4Random::setTheSeed(seed);

  auto* runManager = G4RunManagerFactory::CreateRunManager();
  runManager->SetVerboseLevel(2);

#ifdef G4MULTITHREADED
  G4int nThreads = cmdParam->getPriorityParamAsCppInt("nThreads", "0");
  G4cout << "nThreads from cmdParam " << nThreads << G4endl;
  runManager->SetNumberOfThreads(nThreads);
#endif

  DetectorConstruction* fdet = new DetectorConstruction();
  runManager->SetUserInitialization(fdet);

  G4VUserPhysicsList* physicsList = nullptr;
  if (cmdParam->getIsBasePhysics()) {
    G4int standartPhysNum = cmdParam->getPhysNum();

    if (standartPhysNum == 6) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QGSP_INCLXX");
      physicsList = phys;
    }
    else if (standartPhysNum == 603) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QGSP_INCLXX");
      phys->ReplacePhysics(new G4EmStandardPhysics_option3());
      physicsList = phys;
    }
    else if (standartPhysNum == 604) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QGSP_INCLXX");
      phys->ReplacePhysics(new G4EmStandardPhysics_option4());
      physicsList = phys;
    }
    else if (standartPhysNum == 606) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QGSP_INCLXX");
      phys->ReplacePhysics(new G4EmLivermorePhysics());
      physicsList = phys;
    }
    else if (standartPhysNum == 2) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QBBC");
      physicsList = phys;
    }
    else if (standartPhysNum == 204) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QBBC");
      phys->ReplacePhysics(new G4EmStandardPhysics_option4());
      physicsList = phys;
    }
    else if (standartPhysNum == 206) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QBBC");
      phys->ReplacePhysics(new G4EmLivermorePhysics());
      physicsList = phys;
    }
    else if (standartPhysNum == 911) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QGSP_BIC_AllHP");
      //phys->ReplacePhysics(new G4EmStandardPhysics_option4());
      physicsList = phys;
    }
    else if (standartPhysNum == 9113) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QGSP_BIC_AllHP");
      phys->ReplacePhysics(new G4EmStandardPhysics_option3());
      physicsList = phys;
    }
    else if (standartPhysNum == 9114) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QGSP_BIC_AllHP");
      phys->ReplacePhysics(new G4EmStandardPhysics_option4());
      physicsList = phys;
    }
    else if (standartPhysNum == 9116) {
      G4PhysListFactory factory;
      G4VModularPhysicsList* phys = nullptr;
      phys = factory.GetReferencePhysList("QGSP_BIC_AllHP");
      phys->ReplacePhysics(new G4EmLivermorePhysics());
      physicsList = phys;
    }
  } else {
//    physicsList = new PhysicsList();
  }

  runManager->SetUserInitialization(physicsList);  


  ActionInitialization* AI = new ActionInitialization(fdet);
  runManager->SetUserInitialization(AI);


  runManager->Initialize();

  if (cmdParam->isVisUse()) {
    G4UIExecutive* ui = new G4UIExecutive(argc, argv);
    auto visManager = new G4VisExecutive;
    visManager->Initialize();
    auto UImanager = G4UImanager::GetUIpointer();
    G4String command = "/control/execute ";
    G4String fileName = cmdParam->getInputMacro();
    UImanager->ApplyCommand(command + fileName);
    ui->SessionStart();
  }
  else {
    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    G4String command = "/control/execute ";
    G4String fileName = cmdParam->getInputMacro();
    UImanager->ApplyCommand(command + fileName);
  }


  //delete visManager;
  //delete runManager;

  G4cout << "Total time: " << (clock() - beginAt) / CLOCKS_PER_SEC << G4endl;
  return 0;
}