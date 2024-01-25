#include "NPFastLayersData.hh"
#include "G4Threading.hh"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#ifdef _WIN32
#include <boost/config/compiler/visualc.hpp>
#else
// @todo: check another variants
#include <boost/config/compiler/gcc.hpp>  
#include <inttypes.h>
#endif

using namespace NPLibrary;
using namespace NPLibrary::NPFastLayerData;


#ifdef G4MULTITHREADED
namespace {
  G4Mutex vxMainMergeMutex = G4MUTEX_INITIALIZER;
  G4Mutex vxMetaSetMutex = G4MUTEX_INITIALIZER;
}
NPFastVoxelMain* NPFastVoxelMain::vxMasterInstance = nullptr;
G4ThreadLocal NPFastVoxelMain* NPFastVoxelMain::vxInstance = nullptr;
#else
NPFastVoxelMain* NPFastVoxelMain::vxInstance = 0;
#endif

void NPLibrary::NPFastLayerData::NPFastVoxelMain::Init()
{
  _xLim = -1;
  _yLim = -1;
  _zLim = -1;
  cf = 1.0;
}

void NPLibrary::NPFastLayerData::NPFastVoxelMain::finalize()
{
#ifdef G4MULTITHREADED
  if (!G4Threading::IsWorkerThread()) {
    // master
  }
  else {
    G4AutoLock lMut(&vxMainMergeMutex);
    vxMasterInstance->_merge(dosemap);
    lMut.unlock();
  }
#endif
}

void NPLibrary::NPFastLayerData::NPFastVoxelMain::_merge(NPFastScoringMapPointer rhs)
{
  for (auto it = rhs.begin(); it != rhs.end(); ++it) {
    if (dosemap.count(it->first) == 0) {
      dosemap.insert(NPFastScoringMapPointerPair(it->first, it->second));
    }
    else {
      dosemap.find(it->first)->second->Update(it->second);
    }
  }
}

void NPLibrary::NPFastLayerData::NPFastVoxelMain::dumpDoseDepositWithError(std::string fFileName, myIntType countLowLimit)
{
  std::ofstream outFile(fFileName, std::ios::out);
  for (myIntType i = 0; i < _xLim; i++) {
    for (myIntType j = 0; j < _yLim; j++) {
      for (myIntType k = 0; k < _zLim; k++) {
        if (0 == dosemap.count(getNumFromXYZCoord(i, j, k))) {
          outFile << i << "\t" << j << "\t" << k << "\t" << "0.0\t1.0\n";
          continue;
        }
        NPFastScoringLayer* tmp = dosemap.find(getNumFromXYZCoord(i, j, k))->second;
        if (tmp->nEvents > countLowLimit) {
          outFile << i << "\t" << j << "\t" << k << "\t" << tmp->depEnergy << "\t" << tmp->calculateError() / tmp->depEnergy << "\n";
        }
        else {
          outFile << i << "\t" << j << "\t" << k << "\t" << "0.0\t1.0\n";
        }
      }
    }
  }
  outFile.close();
}

void NPLibrary::NPFastLayerData::NPFastVoxelMain::setVoxelData(int x, int y, int z)
{
  _xLim = x;
  _yLim = y;
  _zLim = z;
#ifdef G4MULTITHREADED
  G4AutoLock lMut(&vxMetaSetMutex);
  if (G4Threading::IsWorkerThread() && (!vxMasterInstance->isLimSet)) {
    vxMasterInstance->_xLim = x;
    vxMasterInstance->_yLim = y;
    vxMasterInstance->_zLim = z;
    vxMasterInstance->isLimSet = true;
  }
#endif
}

void NPLibrary::NPFastLayerData::NPFastVoxelMain::setVoxelData(int x, int y, int z, double val) {
  setVoxelData(x, y, z);
  setCf(val);
}

myIntType NPLibrary::NPFastLayerData::NPFastVoxelMain::getNumFromXYZCoord(myIntType ix, myIntType iy, myIntType iz)
{
  return myIntType(ix + (myIntType)_xLim * iy + (myIntType)_xLim * (myIntType)_yLim * iz);
}

void NPLibrary::NPFastLayerData::NPFastVoxelMain::addData(myIntType voxNum, double ene)
{
  if (dosemap.count(voxNum) > 0) {
    dosemap.find(voxNum)->second->Update(ene);
  }
  else {
    dosemap.insert(NPFastScoringMapPointerPair(voxNum, new NPFastScoringLayer(ene)));
  }
}

double NPLibrary::NPFastLayerData::NPFastVoxelMain::getData(myIntType voxNum)
{
  if (dosemap.count(voxNum)) {
    return dosemap.find(voxNum)->second->depEnergy * cf;
  }
  return 0.0;
}

double NPLibrary::NPFastLayerData::NPFastVoxelMain::getAbsError(myIntType voxNum)
{
  if (dosemap.count(voxNum)) {
    auto* pt = dosemap.find(voxNum)->second;
    return pt->calculateError() * cf;
  }
  return 0.0;
}

#ifdef G4MULTITHREADED
NPLibrary::NPFastLayerData::NPFastVoxelMain::NPFastVoxelMain(bool isMaster)
{
  if (isMaster) vxMasterInstance = this;
  vxInstance = this;
}
#else
NPLibrary::NPFastLayerData::NPFastVoxelMain::NPFastVoxelMain() {

}
#endif

#ifdef G4MULTITHREADED
NPFastVoxelMain* NPFastVoxelMain::Instance()
{
  if (vxInstance == nullptr) {
    G4bool isMaster = !G4Threading::IsWorkerThread();
    //G4bool isMaster = G4Threading::IsMasterThread();
    //if (isMaster) vxMasterInstance = new NPVoxelMain();
    vxInstance = new NPFastVoxelMain(isMaster);
    vxInstance->Init();
  }

  return vxInstance;
}
#else
NPFastVoxelMain* NPFastVoxelMain::Instance()
{
  if (vxInstance == 0) {
    vxInstance = new NPFastVoxelMain();
    vxInstance->Init();
  }
  return vxInstance;
}
#endif

NPLibrary::NPFastLayerData::NPFastVoxelMain::~NPFastVoxelMain()
{
}

void NPLibrary::NPFastLayerData::NPFastScoringLayer::Nullify()
{
  depEnergy = 0.0;
  depEnergy2 = 0.0;
  nEvents = 0;
}

NPLibrary::NPFastLayerData::NPFastScoringLayer::NPFastScoringLayer() : depEnergy(0.0), depEnergy2(0.0), nEvents(0)
{
}

NPLibrary::NPFastLayerData::NPFastScoringLayer::NPFastScoringLayer(double ene) : depEnergy(ene), depEnergy2(ene* ene), nEvents(1)
{
}

NPLibrary::NPFastLayerData::NPFastScoringLayer::NPFastScoringLayer(const NPFastScoringLayer& rhs)
{
  depEnergy = rhs.depEnergy;
  depEnergy2 = rhs.depEnergy2;
  nEvents = rhs.nEvents;
}

NPLibrary::NPFastLayerData::NPFastScoringLayer::~NPFastScoringLayer()
{
}

void NPLibrary::NPFastLayerData::NPFastScoringLayer::Update(double ene)
{
  depEnergy += ene;
  depEnergy2 += ene * ene;
  nEvents += 1;
}

void NPLibrary::NPFastLayerData::NPFastScoringLayer::Update(NPFastScoringLayer rhs)
{
  depEnergy += rhs.depEnergy;
  depEnergy2 += rhs.depEnergy2;
  nEvents += rhs.nEvents;
}

void NPLibrary::NPFastLayerData::NPFastScoringLayer::Update(NPFastScoringLayer* rhs)
{
  depEnergy += rhs->depEnergy;
  depEnergy2 += rhs->depEnergy2;
  nEvents += rhs->nEvents;

}

NPFastScoringLayer NPLibrary::NPFastLayerData::NPFastScoringLayer::operator+(const NPFastScoringLayer& rhs)
{
  NPFastScoringLayer tmp;
  tmp.Nullify();
  tmp.Update(const_cast<NPFastScoringLayer&>(rhs));
  return tmp;
}

NPFastScoringLayer& NPLibrary::NPFastLayerData::NPFastScoringLayer::operator+=(const NPFastScoringLayer& rhs)
{
  depEnergy += rhs.depEnergy;
  depEnergy2 += rhs.depEnergy2;
  nEvents += rhs.nEvents;
  return *this;
}

double NPLibrary::NPFastLayerData::NPFastScoringLayer::calculateError()
{
  if (nEvents < 2) {
    return 1.0;
  }
  return std::sqrt((nEvents * depEnergy2 - depEnergy * depEnergy) / (nEvents - 1));
}
