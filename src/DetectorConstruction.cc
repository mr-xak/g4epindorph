#include "DetectorConstruction.hh"

// Geant4 includes
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4TwoVector.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Cons.hh"
#include "G4Trd.hh"
#include "G4Orb.hh"
#include "G4GenericTrap.hh"
#include "G4SubtractionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4EllipticalTube.hh"
#include "G4UnionSolid.hh"
#include "G4AssemblyVolume.hh"

#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"

#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4ProductionCuts.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4PVParameterised.hh"
#include "G4VisAttributes.hh" 

#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4RunManager.hh"
#include "G4GeometryManager.hh"
#include "G4GeometryTolerance.hh"
#include "G4IStore.hh"

// boost includes
#include "boost/lexical_cast.hpp"
#include <boost/algorithm/string.hpp>

// NPLibrary
#include "NPParamReader.hh"

// App
#include "DetectorMaterials.hh"
#include "DetectorSD.hh"


#include <map>
typedef std::map<std::vector<G4int>, G4String> ltP;
typedef std::map<std::vector<G4int>, G4double> ltD;
typedef std::map<G4int, G4String> atP;
typedef std::map<G4int, G4Material*> adP;
typedef std::vector<G4Material*> vtP;


DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction()
{
  G4cerr << "DetectronConstruction main constructor" << G4endl;
  //pMaterial = new DetectorMaterials();
  pMaterial = DetectorMaterials::GetInstance();
}

DetectorConstruction::~DetectorConstruction()
{
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{

  NPLibrary::NPParamReaderMain* cmdParam = NPLibrary::NPParamReaderMain::Instance();
  G4NistManager* nistMan = G4NistManager::Instance();
  G4Material* Galactic = nistMan->FindOrBuildMaterial("G4_Galactic");
  G4Material* worldMaterial = pMaterial->getMaterialIfDefined("worldMaterial", Galactic);
  G4double worldXLength = cmdParam->getPriorityParamAsG4Double("WorldXLength", "60") * cm;
  G4double worldYLength = cmdParam->getPriorityParamAsG4Double("WorldYLength", "60") * cm;
  G4double worldZLength = cmdParam->getPriorityParamAsG4Double("WorldZLength", "60") * cm;

  G4Box* worldBox = new G4Box("world", 0.5 * worldXLength, 0.5 * worldYLength, 0.5 * worldZLength);
  G4LogicalVolume* worldBoxLV = new G4LogicalVolume(worldBox, worldMaterial, "worldLV");
  G4VPhysicalVolume* worldBoxPV = new G4PVPlacement(0, G4ThreeVector(), worldBoxLV, "worldPV", 0, false, 0);


  G4RotationMatrix* rot1 = new G4RotationMatrix();
  rot1->rotateX(-90*deg);

  G4Tubs* tube1 = new G4Tubs("outterShell1", 0.0, 0.5 * 16.5 * mm, 0.5 * 30 * mm, 360 * deg, 360 * deg);
  G4Cons* cons1 = new G4Cons("outterShell2", 0.0, 0.5 * 16.5 * mm, 0.0, 0.5 * 4 * mm, 0.5 * 21 * mm, 360 * deg, 360 * deg);
  G4UnionSolid *tc1 = new G4UnionSolid("outterShellJoined", tube1, cons1, new G4RotationMatrix(), G4ThreeVector(0, 0, 0.5 * (30 + 21) * mm));
  G4LogicalVolume* tube1LV = new G4LogicalVolume(tc1, nistMan->FindOrBuildMaterial("G4_POLYSTYRENE"), "outterShell1LV");
  new G4PVPlacement(rot1, G4ThreeVector(0, 0, 0), tube1LV, "outterShell1PV", worldBoxLV, false, 0, true);


  G4Tubs* tube2 = new G4Tubs("innerShell1", 0.0, 0.5 * 14.3 * mm, 0.5 * 30 * mm, 360 * deg, 360 * deg);
  G4Cons* cons2 = new G4Cons("innerShell2", 0.0, 0.5 * 14.3 * mm, 0.0, 0.5 * 3.7 * mm, 0.5 * 19 * mm, 360 * deg, 360 * deg);
  G4UnionSolid* tc2 = new G4UnionSolid("innerShellJoined", tube2, cons2, new G4RotationMatrix(), G4ThreeVector(0, 0, 0.5 * (30 + 19) * mm));
  G4LogicalVolume* tube2LV = new G4LogicalVolume(tc2, nistMan->FindOrBuildMaterial("G4_WATER"), "innerShell1LV");
  //new G4PVPlacement(rot1, G4ThreeVector(0, 0, 0), tube2LV, "innerShell1PV", tube1LV, false, 0, true);
  new G4PVPlacement(new G4RotationMatrix(), G4ThreeVector(0, 0, 0), tube2LV, "innerShell1PV", tube1LV, false, 0, true);

  G4cout << "innerShell Volume: " << tc2->GetCubicVolume() / cm3 << " cm3 " << G4endl;
  G4cout << "1 MeV in inner shell is " << (1 * MeV / tube2LV->GetMass()) / CLHEP::gray << " Gy" << G4endl;

  sensLV = tube2LV;

  G4Box* moder1 = new G4Box("moder1", 0.5 * 15 * cm, 0.5 * 15 * cm, 0.5 * 2 * mm);
  G4LogicalVolume *moder1LV = new G4LogicalVolume(moder1, nistMan->FindOrBuildMaterial("G4_PLEXIGLASS"), "moder1LV");
  //G4LogicalVolume* moder1LV = new G4LogicalVolume(moder1, nistMan->FindOrBuildMaterial("G4_AIR"), "moder1LV");
  new G4PVPlacement(0, G4ThreeVector(0, 0, -2*cm), moder1LV, "moder1PV", worldBoxLV, false, 0, true);

  G4Region* regEpi = new G4Region("regEpi");
  setCutsForRegionFromCMDString(regEpi);
  tube1LV->SetRegion(regEpi);
  tube2LV->SetRegion(regEpi);
  regEpi->AddRootLogicalVolume(tube1LV);

  G4cout << "DetectorConstruction completed" << G4endl;
  return worldBoxPV;
}


void DetectorConstruction::ConstructSDandField()
{

  G4cout << "ConstructSDandField called" << G4endl;

  G4SDManager* sdMan = G4SDManager::GetSDMpointer();
  NPLibrary::NPParamReaderMain* cmdParam = NPLibrary::NPParamReaderMain::Instance();

  DetectorSD* det = new DetectorSD("main");
  sensLV->SetSensitiveDetector(det);
  sdMan->AddNewDetector(det);
}

void DetectorConstruction::setCutsForRegionFromCMDString(G4Region* targetRegion, G4String regionName) {
  // from former NP Extra
  NPLibrary::NPParamReaderMain* cmdParam = NPLibrary::NPParamReaderMain::Instance();
  G4String regionCuts = cmdParam->getPriorityParamAsG4String(G4String("useCut4" + regionName), "-1");
  if ("-1" == regionCuts) {
    regionCuts = cmdParam->getPriorityParamAsG4String("WorldCuts", "700,700,700,700");
  }
  std::vector<G4double> cutsData = cmdParam->splitString2G4double(regionCuts, ",");
  G4ProductionCuts* pCut = new G4ProductionCuts(); // @todo: UserSpecialCuts ??
  pCut->SetProductionCut(cutsData[0] * um, "gamma");
  pCut->SetProductionCut(cutsData[1] * um, "e-");
  pCut->SetProductionCut(cutsData[2] * um, "e+");
  pCut->SetProductionCut(cutsData[3] * um, "proton");
  targetRegion->SetProductionCuts(pCut);

}

void DetectorConstruction::setCutsForRegionFromCMDString(G4Region* tgr)
{
  setCutsForRegionFromCMDString(tgr, tgr->GetName());
}
