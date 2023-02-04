#include "Run.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4SDManager.hh"
#include "G4Types.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#include "NPFastLayersData.hh"

typedef std::map<G4int, G4double> tgD;

namespace {
  G4Mutex rnMainMergeMutex = G4MUTEX_INITIALIZER;
}

Run::Run() : G4Run(), eneSum(0.0), letBase1(0.0), letBase2(0.0), let2Avg(0.0), dxSum(0.0), qu(0)
{
  G4cout << "Created Run() instance" << G4endl;
}

Run::~Run()
{

}

void Run::addDoseMap(tgD& detDoseMap) {
  for (auto it = detDoseMap.begin(); it != detDoseMap.end(); ++it) {
    if (doseMap.count(it->first) == 0) {
      doseMap[it->first] = it->second;
    }
    else {
      doseMap[it->first] += it->second;
    }
  }
}
void Run::addLetMap(tgD& detLetMap) {
  for (auto it = detLetMap.begin(); it != detLetMap.end(); ++it) {
    if (letMap.count(it->first) == 0) {
      letMap[it->first] = it->second;
    }
    else {
      letMap[it->first] += it->second;
    }
  }
}

void Run::addLETFulLData(G4double _ene, G4double _let1, G4double _let2, G4double _letAvg, G4double _dx, G4int _qu) {
  eneSum += _ene;
  letBase1 += _let1;
  letBase2 += _let2;
  let2Avg += _letAvg;
  dxSum += _dx;
  qu += _qu;
}

void Run::addParticle2PhaseSpace(int pid, G4double ene, G4double x, G4double y, G4double z, G4double ux, G4double uy, G4double uz)
{
  PhaseSpace0 ph;
  ph.pid = pid;
  ph.ene = (float)ene;
  ph.x = (float)x;
  ph.y = (float)y;
  ph.z = (float)z;
  ph.ux = (float)ux;
  ph.uy = (float)uy;
  ph.uz = (float)uz;
  PhaseSpaceVectorTarget.push_back(ph);
}

void Run::Merge(const G4Run* aRun) {

  G4AutoLock lMut(&rnMainMergeMutex);

  G4cout << "Merge called, is master: " << G4Threading::IsMasterThread() << G4endl;

  const Run* localRun = static_cast<const Run*>(aRun);

  for (auto it = localRun->doseMap.begin(); it != localRun->doseMap.end(); ++it) {
    if (doseMap.count(it->first) == 0) {
      doseMap.insert(std::pair<G4int, G4double>(it->first, it->second));
    }
    else {
      doseMap[it->first] += it->second;
    }
  }

  for (auto it = localRun->letMap.begin(); it != localRun->letMap.end(); ++it) {
    if (letMap.count(it->first) == 0) {
      letMap.insert(std::pair<G4int, G4double>(it->first, it->second));
    }
    else {
      letMap[it->first] += it->second;
    }
  }

  for (size_t i = 0; i < localRun->PhaseSpaceVectorTarget.size(); i++) {
    PhaseSpaceVectorTarget.push_back(localRun->PhaseSpaceVectorTarget[i]);
  }

  eneSum += localRun->eneSum;
  letBase1 += localRun->letBase1;
  letBase2 += localRun->letBase2;
  let2Avg += localRun->let2Avg;
  dxSum += localRun->dxSum;
  qu += localRun->qu;

  lMut.unlock();

  NPLibrary::NPFastLayerData::NPFastVoxelMain* vxFastData = NPLibrary::NPFastLayerData::NPFastVoxelMain::Instance();
  vxFastData->finalize();

  G4Run::Merge(aRun);
}

void Run::EndOfRun() {
  //G4cout << "--== RUN dose/LET data" << G4endl;
  //for (auto it = doseMap.begin(); it != doseMap.end(); ++it) {
  //  G4cout << it->first << "\t" << it->second / MeV << "\t" << letMap[it->first] / it->second << G4endl;
  //}
  //G4cout << "==-- EOF RUN dose/LET data" << G4endl;

  G4cout << "--== STATS FOR SINGLE EPI" << G4endl;
  G4cout << "Total MeV: " << eneSum / MeV << G4endl;
  G4cout << "LETd (old): " << (letBase1 / eneSum) / (keV / um) << " keV/um" << G4endl;
  G4cout << "LETd (new): " << (letBase2 / eneSum) / (keV / um) << " keV/um" << G4endl;
  G4cout << "AvgLet " << (let2Avg / qu) / (keV / um) << " keV/um" << G4endl;
  G4cout << "LETt (var): " << (eneSum / dxSum) / (keV / um) << " keV/um" << G4endl;
  G4cout << "N: " << qu << G4endl;

  G4cout << "Dose: ";
  NPLibrary::NPFastLayerData::NPFastVoxelMain* vxFastData = NPLibrary::NPFastLayerData::NPFastVoxelMain::Instance();
  G4cout << vxFastData->getData(0) << " Gy " ;
  G4cout << "Error: " << vxFastData->getAbsError(0) << G4endl;
}

