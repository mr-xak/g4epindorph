#include "NPBioLayerData.hh"
#include "G4Types.hh"
#include <math.h>
#include <unordered_map>
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/functional/hash.hpp"

using namespace NPLibrary::NPBioLayerData;

template <class T1, class T2>
std::size_t PairHash::operator () (const std::pair<T1, T2>& p) const {
  auto h1 = std::hash<T1>{}(p.first);
  auto h2 = std::hash<T2>{}(p.second);

  // Mainly for demonstration purposes, i.e. works but is overly simple
  // In the real world, use sth. like boost.hash_combine
  //return h1 ^ h2;
  //return boost::hash_combine()

  // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/combine.html
  std::size_t seed = 0;
  boost::hash_combine(seed, h1);
  boost::hash_combine(seed, h2);
  return seed;
}


#ifdef G4MULTITHREADED
namespace {
  G4Mutex vxMainMergeMutex = G4MUTEX_INITIALIZER;
  G4Mutex vxMetaSetMutex = G4MUTEX_INITIALIZER;
}
NPBioVoxelMain* NPBioVoxelMain::vxMasterInstance = 0;
G4ThreadLocal NPBioVoxelMain* NPBioVoxelMain::vxInstance = 0;
#else
NPBioVoxelMain* NPBioVoxelMain::vxInstance = 0;
#endif

void NPBioScoringLayer::Nullify()
{
  depEnergy = 0.0;
  letBase = 0.0;
  nEvents = 0;
  dose = 0.0;
  mev2gy = 0.0;
  lnsurvival = 0.0;
  letd = 0.0;
}

void NPBioScoringLayer::setMeV2GyConversion(double t)
{
  mev2gy = t;
}

void NPBioScoringLayer::Update(double e, double let)
{
  if ((let == 0.0) || (e == 0.0)) {
    return;
  }
  depEnergy += e;
  letBase += let;
  nEvents += 1;
}

void NPBioScoringLayer::Update(NPBioScoringLayer rhs) {
  depEnergy += rhs.depEnergy;
  letBase += rhs.letBase;
  nEvents += rhs.nEvents;
  //dose += rhs.dose;
  //lnsurvival += rhs.lnsurvival;
}

void NPBioScoringLayer::Update(NPBioScoringLayer *rhs) {
  if ((rhs->depEnergy == 0.0) || (rhs->letBase == 0.0)) {
    return;
  }
  depEnergy += rhs->depEnergy;
  letBase += rhs->letBase;
  nEvents += rhs->nEvents;
  //dose += rhs.dose;
  //lnsurvival += rhs.lnsurvival;
}

NPBioScoringLayer::NPBioScoringLayer()
{
  depEnergy = 0.0;
  letBase = 0.0;
  nEvents = 0;
  dose = 0.0;
  mev2gy = 0.0;
  lnsurvival = 0.0;
  letd = 0.0;
}

NPBioScoringLayer::NPBioScoringLayer(double e, double let)
{
  depEnergy = e;
  letBase = let;
  nEvents = 1;
  dose = 0.0;
  mev2gy = 0.0;
  lnsurvival = 0.0;
  letd = 0.0;
}

NPBioScoringLayer::NPBioScoringLayer(NPBioScoringLayer& rhs)
{
  depEnergy = rhs.depEnergy;
  letBase = rhs.letBase;
  nEvents = rhs.nEvents;
  mev2gy = rhs.mev2gy;
  dose = rhs.dose;
  lnsurvival = rhs.lnsurvival;
  letd = rhs.letd;
}

void NPBioScoringLayer::calculateLetD()
{
  if (nEvents > 1 && depEnergy > 0.0) {
    letd = letBase / depEnergy;
  }
  else {
    letd = 0.0;
  }
}

void NPBioScoringLayer::calculateDose()
{
  if (mev2gy > 0.0) {
    dose = depEnergy * 1e-3 * mev2gy;
  }
}

NPBioScoringLayer NPBioScoringLayer::operator+(const NPBioScoringLayer& rhs)
{
  NPBioScoringLayer tmp;
  tmp.Nullify();
  tmp.Update(const_cast<NPBioScoringLayer&>(rhs));
  return tmp;
}

NPBioScoringLayer& NPBioScoringLayer::operator+=(const NPBioScoringLayer& rhs)
{
  depEnergy += rhs.depEnergy;
  letBase += rhs.letBase;
  nEvents += rhs.nEvents;
  return *this;
}

#ifdef G4MULTITHREADED
NPBioVoxelMain::NPBioVoxelMain(bool isMaster)
{
  if (isMaster) vxMasterInstance = this;
  vxInstance = this;
}
#else
NPBioVoxelMain::NPBioVoxelMain() {
  vxMasterInstance = this;
  vxInstance = this;
}
#endif

#ifdef G4MULTITHREADED
NPBioVoxelMain* NPBioVoxelMain::Instance()
{
  if (vxInstance == 0) {
    G4bool isMaster = !G4Threading::IsWorkerThread();
    //G4bool isMaster = G4Threading::IsMasterThread();
    //if (isMaster) vxMasterInstance = new NPBioVoxelMain();
    vxInstance = new NPBioVoxelMain(isMaster);
    //vxInstance->Init();
  }

  return vxInstance;
}
#else
NPBioVoxelMain* NPBioVoxelMain::Instance()
{
  if (vxInstance == 0) {
    vxInstance = new NPBioVoxelMain();
//    vxInstance->Init();
  }
  return vxInstance;
}
#endif


void NPBioVoxelMain::setVoxelData(int vx, int vy, int vz)
{
  _xLim = vx;
  _yLim = vy;
  _zLim = vz;
#ifdef G4MULTITHREADED
  G4AutoLock lMut(&vxMetaSetMutex);
  if (G4Threading::IsWorkerThread() && (!vxMasterInstance->isLimSet)) {
    vxMasterInstance->_xLim = vx;
    vxMasterInstance->_yLim = vy;
    vxMasterInstance->_zLim = vz;
    vxMasterInstance->isLimSet = true;
  }
#endif
  //dosemap2 = new double[_xLim * _yLim * _zLim];
  //letmap2 = new double[_xLim * _yLim * _zLim];
  //for (int i = 0; i < _xLim; i++) {
  //  for (int j = 0; j < _yLim; j++) {
  //    for (int k = 0; k < _zLim; k++) {
  //      dosemap2[getNumFromXYZCoord(i, j, k)] = 0.0;
  //      letmap2[getNumFromXYZCoord(i, j, k)] = 0.0;
  //    }
  //  }
  //}
}

myIntType NPBioVoxelMain::getNumFromXYZCoord(myIntType ix, myIntType iy, myIntType iz)
{
  return myIntType(ix + (myIntType)_xLim * iy + (myIntType)_xLim * (myIntType)_yLim * iz);
}

myIntType NPBioVoxelMain::getXCoordFromNum(myIntType bn)
{
  return bn - myIntType((bn - (myIntType(bn / (_xLim * _yLim))) * _xLim * _yLim) / _xLim) * _xLim - (myIntType(bn / (_xLim * _yLim))) * _xLim * _yLim;
}

myIntType NPBioVoxelMain::getYCoordFromNum(myIntType bn)
{
  return myIntType((bn - (myIntType(bn / (_xLim * _yLim))) * _xLim * _yLim) / _xLim);
}

myIntType NPBioVoxelMain::getZCoordFromNum(myIntType bn)
{
  return myIntType(bn / (_xLim * _yLim));
}

void NPLibrary::NPBioLayerData::NPBioVoxelMain::addData(myIntType voxNum, myIntType part, double ene, double let)
{
  //dosemap2[voxNum] += ene;
  //letmap2[voxNum] += let;

  myIntPair t0(voxNum, 0);
  myIntPair tT(voxNum, part);
  //myIntType t0 = voxNum;

  //if (dosemap1.count(t0) == 0) {
  //  dosemap1.insert(std::pair<myIntType, double>(t0, ene));
  //}
  //else {
  //  dosemap1.find(t0)->second += ene;
  //}

  //if (letmap1.count(t0) == 0) {
  //  letmap1.insert(std::pair<myIntType, double>(t0, let));
  //}
  //else {
  //  letmap1.find(t0)->second += let;
  //}

  if (dosemap.count(t0) == 0) {
    NPBioScoringLayer* tmp = new NPBioScoringLayer();
    tmp->Nullify();
    tmp->Update(ene, let);
    dosemap.insert(NPBioTotalScoringMapPointerPair(t0, tmp));
  }
  else {
    dosemap.find(t0)->second->Update(ene, let);
  }

  if (dosemap.count(tT) == 0) {
    NPBioScoringLayer* tmp = new NPBioScoringLayer();
    tmp->Nullify();
    tmp->Update(ene, let);
    dosemap.insert(NPBioTotalScoringMapPointerPair(tT, tmp));
  }
  else {
    dosemap.find(tT)->second->Update(ene, let);
  }
}

void NPBioVoxelMain::finalize()
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

  for (auto it = dosemap.begin(); it != dosemap.end(); ++it) {
    it->second->calculateLetD();
  }
}

void NPBioVoxelMain::_merge(NPBioTotalScoringMapPointer rhs)
{
  for (auto it = rhs.begin(); it != rhs.end(); ++it) {
    if (dosemap.count(it->first) == 0) {
      dosemap.insert(NPBioTotalScoringMapPointerPair(it->first, it->second));
    }
    else {
      dosemap.find(it->first)->second->Update(it->second);
    }
  }
}

int NPBioVoxelMain::addMeV2GyData(G4String fFileName)
{
  std::cout << "Adding MeV2Gy Data from " << fFileName << std::endl;
  
  std::string buf;
  std::vector<std::string> strs;
  std::ifstream mev2gyFileStream;
  mev2gyFileStream.open(fFileName.c_str(), std::ios::in);
  myIntType i, j, k;
  double val;
  
  while (!mev2gyFileStream.eof()) {
    std::getline(mev2gyFileStream, buf);
    if (strlen(buf.c_str()) < 1) {
      continue;
    }
    boost::split(strs, buf, boost::is_any_of("\t"));
    i = boost::lexical_cast<myIntType>(strs[0]);
    j = boost::lexical_cast<myIntType>(strs[1]);
    k = boost::lexical_cast<myIntType>(strs[2]);
    boost::trim(strs[4]);
    val = boost::lexical_cast<double>(strs[4]);
    for (int part = -1; part <= 8; part++) {
      myIntType voxNum = getNumFromXYZCoord(i, j, k);
      myIntPair tmp(voxNum, (myIntType)part);
      if (dosemap.count(tmp) > 0) {
        NPBioScoringLayer* tmpLayer;
        tmpLayer = dosemap.find(tmp)->second;
        tmpLayer->setMeV2GyConversion(val);
      }
    }
  }
  return 0;
  /*for (myIntType i = 0; i < _xLim; i++) {
    for (myIntType j = 0; j < _yLim; j++) {
      for (myIntType k = 0; k < _zLim; k++) {
      }
    }
  }*/
}

int NPLibrary::NPBioLayerData::NPBioVoxelMain::addMeV2GyData(G4double val)
{
  /* calling this function before simulation start (e.g. in DetectorConstruction/WorkerInitialization 
  * may trigger to have no effect between unordered_map vs simple map
  */
  for (myIntType i = 0; i < _xLim; i++) {
    for (myIntType j = 0; j < _yLim; j++) {
      for (myIntType k = 0; k < _zLim; k++) {
        for (int part = -1; part <= 8; part++) {
          myIntType voxNum = getNumFromXYZCoord(i, j, k);
          myIntPair tmp(voxNum, (myIntType)part);
          if (dosemap.count(tmp) > 0) {
            NPBioScoringLayer* tmpLayer;
            tmpLayer = dosemap.find(tmp)->second;
            tmpLayer->setMeV2GyConversion(val);
          }
          else {
            NPBioScoringLayer* tmpLayer = new NPBioScoringLayer();
            tmpLayer->Nullify();
            tmpLayer->setMeV2GyConversion(val);
            dosemap.insert(NPBioTotalScoringMapPointerPair(tmp, tmpLayer));
          }
        }
      }
    }
  }
  return 0;
}



void NPBioVoxelMain::dumpDoseDepsInFiles(int mode = 0, G4String fNameBase = "dose")
{
  std::cout << "Saving dose, mode " << mode << std::endl;
  if (mode == 0) {
    //for (auto it = dosemap.begin(); it != dosemap.end(); ++it) {
    //}
    for (int part = -1; part <= 8; part++) {
      std::stringstream fName;
      fName << fNameBase << ".p" << part << ".doses";
      std::ofstream fStream(fName.str(), std::ios::out);
      for (myIntType i = 0; i < _xLim; i++) {
        for (myIntType j = 0; j < _yLim; j++) {
          for (myIntType k = 0; k < _zLim; k++) {
            myIntType voxNum = getNumFromXYZCoord(i, j, k);
            myIntPair tmp(voxNum, (myIntType)part);
            if (dosemap.count(tmp) > 0) {
              NPBioScoringLayer* tmpLayer;
              tmpLayer = dosemap.find(tmp)->second;
              fStream << i << "\t" << j << "\t" << k << "\t" << tmpLayer->depEnergy << "\t" << tmpLayer->letd << "\t" << tmpLayer->nEvents << "\n";
            }
          }
        }
      }
      fStream.close();
    }
  }
  else if (mode == 1) {
    for (int part = -1; part <= 8; part++) {
      std::stringstream fName;
      fName << fNameBase << ".d" << part << ".doses";
      std::ofstream fStream(fName.str(), std::ios::out);
      for (myIntType i = 0; i < _xLim; i++) {
        for (myIntType j = 0; j < _yLim; j++) {
          for (myIntType k = 0; k < _zLim; k++) {
            myIntType voxNum = getNumFromXYZCoord(i, j, k);
            myIntPair tmp(voxNum, (myIntType)part);
            if (dosemap.count(tmp) > 0) {
              NPBioScoringLayer* tmpLayer;
              tmpLayer = dosemap.find(tmp)->second;
              tmpLayer->calculateDose();
              fStream << i << "\t" << j << "\t" << k << "\t" << tmpLayer->depEnergy << "\t" << tmpLayer->letd << "\t" << tmpLayer->dose << "\n";
            }
          }
        }
      }
      fStream.close();
    }
  }
  else if (mode == 3) {
    std::stringstream fName;
    fName << fNameBase << ".w" << ".doses";
    std::ofstream fStream(fName.str(), std::ios::out);
    fStream << "I\tJ\tK\t  Dose  \t Tot.LETd \t e- dose \t e- LETd \t p dose \t p LETd \t He dose \t He LETd \t Li dose \t Li LETd \t Be dose \t Be LETd \t B dose \t B LETd \t C dose \t C LETd \t N dose \t N LETd \t O dose \t O LETd \t\n";
    for (myIntType i = 0; i < _xLim; i++) {
      for (myIntType j = 0; j < _yLim; j++) {
        for (myIntType k = 0; k < _zLim; k++) {
          myIntType voxNum = getNumFromXYZCoord(i, j, k);
          myIntPair tmp0(voxNum, 0);
          if (dosemap.count(tmp0)) {
            NPBioScoringLayer* tmpLayer0;
            tmpLayer0 = dosemap.find(tmp0)->second;
            //if (tmpLayer0->nEvents < 1000) {
            //  continue;
            //}
            tmpLayer0->calculateDose();
            tmpLayer0->calculateLetD();

            //std::cout << tmpLayer0->mev2gy << std::endl;

            fStream << std::fixed << i << "\t" << j << "\t" << k << "\t" << std::scientific << tmpLayer0->dose << "\t" << tmpLayer0->letd << "\t";
            for (int part = -1; part <= 8; part++) {
              if (part == 0) continue;
              myIntPair tmp(voxNum, (myIntType)part);
              if (dosemap.count(tmp)) {
                NPBioScoringLayer* tmpLayer;
                tmpLayer = dosemap.find(tmp)->second;
                tmpLayer->calculateDose();
                tmpLayer->calculateLetD();
                fStream << tmpLayer->depEnergy / tmpLayer0->depEnergy << "\t";
                fStream << tmpLayer->letd << "\t";
              }
              else {
                fStream << "0.000000e0\t0.000000e0" << "\t";
              }
            }
            fStream << std::endl;
          }
        }
        //fStream << std::endl;
      }
      //fStream << std::endl;
    }
    fStream.close();
  }

}

void NPBioVoxelMain::dummyPrint() {
  for (auto it = dosemap.begin(); it != dosemap.end(); ++it) {
    std::cout << it->first.first << "\t" << it->first.second << "\t" << it->second->depEnergy << "\t" << it->second->letd << "\n";
  }
}

NPBioVoxelMain::~NPBioVoxelMain()
{

}