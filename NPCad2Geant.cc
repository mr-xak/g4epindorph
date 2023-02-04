#include "NPCad2Geant.hh"

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
#include "G4TriangularFacet.hh"

using namespace NPLibrary;
using namespace NPLibrary::NPCad2Geant;

NPSTL2Solid::NPSTL2Solid(G4double _baseScale) {
  baseScale = CLHEP::mm;
  baseScale *= _baseScale;
  baseOffset = G4ThreeVector(0, 0, 0);

  numOfFacets = -1;

  volSolid = new G4TessellatedSolid("volSolid");
}

NPSTL2Solid::~NPSTL2Solid() {
}

G4VSolid* NPSTL2Solid::getSolid() {
  return volSolid;
}

void NPSTL2Solid::readStlFile(std::string pth) {
  std::cout<<"NPSTL2Solid::readStlFile Reading ASCII stl file from: "<<pth<<std::endl;
  std::ifstream fStream(pth, std::ios::in);

  char buf1[512];
  int facetCount = 0;
  std::vector<G4ThreeVector> bufFacet;

  while (!fStream.eof()) {
    fStream.getline(buf1, 512);
    if (boost::find_first(buf1, "facet") || boost::find_first(buf1, "endsolid")) {
      if (bufFacet.size() == 3) {
        //std::cout<<"Saving loop for: "<<bufFacet[0].x()<<" "<<bufFacet[0].y()<<bufFacet[0].z()
        //  <<bufFacet[1].x()<<" "<<bufFacet[1].y()<<bufFacet[1].z()
        //  <<bufFacet[2].x()<<" "<<bufFacet[2].y()<<bufFacet[2].z()
        //  <<std::endl;
        if (true) {
          G4TriangularFacet *facet = new G4TriangularFacet(bufFacet[0], bufFacet[1], bufFacet[2], ABSOLUTE);
          volSolid->AddFacet((G4VFacet *)facet);
        }
        bufFacet.clear();
        facetCount++;
      }
    }
    if (boost::find_first(buf1, "vertex")) {
      G4ThreeVector v;
      std::vector<std::string> strs;
      G4String lc(buf1); // hack, @todo: rework
      boost::trim(lc);
      boost::split(strs, lc, boost::is_any_of(" "));
      v.setX(boost::lexical_cast<G4double>(strs[1]) * baseScale + baseOffset.x());
      v.setY(boost::lexical_cast<G4double>(strs[2]) * baseScale + baseOffset.y());
      v.setZ(boost::lexical_cast<G4double>(strs[3]) * baseScale + baseOffset.z());
      bufFacet.push_back(v);
    }
  }

  volSolid->SetSolidClosed(true);

  numOfFacets = facetCount;

  std::cout<<"Readed "<<numOfFacets<<" facets"<<std::endl;
}

void NPLibrary::NPCad2Geant::NPSTL2Solid::readBinaryStlFile(std::string fname)
{
  std::cout << "NPSTL2Solid::readBinaryStlFile Reading binary stl file from: " << fname << std::endl;
  std::ifstream myFile(fname, std::ios::in | std::ios::binary);
  char header_info[80] = "";
  char nTri[4];
  unsigned long nTriLong;
  myFile.read(header_info, 80);
  myFile.read(nTri, 4);
  nTriLong = *((unsigned long*)nTri);
  std::cout << "Reading " << nTriLong << " facets ... " << std::endl;
  for (int i = 0; i < nTriLong; i++) {
    G4ThreeVector bufFacet1, bufFacet2, bufFacet3;
    char facet[50];
    myFile.read(facet, 50);

    char f1[4] = { facet[12], facet[13],facet[14],facet[15] };
    float xx = *((float*)f1);
    double facetXX = double(xx);
    char f2[4] = { facet[16], facet[17],facet[18],facet[19] };
    float xy = *((float*)f2);
    double facetXY = double(xy);
    char f3[4] = { facet[20], facet[21],facet[22],facet[23] };
    float xz = *((float*)f3);
    double facetXZ = double(xz);
    bufFacet1.setX(facetXX);
    bufFacet1.setY(facetXY);
    bufFacet1.setZ(facetXZ);

    char f4[4] = { facet[24], facet[25],facet[26],facet[27] };
    float yx = *((float*)f4);
    double facetYX = double(yx);
    char f5[4] = { facet[28], facet[29],facet[30],facet[31] };
    float yy = *((float*)f5);
    double facetYY = double(yy);
    char f6[4] = { facet[32], facet[33],facet[34],facet[35] };
    float yz = *((float*)f6);
    double facetYZ = double(yz);
    bufFacet2.setX(facetYX);
    bufFacet2.setY(facetYY);
    bufFacet2.setZ(facetYZ);

    char f7[4] = { facet[36], facet[37],facet[38],facet[39] };
    float zx = *((float*)f7);
    double facetZX = double(zx);
    char f8[4] = { facet[40], facet[41],facet[42],facet[43] };
    float zy = *((float*)f8);
    double facetZY = double(zy);
    char f9[4] = { facet[44], facet[45],facet[46],facet[47] };
    float zz = *((float*)f9);
    double facetZZ = double(zz);
    bufFacet3.setX(facetZX);
    bufFacet3.setY(facetZY);
    bufFacet3.setZ(facetZZ);

    G4TriangularFacet* g4facet = new G4TriangularFacet(bufFacet1, bufFacet2, bufFacet3, ABSOLUTE);
    volSolid->AddFacet((G4VFacet*)g4facet);

  }
  std::cout << "Read " << nTriLong << " facets"<<std::endl;
  numOfFacets = nTriLong;
  volSolid->SetSolidClosed(true);
  myFile.close();
}
