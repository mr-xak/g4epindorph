#include "ActionInitialization.hh"
#include "RunAction.hh"
//#include "PGAPhaseSpace.hh"
#include "PGAGPS.hh"
#include "PGA.hh"
//#include "PGAProtobuf.hh"
#include "SteppingAction.hh"

#include "NPParamReader.hh"
#include "NPProtobufPhaseSpace.cc"

ActionInitialization::ActionInitialization(DetectorConstruction* fdet) : G4VUserActionInitialization(), fDetector(fdet), masterRunAction(nullptr)
{
  //masterRunAction = new RunAction();
}


ActionInitialization::~ActionInitialization()
{}


void ActionInitialization::Build() const
{
  G4cout << "AI Build called" << G4endl;

  //SetUserAction(run);
  NPLibrary::NPParamReaderMain* cmdParam = NPLibrary::NPParamReaderMain::Instance();
  G4String PGAMode = cmdParam->getPriorityParamAsG4String("PGAMode", "gps");
  if ("gps" == PGAMode) {
    PGAGPS* prim = new PGAGPS();
    SetUserAction(prim);
  }
  else if ("protobuf" == PGAMode) {
    PGAProtobuf* prim = new PGAProtobuf();
    SetUserAction(prim);
  }
  else {

  }
  //else {
  //  PrimaryGeneratorAction *prim = new PrimaryGeneratorAction();
  //  SetUserAction(prim);
  //}



  SetUserAction(new SteppingAction());

  //SetUserAction(new StackingAction());

  SetUserAction(new RunAction());
}

void ActionInitialization::BuildForMaster() const
{

  G4cout << "AI BuildForMaster called" << G4endl;
#ifdef G4MULTITHREADED
  SetUserAction(new RunAction());
#endif
}