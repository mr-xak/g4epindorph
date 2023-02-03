#ifndef NPCad2Geant_hh
#define NPCad2Geant_hh

// std
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

// boost
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"

// Geant
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4Types.hh"
#include "G4String.hh"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "G4TessellatedSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"
#include "G4AssemblyVolume.hh"
#include "G4VFacet.hh"


namespace NPLibrary {
  namespace NPCad2Geant {
    class NPStructureBase {
    }; // NPStructureBase

    class NPSTL2Solid {
    public:
      NPSTL2Solid(G4double _baseScale = 1.0);
      ~NPSTL2Solid();

    public:
      G4VSolid* getSolid();
      void readStlFile(std::string);
      void readBinaryStlFile(std::string);
      void setBaseOffset(G4ThreeVector _v) { baseOffset = _v; };
      void setScale(G4double _v) { baseScale = _v; };
    private:
      G4TessellatedSolid * volSolid;
      int numOfFacets;
      G4double baseScale;
      G4ThreeVector baseOffset;
    }; //NPSTL2Solid
  }; // namespace NPCad2Geant

  //using namespace NPCad2Geant;
}; // namespace NPLibrary

#endif