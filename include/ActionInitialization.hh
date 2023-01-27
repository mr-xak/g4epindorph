#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "G4String.hh"
#include "DetectorConstruction.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class RunAction;
class DetectorConstruction;
class TrackingAction;
class SteppingAction;
class StackingAction;

class ActionInitialization : public G4VUserActionInitialization
{
public:

  ActionInitialization(DetectorConstruction*);

  virtual ~ActionInitialization();

  virtual void Build() const;

  virtual void BuildForMaster() const;

private:
  DetectorConstruction* fDetector;
  RunAction* masterRunAction;
};

#endif