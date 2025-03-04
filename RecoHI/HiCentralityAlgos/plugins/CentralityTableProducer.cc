
// system include files
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/RecoCandidate/interface/RecoCaloTowerCandidate.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"

#include "DataFormats/Common/interface/Ref.h"

#include "DataFormats/HeavyIonEvent/interface/CentralityBins.h"

#include "CondFormats/DataRecord/interface/HeavyIonRcd.h"
#include "CondFormats/HIObjects/interface/CentralityTable.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

#include <TFile.h>

using namespace std;
//
// class decleration
//

class CentralityTableProducer : public edm::EDAnalyzer {
public:
  explicit CentralityTableProducer(const edm::ParameterSet&);
  ~CentralityTableProducer() override;

private:
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;
  void printBin(const CentralityTable::CBin*);
  // ----------member data ---------------------------

  bool makeDBFromTFile_;
  bool makeTFileFromDB_;
  bool firstRunOnly_;
  bool debug_;

  TFile* inputTFile_;
  string inputTFileName_;
  edm::Service<TFileService> fs;

  string rootTag_;
  ofstream text_;

  const CentralityBins* CB;

  unsigned int runnum_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
CentralityTableProducer::CentralityTableProducer(const edm::ParameterSet& iConfig) : text_("bins.txt"), runnum_(0) {
  //now do what ever initialization is needed
  makeDBFromTFile_ = iConfig.getUntrackedParameter<bool>("makeDBFromTFile", true);
  makeTFileFromDB_ = iConfig.getUntrackedParameter<bool>("makeTFileFromDB", false);
  firstRunOnly_ = iConfig.getUntrackedParameter<bool>("isMC", false);
  debug_ = iConfig.getUntrackedParameter<bool>("debug", false);
  if (makeDBFromTFile_) {
    inputTFileName_ = iConfig.getParameter<string>("inputTFile");
    rootTag_ = iConfig.getParameter<string>("rootTag");
    inputTFile_ = new TFile(inputTFileName_.data(), "read");
    cout << inputTFileName_.data() << endl;
  }
}

CentralityTableProducer::~CentralityTableProducer() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}

//
// member functions
//

// ------------ method called to for each event  ------------
void CentralityTableProducer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  if (!makeTFileFromDB_)
    return;
  if ((!firstRunOnly_ && runnum_ != iEvent.id().run()) || (firstRunOnly_ && runnum_ == 0)) {
    runnum_ = iEvent.id().run();
    cout << "Adding table for run : " << runnum_ << endl;
    TFileDirectory subDir = fs->mkdir(Form("run%d", runnum_));
    CB = subDir.make<CentralityBins>();
  }
}

// ------------ method called once each job just after ending the event loop  ------------
void CentralityTableProducer::endJob() {
  if (makeDBFromTFile_) {
    runnum_ = 1;
    // Get values from root file
    CB = (CentralityBins*)inputTFile_->Get(Form("%s/run%d", rootTag_.data(), runnum_));
    cout << rootTag_.data() << endl;
    CentralityTable CT;
    CT.m_table.reserve(CB->getNbins());

    text_ << "# BinEdge NpartMean NpartVar NcollMean NcollVar NhardMean NhardVar bMean bVar" << endl;
    for (int j = 0; j < CB->getNbins(); j++) {
      CentralityTable::CBin* thisBin = new CentralityTable::CBin();
      thisBin->bin_edge = CB->lowEdgeOfBin(j);
      thisBin->n_part.mean = CB->NpartMeanOfBin(j);
      thisBin->n_part.var = CB->NpartSigmaOfBin(j);
      thisBin->n_coll.mean = CB->NcollMeanOfBin(j);
      thisBin->n_coll.var = CB->NcollSigmaOfBin(j);
      thisBin->n_hard.mean = CB->NhardMeanOfBin(j);
      thisBin->n_hard.var = CB->NhardSigmaOfBin(j);
      thisBin->b.mean = CB->bMeanOfBin(j);
      thisBin->b.var = CB->bSigmaOfBin(j);
      printBin(thisBin);
      CT.m_table.push_back(*thisBin);
      if (thisBin)
        delete thisBin;
    }

    edm::Service<cond::service::PoolDBOutputService> pool;
    if (pool.isAvailable()) {
      pool->writeOneIOV(CT, pool->currentTime(), "HeavyIonRcd");
    }
  }
}

void CentralityTableProducer::printBin(const CentralityTable::CBin* thisBin) {
  cout << "HF Cut = " << thisBin->bin_edge << endl;
  cout << "Npart = " << thisBin->n_part.mean << endl;
  cout << "sigma = " << thisBin->n_part.var << endl;
  cout << "Ncoll = " << thisBin->n_coll.mean << endl;
  cout << "sigma = " << thisBin->n_coll.var << endl;
  cout << "B     = " << thisBin->b.mean << endl;
  cout << "sigma = " << thisBin->b.var << endl;
  text_ << Form("%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f",
                (Float_t)thisBin->bin_edge,
                (Float_t)thisBin->n_part.mean,
                (Float_t)thisBin->n_part.var,
                (Float_t)thisBin->n_coll.mean,
                (Float_t)thisBin->n_coll.var,
                (Float_t)thisBin->n_hard.mean,
                (Float_t)thisBin->n_hard.var,
                (Float_t)thisBin->b.mean,
                (Float_t)thisBin->b.var)
        << endl;
  cout << "__________________________________________________" << endl;
}

//define this as a plug-in
DEFINE_FWK_MODULE(CentralityTableProducer);
