#ifndef DetectorSD_hh
#define DetectorSD_hh 1

#include "G4VSensitiveDetector.hh"
#include <map>
#include <vector>


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
  G4double dxLetSum;
  G4long qu;
  G4long qu2;

  G4double pEneSum;
  G4double dEneSum;
  G4double tEneSum;
  G4double HEneSum;
  G4double alphaEneSum;
  G4double HeEneSum;
  G4double LiEneSum;
  G4double BeEneSum;
  G4double BEneSum;
  G4double CEneSum;
  G4double NEneSum;
  G4double OEneSum;

  G4double pLetBase2;
  G4double alphaLetBase2;
  G4double CLetBase2;
  G4double NLetBase2;
  G4double OLetBase2;
};

#endif