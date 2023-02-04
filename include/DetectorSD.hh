#ifndef DetectorSD_hh
#define DetectorSD_hh 1

#include "G4VSensitiveDetector.hh"
#include <map>
#include <vector>
#include "NPFastLayersData.hh"


class G4Step;
class RunAction;

class DetectorSD : public G4VSensitiveDetector
{
public:
  DetectorSD(G4String);
  ~DetectorSD();
  void Initialize(G4HCofThisEvent*);
  G4bool ProcessHits(G4Step*, G4TouchableHistory*);
  void EndOfEvent(G4HCofThisEvent*);
private:
  G4String ownName;


  G4double eneSum;
  G4double letBase1;
  G4double letBase2;
  G4double let2Avg;
  G4double dxSum;
  G4long qu;
  G4long qu2;
};

#endif