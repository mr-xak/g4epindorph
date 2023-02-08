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
#include "NPFastLayersData.hh"
#include "NPCad2Geant.hh"

// App
#include "DetectorMaterials.hh"
#include "DetectorSD.hh"

using namespace NPLibrary;
using namespace NPLibrary::NPFastLayerData;


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
  G4Material* air = nistMan->FindOrBuildMaterial("G4_AIR");
  G4Material* worldMaterial = pMaterial->getMaterialIfDefined("worldMaterial", Galactic);
  G4double worldXLength = cmdParam->getPriorityParamAsG4Double("WorldXLength", "60") * cm;
  G4double worldYLength = cmdParam->getPriorityParamAsG4Double("WorldYLength", "60") * cm;
  G4double worldZLength = cmdParam->getPriorityParamAsG4Double("WorldZLength", "60") * cm;

  G4Box* worldBox = new G4Box("world", 0.5 * worldXLength, 0.5 * worldYLength, 0.5 * worldZLength);
  G4LogicalVolume* worldBoxLV = new G4LogicalVolume(worldBox, worldMaterial, "worldLV");
  G4VPhysicalVolume* worldBoxPV = new G4PVPlacement(0, G4ThreeVector(), worldBoxLV, "worldPV", 0, false, 0);


  G4RotationMatrix* rot1 = new G4RotationMatrix();
  rot1->rotateX(90*deg);

  G4ThreeVector epiPos = cmdParam->getPriorityParamAsG4ThreeVector("epindorphPosition");

  G4bool epiInWorld = cmdParam->getPriorityParamAsCppInt("epindorphInWorld", "1");

  G4Tubs* tube1 = new G4Tubs("outterShell1", 0.0, 0.5 * 16.5 * mm, 0.5 * 30 * mm, 360 * deg, 360 * deg);
  G4Cons* cons1 = new G4Cons("outterShell2", 0.0, 0.5 * 16.5 * mm, 0.0, 0.5 * 4 * mm, 0.5 * 21 * mm, 360 * deg, 360 * deg);
  G4UnionSolid *tc1 = new G4UnionSolid("outterShellJoined", tube1, cons1, new G4RotationMatrix(), G4ThreeVector(0, 0, 0.5 * (30 + 21) * mm));
  G4LogicalVolume* tube1LV = new G4LogicalVolume(tc1, nistMan->FindOrBuildMaterial("G4_POLYSTYRENE"), "outterShell1LV");

  if (epiInWorld) {
    new G4PVPlacement(rot1, epiPos, tube1LV, "outterShell1PV", worldBoxLV, false, 0, true);
  }
  else {
    G4ThreeVector outterEpiBoxSizes = cmdParam->getPriorityParamAsG4ThreeVector("outterEpiBoxSizes", G4ThreeVector(16.5*mm, 16.5*mm, 51*mm));
    G4ThreeVector outterEpiBoxPosition = cmdParam->getPriorityParamAsG4ThreeVector("outterEpiBoxPosition");
    G4Box* outterEpiBox = new G4Box("outterEpiBox", 0.5 * outterEpiBoxSizes.x(), 0.5 * outterEpiBoxSizes.y(), 0.5 * outterEpiBoxSizes.z());
    G4Material* outterEpiBoxMaterial = pMaterial->getMaterialIfDefined("outterEpiBoxMaterial", air);
    G4LogicalVolume* outterEpiBoxLV = new G4LogicalVolume(outterEpiBox, outterEpiBoxMaterial, "outterEpiBoxLV");
    new G4PVPlacement(rot1, outterEpiBoxPosition, outterEpiBoxLV, "outterEpiBoxPV", worldBoxLV, false, 0, true);
    new G4PVPlacement(new G4RotationMatrix(), epiPos, tube1LV, "outterShell1PV", outterEpiBoxLV, false, 0, true);
  }


  G4Tubs* tube2 = new G4Tubs("innerShell1", 0.0, 0.5 * 14.3 * mm, 0.5 * 30 * mm, 360 * deg, 360 * deg);
  G4Cons* cons2 = new G4Cons("innerShell2", 0.0, 0.5 * 14.3 * mm, 0.0, 0.5 * 3.7 * mm, 0.5 * 19 * mm, 360 * deg, 360 * deg);
  G4UnionSolid* tc2 = new G4UnionSolid("innerShellJoined", tube2, cons2, new G4RotationMatrix(), G4ThreeVector(0, 0, 0.5 * (30 + 19) * mm));
  G4LogicalVolume* tube2LV = new G4LogicalVolume(tc2, nistMan->FindOrBuildMaterial("G4_WATER"), "innerShell1LV");
  //new G4PVPlacement(rot1, G4ThreeVector(0, 0, 0), tube2LV, "innerShell1PV", tube1LV, false, 0, true);
  new G4PVPlacement(new G4RotationMatrix(), G4ThreeVector(0, 0, 0), tube2LV, "innerShell1PV", tube1LV, false, 0, true);

  G4cout << "innerShell Volume: " << tc2->GetCubicVolume() / cm3 << " cm3 " << G4endl;
  G4cout << "1 MeV in inner shell is " << (1 * MeV / tube2LV->GetMass()) / CLHEP::gray << " Gy" << G4endl;

  sensLV = tube2LV;

  G4bool useModerator = cmdParam->getPriorityParamAsCppInt("useModerator", "1");

  if (useModerator) {
    G4ThreeVector moderPos = cmdParam->getPriorityParamAsG4ThreeVector("moderatorPosition");
    G4double moderatorThin = cmdParam->getPriorityParamAsG4Double("moderatorThickness", "2");
    G4Box* moder1 = new G4Box("moder1", 0.5 * 15 * cm, 0.5 * 15 * cm, 0.5 * moderatorThin * mm);
    G4Material* moderatorMaterial = pMaterial->getMaterialIfDefined("moderatorMaterial", nistMan->FindOrBuildMaterial("G4_PLEXIGLASS"));
    G4LogicalVolume* moder1LV = new G4LogicalVolume(moder1, moderatorMaterial, "moder1LV");
    new G4PVPlacement(0, moderPos, moder1LV, "moder1PV", worldBoxLV, false, 0, true);
  }

  if ((bool)cmdParam->getPriorityParamAsCppInt("useStlObjectBinary", "0")) {
    G4String stlFilterPath = cmdParam->getPriorityParamAsG4String("stlFilterPath", "filter.stl");
    G4Material* stlObjectMaterial = pMaterial->getMaterialIfDefined("stlObjectMaterial", G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"));
    G4ThreeVector stlFilterPosition = cmdParam->getPriorityParamAsG4ThreeVector("stlFilterPosition", G4ThreeVector());
    G4RotationMatrix* stlFilterRotation = cmdParam->getPriorityParamAsG4RotationMatrixPointer("stlFilterRotation");
    G4double stlFilterScale = cmdParam->getPriorityParamAsG4Double("stlFilterScale", "0.0");
    NPLibrary::NPCad2Geant::NPSTL2Solid* stlMaker = new NPLibrary::NPCad2Geant::NPSTL2Solid();
    stlMaker->readBinaryStlFile(stlFilterPath);

    G4VSolid* resSolid = stlMaker->getSolid();
    G4cout << "cadSolidVolume, mm3: " << resSolid->GetCubicVolume() / mm3 << G4endl;

    G4LogicalVolume* resSolidLV = new G4LogicalVolume(resSolid, stlObjectMaterial, "resSolidLV");

    G4cout << "cadSolidMaterial: " << stlObjectMaterial->GetName() << "; cadSolidMass, g: " << resSolidLV->GetMass() / g << G4endl;

    new G4PVPlacement(stlFilterRotation, stlFilterPosition, resSolidLV, "resSolidPV", worldBoxLV, false, 0, true);
  }

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

  auto* vxFastData = NPLibrary::NPFastLayerData::NPFastVoxelMain::Instance();
  vxFastData->setVoxelData(1, 1, 1, (1 * MeV / sensLV->GetMass()) / CLHEP::gray);

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
