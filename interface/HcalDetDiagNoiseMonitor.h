#ifndef DQM_HCALMONITORTASKS_HCALDETDIAGNOISEMONITOR_H
#define DQM_HCALMONITORTASKS_HCALDETDIAGNOISEMONITOR_H

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "DQM/HcalMonitorTasks/interface/HcalBaseDQMonitor.h"

#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/JetReco/interface/CaloJet.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

// forward declarations
class HcalDetDiagNoiseRMSummary; 
class DQMStore;
class MonitorElement;
class HcalDbService;
class HcalLogicalMap;
class HcalLogicalMapGenerator;

// #########################################################################################

/** \class HcalDetDiagNoiseMonitor
  *  
  * $Date: 2010/03/03 18:37:28 $
  * $Revision: 1.4.2.2 $
  * \author D. Vishnevskiy
  */


class HcalDetDiagNoiseMonitor:public HcalBaseDQMonitor {
public:
  HcalDetDiagNoiseMonitor(const edm::ParameterSet& ps); 
  ~HcalDetDiagNoiseMonitor(); 

  void setup();
  void analyze(edm::Event const&e, edm::EventSetup const&s);
  void done();
  void reset();
  void beginRun(const edm::Run& run, const edm::EventSetup& c);
  void cleanup(); 
  void UpdateHistos();
  int  GetStatistics(){ return ievt_; }
private:
  edm::InputTag digiLabel_;
  edm::InputTag rawDataLabel_;
  
  HcalLogicalMap          *lmap;
  HcalLogicalMapGenerator *gen;
  const HcalDbService* cond_; // cond isn't used for anything!  Remove it?
 
  std::string ReferenceData;
  std::string ReferenceRun;
  std::string OutputFilePath;
  bool IsReference;
  bool UseDB;
  
  double  HPDthresholdHi;
  double  HPDthresholdLo;
  double  SiPMthreshold;
  double  SpikeThreshold;
  int     UpdateEvents;
  
  void SaveReference();
  void LoadReference();
  
  int         ievt_;
  int         run_number;
  int         NoisyEvents;
  MonitorElement *meEVT_;
  MonitorElement *HB_RBXmapRatio;
  MonitorElement *HB_RBXmapRatioCur;
  MonitorElement *HB_RBXmapSpikeCnt;
  MonitorElement *HB_RBXmapSpikeAmp;
  MonitorElement *HE_RBXmapRatio;
  MonitorElement *HE_RBXmapRatioCur;
  MonitorElement *HE_RBXmapSpikeCnt;
  MonitorElement *HE_RBXmapSpikeAmp;
  MonitorElement *HO_RBXmapRatio;
  MonitorElement *HO_RBXmapRatioCur;
  MonitorElement *HO_RBXmapSpikeCnt;
  MonitorElement *HO_RBXmapSpikeAmp;
  
  MonitorElement *PixelMult;
  MonitorElement *HPDEnergy;
  MonitorElement *RBXEnergy;
  
  HcalDetDiagNoiseRMSummary* RBXSummary;
  HcalDetDiagNoiseRMSummary* RBXCurrentSummary;

// #########################################################################################

  MonitorElement *Met_AllEvents;
  MonitorElement *Mephi_AllEvents;
  MonitorElement *Mex_AllEvents;
  MonitorElement *Mey_AllEvents;
  MonitorElement *SumEt_AllEvents;
  MonitorElement *Met_passingTrigger;
  MonitorElement *Mephi_passingTrigger;
  MonitorElement *Mex_passingTrigger;
  MonitorElement *Mey_passingTrigger;
  MonitorElement *SumEt_passingTrigger;
  MonitorElement *CorrectedMet_passingTrigger;
  MonitorElement *CorrectedMephi_passingTrigger;
  MonitorElement *CorrectedMex_passingTrigger;
  MonitorElement *CorrectedMey_passingTrigger;
  MonitorElement *CorrectedSumEt_passingTrigger;
  MonitorElement *HCALFraction_passingTrigger;
  MonitorElement *chargeFraction_passingTrigger;
  MonitorElement *JetEt_passingTrigger;
  MonitorElement *JetEta_passingTrigger;
  MonitorElement *JetPhi_passingTrigger;
  MonitorElement *JetEt_passingTrigger_TaggedAnomalous;
  MonitorElement *JetEta_passingTrigger_TaggedAnomalous;
  MonitorElement *JetPhi_passingTrigger_TaggedAnomalous;
  MonitorElement *Met_passingTrigger_HcalNoiseCategory;
  MonitorElement *Mephi_passingTrigger_HcalNoiseCategory;
  MonitorElement *Mex_passingTrigger_HcalNoiseCategory;
  MonitorElement *Mey_passingTrigger_HcalNoiseCategory;
  MonitorElement *SumEt_passingTrigger_HcalNoiseCategory;
  MonitorElement *Met_passingTrigger_PhysicsCategory;
  MonitorElement *Mephi_passingTrigger_PhysicsCategory;
  MonitorElement *Mex_passingTrigger_PhysicsCategory;
  MonitorElement *Mey_passingTrigger_PhysicsCategory;
  MonitorElement *SumEt_passingTrigger_PhysicsCategory;
  MonitorElement *HCALFractionVSchargeFraction_passingTrigger;
  MonitorElement *RBXMaxZeros_passingTrigger_HcalNoiseCategory;
  MonitorElement *RBXE2tsOverE10ts_passingTrigger_HcalNoiseCategory;
  MonitorElement *RBXHitsHighest_passingTrigger_HcalNoiseCategory;
  MonitorElement *HPDE2tsOverE10ts_passingTrigger_HcalNoiseCategory;
  MonitorElement *HPDHitsHighest_passingTrigger_HcalNoiseCategory;
  MonitorElement *RBXMaxZeros_passingTrigger_PhysicsCategory;
  MonitorElement *RBXHitsHighest_passingTrigger_PhysicsCategory;
  MonitorElement *RBXE2tsOverE10ts_passingTrigger_PhysicsCategory;
  MonitorElement *HPDHitsHighest_passingTrigger_PhysicsCategory;
  MonitorElement *HPDE2tsOverE10ts_passingTrigger_PhysicsCategory;
  MonitorElement *NLumiSections;

  edm::InputTag hlTriggerResults_;
  edm::InputTag MetSource_;
  edm::InputTag JetSource_;
  edm::InputTag TrackSource_;
  std::string rbxCollName_;
  std::string TriggerRequirement_;
  bool UseMetCutInsteadOfTrigger_;
  double MetCut_;
  double JetMinEt_;
  double JetMaxEta_;
  double ConstituentsToJetMatchingDeltaR_;
  double TrackMaxIp_;
  double TrackMinThreshold_;
  double MinJetChargeFraction_;
  double MaxJetHadronicEnergyFraction_;

  edm::InputTag caloTowerCollName_;

  std::vector<unsigned int> lumi;
  std::vector<reco::CaloJet> HcalNoisyJetContainer;

  int numRBXhits;
  double rbxenergy;
  double hpdEnergyHighest;
  double nHitsHighest;
  double totale2ts;
  double totale10ts;
  int numHPDhits;
  double e2ts;
  double e10ts;
  bool isRBXNoisy;
  bool isHPDNoisy;

// #########################################################################################

};

#endif
