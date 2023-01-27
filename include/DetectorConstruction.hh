#ifndef DetectorConstruction_H
#define DetectorConstruction_H 1

#include "G4VUserDetectorConstruction.hh"
#include "DetectorMaterials.hh"
#include <vector>
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"


typedef std::vector<G4Material*> vtP;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  ~DetectorConstruction();

  G4VPhysicalVolume* Construct();
  void ConstructSDandField();

  void setCutsForRegionFromCMDString(G4Region*, G4String);
  void setCutsForRegionFromCMDString(G4Region*);
private:
  DetectorMaterials* pMaterial;
  G4LogicalVolume* sensLV;
};

#endif