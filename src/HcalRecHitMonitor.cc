#include "DQM/HcalMonitorTasks/interface/HcalRecHitMonitor.h"
#include "FWCore/Common/interface/TriggerNames.h" 
#include "CondFormats/HcalObjects/interface/HcalChannelStatus.h"
#include "CondFormats/HcalObjects/interface/HcalChannelQuality.h"

#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"
#include "RecoLocalCalo/HcalRecAlgos/interface/HcalCaloFlagLabels.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"
#include "Geometry/HcalTowerAlgo/src/HcalHardcodeGeometryData.h" // for eta bounds
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <cmath>

using namespace std;
using namespace edm;

HcalRecHitMonitor::HcalRecHitMonitor(const edm::ParameterSet& ps)
{
  // Common Base Class parameters
  Online_                = ps.getUntrackedParameter<bool>("online",false);
  mergeRuns_             = ps.getUntrackedParameter<bool>("mergeRuns",false);
  enableCleanup_         = ps.getUntrackedParameter<bool>("enableCleanup",false);
  debug_                 = ps.getUntrackedParameter<int>("debug",false);
  prefixME_              = ps.getUntrackedParameter<string>("subSystemFolder","Hcal/");
  if (prefixME_.substr(prefixME_.size()-1,prefixME_.size())!="/")
    prefixME_.append("/");
  subdir_                = ps.getUntrackedParameter<string>("TaskFolder","RecHitMonitor_Hcal/"); 
  if (subdir_.size()>0 && subdir_.substr(subdir_.size()-1,subdir_.size())!="/")
    subdir_.append("/");
  subdir_=prefixME_+subdir_;
  AllowedCalibTypes_     = ps.getUntrackedParameter<vector<int> > ("AllowedCalibTypes");
  skipOutOfOrderLS_      = ps.getUntrackedParameter<bool>("skipOutOfOrderLS",false);
  NLumiBlocks_           = ps.getUntrackedParameter<int>("NLumiBlocks",4000);
  makeDiagnostics_       = ps.getUntrackedParameter<bool>("makeDiagnostics",false);

  hbheRechitLabel_       = ps.getUntrackedParameter<edm::InputTag>("hbheRechitLabel");
  hoRechitLabel_         = ps.getUntrackedParameter<edm::InputTag>("hoRechitLabel");
  hfRechitLabel_         = ps.getUntrackedParameter<edm::InputTag>("hfRechitLabel");
  l1gtLabel_             = ps.getUntrackedParameter<edm::InputTag>("L1GTLabel"); // should be l1GtUnpack

  L1TriggerBits_         = ps.getUntrackedParameter<std::vector<int> >("L1TriggerBits");
  BPTXBits_              = ps.getUntrackedParameter<std::vector<int> >("BPTXBits");

  energyThreshold_       = ps.getUntrackedParameter<double>("energyThreshold",2);
  HBenergyThreshold_     = ps.getUntrackedParameter<double>("HB_energyThreshold",energyThreshold_);
  HEenergyThreshold_     = ps.getUntrackedParameter<double>("HE_energyThreshold",energyThreshold_);
  HOenergyThreshold_     = ps.getUntrackedParameter<double>("HO_energyThreshold",energyThreshold_);
  HFenergyThreshold_     = ps.getUntrackedParameter<double>("HF_energyThreshold",energyThreshold_);
  
  ETThreshold_       = ps.getUntrackedParameter<double>("ETThreshold",0);
  HBETThreshold_     = ps.getUntrackedParameter<double>("HB_ETThreshold",ETThreshold_);
  HEETThreshold_     = ps.getUntrackedParameter<double>("HE_ETThreshold",ETThreshold_);
  HOETThreshold_     = ps.getUntrackedParameter<double>("HO_ETThreshold",ETThreshold_);
  HFETThreshold_     = ps.getUntrackedParameter<double>("HF_ETThreshold",ETThreshold_);

  timediffThresh_    = ps.getUntrackedParameter<double>("collisiontimediffThresh",10.);

  collisionHFthresh_ = ps.getUntrackedParameter<double>("collisionHFthresh",3.);
  collisionHEthresh_ = ps.getUntrackedParameter<double>("collisionHEthresh",3.);
  collisionHFETthresh_ = ps.getUntrackedParameter<double>("collisionHFETthresh",3.);
  collisionHEETthresh_ = ps.getUntrackedParameter<double>("collisionHEETthresh",3.);

} //constructor

HcalRecHitMonitor::~HcalRecHitMonitor()
{
} //destructor


/* ------------------------------------ */ 


void HcalRecHitMonitor::setup()
{
  HcalBaseDQMonitor::setup();


  if (debug_>0)
    std::cout <<"<HcalRecHitMonitor::setup>  Setting up histograms"<<endl;

  // Can we include this just in the setup, or do we need to get a new logical map with every run?
  HcalLogicalMapGenerator gen;
  logicalMap=new HcalLogicalMap(gen.createMap());

  // RecHit Monitor - specific cfg variables

  if (debug_>1)
    std::cout <<"<HcalRecHitMonitor::setup>  Creating Histograms"<<endl;

  dbe_->setCurrentFolder(subdir_);

  dbe_->setCurrentFolder(subdir_+"rechit_parameters");
  dbe_->setCurrentFolder(subdir_+"rechit_parameters/thresholds");
  MonitorElement* THR=dbe_->bookFloat("HB_Rechit_Energy_Threshold");
  THR->Fill(HBenergyThreshold_);
  THR=dbe_->bookFloat("HE_Rechit_Energy_Threshold");
  THR->Fill(HEenergyThreshold_);
  THR=dbe_->bookFloat("HO_Rechit_Energy_Threshold");
  THR->Fill(HOenergyThreshold_);
  THR=dbe_->bookFloat("HF_Rechit_Energy_Threshold");
  THR->Fill(HFenergyThreshold_);
  THR=dbe_->bookFloat("HB_Rechit_ET_Threshold");
  THR->Fill(HBETThreshold_);
  THR=dbe_->bookFloat("HE_Rechit_ET_Threshold");
  THR->Fill(HEETThreshold_);
  THR=dbe_->bookFloat("HO_Rechit_ET_Threshold");
  THR->Fill(HOETThreshold_);
  THR=dbe_->bookFloat("HF_Rechit_ET_Threshold");
  THR->Fill(HFETThreshold_);
  dbe_->setCurrentFolder(subdir_+"rechit_parameters/collision_parameters");
  THR=dbe_->bookFloat("Minimum_HF_energy_for_luminosityplots");
  THR->Fill(collisionHFthresh_);
  THR=dbe_->bookFloat("Minimum_HF_ET_for_luminosityplots");
  THR->Fill(collisionHFETthresh_);
  THR=dbe_->bookFloat("Minimum_HE_energy_for_luminosityplots");
  THR->Fill(collisionHEthresh_);
  THR=dbe_->bookFloat("Minimum_HE_ET_for_luminosityplots");
  THR->Fill(collisionHEETthresh_);
  THR=dbe_->bookFloat("Maximum_HFM_HFP_time_difference_for_luminosityplots");
  THR->Fill(timediffThresh_);


  // Add parameter MEs here;

  //MonitorElement *me;
  
  dbe_->setCurrentFolder(subdir_+"rechit_info");

  SetupEtaPhiHists(OccupancyByDepth,"RecHit Occupancy","");;
  
  h_rechitieta = dbe_->book1D("HcalRecHitIeta",
			      "Hcal RecHit ieta",
			      83,-41.5,41.5);
  h_rechitiphi = dbe_->book1D("HcalRecHitIphi",
			      "Hcal RecHit iphi",
			      72,0.5,72.5);
  
  h_HFtimedifference = dbe_->book1D("HFweightedtimeDifference",
				    "Energy-Weighted time difference between HF+ and HF-",
				    251,-250.5,250.5);
  h_HEtimedifference = dbe_->book1D("HEweightedtimeDifference",
				    "Energy-Weighted time difference between HE+ and HE-",
				    251,-250.5,250.5);
  h_HFrawtimedifference = dbe_->book1D("HFtimeDifference",
				       "Average Time difference between HF+ and HF-",
				       251,-250.5,250.5);
  h_HErawtimedifference = dbe_->book1D("HEtimeDifference",
				       "Average Time difference between HE+ and HE-",
				       251,-250.5,250.5);

  h_HFenergydifference = dbe_->book1D("HFenergyDifference",
				      "Sum(E_HFPlus - E_HFMinus)/Sum(E_HFPlus + E_HFMinus)",
				      200,-1,1);
  h_HEenergydifference = dbe_->book1D("HEenergyDifference",
				      "Sum(E_HEPlus - E_HEMinus)/Sum(E_HEPlus + E_HEMinus)",
				      200,-1,1);
  h_HFrawenergydifference = dbe_->book1D("HFaverageenergyDifference",
					 "E_HFPlus - E_HFMinus (energy averaged over rechits)",
					 500,-100,100);
  h_HErawenergydifference = dbe_->book1D("HEaverageenergyDifference",
					 "E_HEPlus - E_HEMinus (energy averaged over rechits)",
					 500,-100,100);
  
  h_HFnotBPTXtimedifference = dbe_->book1D("HFnotBPTXweightedtimeDifference",
					   "Energy-Weighted time difference between HF+ and HF-",
					   251,-250.5,250.5);
  h_HEnotBPTXtimedifference = dbe_->book1D("HEnotBPTXweightedtimeDifference",
					   "Energy-Weighted time difference between HE+ and HE-",
					   251,-250.5,250.5);
  h_HFnotBPTXrawtimedifference = dbe_->book1D("HFnotBPTXtimeDifference",
					      "Average Time difference between HF+ and HF-",
					      251,-250.5,250.5);
  h_HEnotBPTXrawtimedifference = dbe_->book1D("HEnotBPTXtimeDifference",
					      "Average Time difference between HE+ and HE-",
					      251,-250.5,250.5);

  h_HFnotBPTXenergydifference = dbe_->book1D("HFnotBPTXenergyDifference",
					     "Sum(E_HFPlus - E_HFMinus)/Sum(E_HFPlus + E_HFMinus)",
					     200,-1,1);
  h_HEnotBPTXenergydifference = dbe_->book1D("HEnotBPTXenergyDifference",
					     "Sum(E_HEPlus - E_HEMinus)/Sum(E_HEPlus + E_HEMinus)",
					     200,-1,1);
  h_HFnotBPTXrawenergydifference = dbe_->book1D("HFnotBPTXaverageenergyDifference",
						"E_HFPlus - E_HFMinus (energy averaged over rechits)",
						500,-100,100);
  h_HEnotBPTXrawenergydifference = dbe_->book1D("HEnotBPTXaverageenergyDifference",
						"E_HEPlus - E_HEMinus (energy averaged over rechits)",
						500,-100,100);
  
  dbe_->setCurrentFolder(subdir_+"luminosityplots");
  h_LumiPlot_EventsPerLS=dbe_->book1D("EventsPerLS",
				      "Number of Events with HF+ and HF- HT>1 GeV vs LS (HFM-HFP time cut)",
				      NLumiBlocks_,0.5,NLumiBlocks_+0.5); 
  h_LumiPlot_EventsPerLS_notimecut=dbe_->book1D("EventsPerLS_notimecut",
						"Number of Events with HF+ and HF- HT>1 GeV (no time cut) vs LS",
						NLumiBlocks_,0.5,NLumiBlocks_+0.5); 

  h_LumiPlot_SumHT_HFPlus_vs_HFMinus = dbe_->book2D("SumHT_plus_minus",
						    "Sum HT for HF+ vs HF-",60,0,30,60,0,30);
  h_LumiPlot_SumEnergy_HFPlus_vs_HFMinus = dbe_->book2D("SumEnergy_plus_minus",
							"Sum Energy for HF+ vs HF-",
							60,0,150,60,0,150);
  h_LumiPlot_timeHFPlus_vs_timeHFMinus = dbe_->book2D("timeHFplus_vs_timeHFminus",
						      "Energy-weighted time average of HF+ vs HF-",
						      120,-120,120,120,-120,120);

  h_LumiPlot_MinTime_vs_MinHT = dbe_->book2D("MinTime_vs_MinHT",
					     "(HF+,HF-) Min Time vs Min HT",
					     100,0,10,80,-40,40);
  h_LumiPlot_LS_allevents = dbe_->book1D("LS_allevents",
					 "LS # of all events",
					 NLumiBlocks_,0.5,NLumiBlocks_+0.5);

  h_LumiPlot_BX_allevents = dbe_->book1D("BX_allevents",
					 "BX # of all events",
					 3600,0,3600);
  h_LumiPlot_BX_goodevents = dbe_->book1D("BX_goodevents",
					  "BX # of good events (HFM & HFP HT>1 & HFM-HFP time cut)",
					  3600,0,3600);
  h_LumiPlot_BX_goodevents_notimecut = dbe_->book1D("BX_goodevents_notimecut",
						    "BX # of good events (HFM,HFP HT>1, no time cut)",
						    3600,0,3600);
  
  dbe_->setCurrentFolder(subdir_+"rechit_info/sumplots");
  SetupEtaPhiHists(SumEnergyByDepth,"RecHit Summed Energy","GeV");
  SetupEtaPhiHists(SqrtSumEnergy2ByDepth,"RecHit Sqrt Summed Energy2","GeV");
  SetupEtaPhiHists(SumTimeByDepth,"RecHit Summed Time","nS");

  dbe_->setCurrentFolder(subdir_+"rechit_info_threshold");
  h_rechitieta_thresh = dbe_->book1D("HcalRecHitIeta_thresh",
			      "Hcal RecHit ieta above energy and ET threshold",
			      83,-41.5,41.5);
  h_rechitiphi_thresh = dbe_->book1D("HcalRecHitIphi_thresh",
			      "Hcal RecHit iphi above energy and ET threshold",
			      72,0.5,72.5);
  

  SetupEtaPhiHists(OccupancyThreshByDepth,"Above Threshold RecHit Occupancy","");

  dbe_->setCurrentFolder(subdir_+"rechit_info_threshold/sumplots");
  SetupEtaPhiHists(SumEnergyThreshByDepth,"Above Threshold RecHit Summed Energy","GeV");
  SetupEtaPhiHists(SumTimeThreshByDepth,"Above Threshold RecHit Summed Time","nS");
  
  dbe_->setCurrentFolder(subdir_+"AnomalousCellFlags");// HB Flag Histograms

  h_HF_FlagCorr=dbe_->book2D("HF_FlagCorrelation",
			     "HF LongShort vs. DigiTime flags; DigiTime; LongShort", 
			     2,-0.5,1.5,2,-0.5,1.5);
  h_HF_FlagCorr->setBinLabel(1,"OFF",1);
  h_HF_FlagCorr->setBinLabel(2,"ON",1);
  h_HF_FlagCorr->setBinLabel(1,"OFF",2);
  h_HF_FlagCorr->setBinLabel(2,"ON",2);

  h_HBHE_FlagCorr=dbe_->book2D("HBHE_FlagCorrelation",
			       "HBHE HpdHitMultiplicity vs. PulseShape flags; PulseShape; HpdHitMultiplicity", 
			       2,-0.5,1.5,2,-0.5,1.5);
  h_HBHE_FlagCorr->setBinLabel(1,"OFF",1);
  h_HBHE_FlagCorr->setBinLabel(2,"ON",1);
  h_HBHE_FlagCorr->setBinLabel(1,"OFF",2);
  h_HBHE_FlagCorr->setBinLabel(2,"ON",2);

  h_FlagMap_HPDMULT=dbe_->book2D("FlagMap_HPDMULT",
				 "RBX Map of HBHEHpdHitMultiplicity Flags;RBX;RM",
				  72,-0.5,71.5,4,0.5,4.5);
  h_FlagMap_PULSESHAPE=dbe_->book2D("FlagMap_PULSESHAPE",
				    "RBX Map of HBHEPulseShape Flags;RBX;RM",
				  72,-0.5,71.5,4,0.5,4.5);
  h_FlagMap_DIGITIME=dbe_->book2D("FlagMap_DIGITIME",
				  "RBX Map of HFDigiTime Flags;RBX;RM",
				  24,131.5,155.5,4,0.5,4.5);
  h_FlagMap_LONGSHORT=dbe_->book2D("FlagMap_LONGSHORT",
				   "RBX Map of HFLongShort Flags;RBX;RM",
				   24,131.5,155.5,4,0.5,4.5);

  h_FlagMap_TIMEADD=dbe_->book2D("FlagMap_TIMEADD",
				 "RBX Map of Timing Added Flags;RBX;RM",
				   156,-0.5,155.5,4,0.5,4.5);
  h_FlagMap_TIMESUBTRACT=dbe_->book2D("FlagMap_TIMESUBTRACT",
				      "RBX Map of Timing Subtracted Flags;RBX;RM",
				   156,-0.5,155.5,4,0.5,4.5);
  h_FlagMap_TIMEERROR=dbe_->book2D("FlagMap_TIMEERROR",
				   "RBX Map of Timing Error Flags;RBX;RM",
				   156,-0.5,155.5,4,0.5,4.5);

  h_HBflagcounter=dbe_->book1D("HBflags","HB flags",32,-0.5,31.5);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::HBHEHpdHitMultiplicity, "HpdHitMult",1);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::HBHEPulseShape, "PulseShape",1);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::HSCP_R1R2, "HSCP R1R2",1);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::HSCP_FracLeader, "HSCP FracLeader",1);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::HSCP_OuterEnergy, "HSCP OuterEnergy",1);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::HSCP_ExpFit, "HSCP ExpFit",1);
  // 2-bit timing counter
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::HBHETimingTrustBits,"TimingTrust1",1);
  h_HBflagcounter->setBinLabel(2+HcalCaloFlagLabels::HBHETimingTrustBits,"TimingTrust2",1);
  //3-bit timing shape cut
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::HBHETimingShapedCutsBits,"TimingShape1",1);
  h_HBflagcounter->setBinLabel(2+HcalCaloFlagLabels::HBHETimingShapedCutsBits,"TimingShape2",1);
  h_HBflagcounter->setBinLabel(3+HcalCaloFlagLabels::HBHETimingShapedCutsBits,"TimingShape3",1);

  // common flags
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingSubtractedBit, "Subtracted",1);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingAddedBit, "Added",1);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingErrorBit, "TimingError",1);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::ADCSaturationBit, "Saturation",1);
  h_HBflagcounter->setBinLabel(1+HcalCaloFlagLabels::Fraction2TS,"Frac2TS",1);

  // HE Flag Histograms
  h_HEflagcounter=dbe_->book1D("HEflags","HE flags",32,-0.5,31.5);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::HBHEHpdHitMultiplicity, "HpdHitMult",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::HBHEPulseShape, "PulseShape",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::HSCP_R1R2, "HSCP R1R2",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::HSCP_FracLeader, "HSCP FracLeader",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::HSCP_OuterEnergy, "HSCP OuterEnergy",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::HSCP_ExpFit, "HSCP ExpFit",1);
  // 2-bit timing counter
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::HBHETimingTrustBits,"TimingTrust1",1);
  h_HEflagcounter->setBinLabel(2+HcalCaloFlagLabels::HBHETimingTrustBits,"TimingTrust2",1);
  //3-bit timing shape cut
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::HBHETimingShapedCutsBits,"TimingShape1",1);
  h_HEflagcounter->setBinLabel(2+HcalCaloFlagLabels::HBHETimingShapedCutsBits,"TimingShape2",1);
  h_HEflagcounter->setBinLabel(3+HcalCaloFlagLabels::HBHETimingShapedCutsBits,"TimingShape3",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingSubtractedBit, "Subtracted",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingAddedBit, "Added",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingErrorBit, "TimingError",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::ADCSaturationBit, "Saturation",1);
  h_HEflagcounter->setBinLabel(1+HcalCaloFlagLabels::Fraction2TS,"Frac2TS",1);

  // HO Flag Histograms
  h_HOflagcounter=dbe_->book1D("HOflags","HO flags",32,-0.5,31.5);
  h_HOflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingSubtractedBit, "Subtracted",1);
  h_HOflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingAddedBit, "Added",1);
  h_HOflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingErrorBit, "TimingError",1);
  h_HOflagcounter->setBinLabel(1+HcalCaloFlagLabels::ADCSaturationBit, "Saturation",1);
  h_HOflagcounter->setBinLabel(1+HcalCaloFlagLabels::Fraction2TS,"Frac2TS",1);

  // HF Flag Histograms
  h_HFflagcounter=dbe_->book1D("HFflags","HF flags",32,-0.5,31.5);
  h_HFflagcounter->setBinLabel(1+HcalCaloFlagLabels::HFLongShort, "LongShort",1);
  h_HFflagcounter->setBinLabel(1+HcalCaloFlagLabels::HFDigiTime, "DigiTime",1);
  h_HFflagcounter->setBinLabel(1+HcalCaloFlagLabels::HFTimingTrustBits,"TimingTrust1",1);
  h_HFflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingSubtractedBit, "Subtracted",1);
  h_HFflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingAddedBit, "Added",1);
  h_HFflagcounter->setBinLabel(1+HcalCaloFlagLabels::TimingErrorBit, "TimingError",1);
  h_HFflagcounter->setBinLabel(1+HcalCaloFlagLabels::ADCSaturationBit, "Saturation",1);
  h_HFflagcounter->setBinLabel(1+HcalCaloFlagLabels::Fraction2TS,"Frac2TS",1);

  h_HBflagcounter->getTH1F()->LabelsOption("v");
  h_HEflagcounter->getTH1F()->LabelsOption("v");
  h_HOflagcounter->getTH1F()->LabelsOption("v");
  h_HFflagcounter->getTH1F()->LabelsOption("v");
  
  // hb
  dbe_->setCurrentFolder(subdir_+"diagnostics/hb");

  h_HBsizeVsLS=dbe_->bookProfile("HBRecHitsVsLB","HB RecHits vs Luminosity Block",
				  NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				  0,10000);

  h_HBTime=dbe_->book1D("HB_time","HB RecHit Time",
			int(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN),RECHITMON_TIME_MIN,RECHITMON_TIME_MAX);
  h_HBThreshTime=dbe_->book1D("HB_time_thresh", 
			      "HB RecHit Time Above Threshold",
			      int(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN),RECHITMON_TIME_MIN,RECHITMON_TIME_MAX);
  h_HBOccupancy=dbe_->book1D("HB_occupancy",
			     "HB RecHit Occupancy",260,-0.5,2599.5);
  h_HBThreshOccupancy=dbe_->book1D("HB_occupancy_thresh",
				   "HB RecHit Occupancy Above Threshold",260,-0.5,2599.5);
  
  //he
  dbe_->setCurrentFolder(subdir_+"diagnostics/he");

  h_HEsizeVsLS=dbe_->bookProfile("HERecHitsVsLB","HE RecHits vs Luminosity Block",
				  NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				  0,10000);
	
  h_HETime=dbe_->book1D("HE_time","HE RecHit Time",
			int(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN),RECHITMON_TIME_MIN,RECHITMON_TIME_MAX);
  h_HEThreshTime=dbe_->book1D("HE_time_thresh", 
			      "HE RecHit Time Above Threshold",
			      int(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN),RECHITMON_TIME_MIN,RECHITMON_TIME_MAX);
  h_HEOccupancy=dbe_->book1D("HE_occupancy","HE RecHit Occupancy",260,-0.5,2599.5);
  h_HEThreshOccupancy=dbe_->book1D("HE_occupancy_thresh",
				   "HE RecHit Occupancy Above Threshold",260,-0.5,2599.5);
  
  // ho
  dbe_->setCurrentFolder(subdir_+"diagnostics/ho");	

  h_HOsizeVsLS=dbe_->bookProfile("HORecHitsVsLB","HO RecHits vs Luminosity Block",
				  NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				  0,10000);
  h_HOTime=dbe_->book1D("HO_time",
			"HO RecHit Time",
			int(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN),RECHITMON_TIME_MIN,RECHITMON_TIME_MAX);
  h_HOThreshTime=dbe_->book1D("HO_time_thresh", 
			      "HO RecHit Time Above Threshold",
			      int(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN),RECHITMON_TIME_MIN,RECHITMON_TIME_MAX);
  h_HOOccupancy=dbe_->book1D("HO_occupancy",
			     "HO RecHit Occupancy",217,-0.5,2169.5);
  h_HOThreshOccupancy=dbe_->book1D("HO_occupancy_thresh",
				   "HO RecHit Occupancy Above Threshold",217,-0.5,2169.5);
  
  // hf
  dbe_->setCurrentFolder(subdir_+"diagnostics/hf");
  h_HFsizeVsLS=dbe_->bookProfile("HFRecHitsVsLB",
				 "HF RecHits vs Luminosity Block",
				  NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				  0,10000);	
  h_HFTime=dbe_->book1D("HF_time","HF RecHit Time",
			int(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN),RECHITMON_TIME_MIN,RECHITMON_TIME_MAX);
  h_HFThreshTime=dbe_->book1D("HF_time_thresh", 
			      "HF RecHit Time Above Threshold",
			      int(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN),RECHITMON_TIME_MIN,RECHITMON_TIME_MAX);
  h_HFOccupancy=dbe_->book1D("HF_occupancy","HF RecHit Occupancy",173,-0.5,1729.5);
  h_HFThreshOccupancy=dbe_->book1D("HF_occupancy_thresh",
				   "HF RecHit Occupancy Above Threshold",
				   173,-0.5,1729.5);

  // clear all counters, reset histograms
  this->zeroCounters();
  this->reset();

  return;
} //void HcalRecHitMonitor::setup(...)

void HcalRecHitMonitor::beginRun(const edm::Run& run, const edm::EventSetup& c)
{

 if (debug_>0) std::cout <<"HcalRecHitMonitor::beginRun():  task =  '"<<subdir_<<"'"<<endl;
 HcalBaseDQMonitor::beginRun(run, c);
 if (tevt_==0) // create histograms, if they haven't been created already
   this->setup();
 // Clear histograms at the start of each run if not merging runs
 if (mergeRuns_==false)
   this->reset();

  return;
  
} //void HcalRecHitMonitor::beginRun(...)


void HcalRecHitMonitor::endRun(const edm::Run& run, const edm::EventSetup& c)
{
  if (debug_>0) std::cout <<"HcalRecHitMonitor::endRun():  task =  '"<<subdir_<<"'"<<endl;

  //Any special fill calls needed?  Shouldn't be necessary; last endLuminosityBlock should do necessary fills
} // void HcalRecHitMonitor::endrun(...)


/* --------------------------- */

void HcalRecHitMonitor::reset()
{
  h_HBsizeVsLS->Reset();
  h_HEsizeVsLS->Reset();
  h_HOsizeVsLS->Reset();
  h_HFsizeVsLS->Reset();
  h_HBTime->Reset();
  h_HBThreshTime->Reset();
  h_HBOccupancy->Reset();
  h_HBThreshOccupancy->Reset();
  h_HETime->Reset();
  h_HEThreshTime->Reset();
  h_HEOccupancy->Reset();
  h_HEThreshOccupancy->Reset();
  h_HOTime->Reset();
  h_HOThreshTime->Reset();
  h_HOOccupancy->Reset();
  h_HOThreshOccupancy->Reset();
  h_HFTime->Reset();
  h_HFThreshTime->Reset();
  h_HFOccupancy->Reset();
  h_HFThreshOccupancy->Reset();
  h_HBflagcounter->Reset();
  h_HEflagcounter->Reset();
  h_HOflagcounter->Reset();
  h_HFflagcounter->Reset();
  h_FlagMap_HPDMULT->Reset();
  h_FlagMap_PULSESHAPE->Reset();
  h_FlagMap_DIGITIME->Reset();
  h_FlagMap_LONGSHORT->Reset();
  h_FlagMap_TIMEADD->Reset();
  h_FlagMap_TIMESUBTRACT->Reset();
  h_FlagMap_TIMEERROR->Reset();
  h_HF_FlagCorr->Reset();
  h_HBHE_FlagCorr->Reset();
  h_HFtimedifference->Reset();
  h_HFenergydifference->Reset();
  h_HEtimedifference->Reset();
  h_HEenergydifference->Reset();
  h_HFrawenergydifference->Reset();
  h_HErawenergydifference->Reset();
  h_HFrawtimedifference->Reset();
  h_HErawtimedifference->Reset();
  h_HFnotBPTXtimedifference->Reset();
  h_HFnotBPTXenergydifference->Reset();
  h_HEnotBPTXtimedifference->Reset();
  h_HEnotBPTXenergydifference->Reset();
  h_HFnotBPTXrawenergydifference->Reset();
  h_HEnotBPTXrawenergydifference->Reset();
  h_HFnotBPTXrawtimedifference->Reset();
  h_HEnotBPTXrawtimedifference->Reset();
  h_LumiPlot_LS_allevents->Reset();
  h_LumiPlot_EventsPerLS->Reset();
  h_LumiPlot_EventsPerLS_notimecut->Reset();
  h_LumiPlot_SumHT_HFPlus_vs_HFMinus->Reset();
  h_LumiPlot_timeHFPlus_vs_timeHFMinus->Reset();
  h_LumiPlot_SumEnergy_HFPlus_vs_HFMinus->Reset();
  h_LumiPlot_BX_allevents->Reset();
  h_LumiPlot_BX_goodevents->Reset();
  h_LumiPlot_BX_goodevents_notimecut->Reset();
  h_LumiPlot_MinTime_vs_MinHT->Reset();
}  


void HcalRecHitMonitor::endJob()
{
  if (!enableCleanup_) return;
  HcalBaseDQMonitor::cleanup();
  this->cleanup();
}


/* --------------------------------- */

void HcalRecHitMonitor::cleanup()
{
  //Add code to clean out subdirectories
  if (!enableCleanup_) return;
  if (dbe_)
    {
      dbe_->setCurrentFolder(subdir_); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"rechit_parameters"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"rechit_info"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"rechit_info/sumplots"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"rechit_info_threshold"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"rechit_info_threshold/sumplots"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"luminosityplots"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"AnomalousCellFlags"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"diagnostics/hb"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"diagnostics/he"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"diagnostics/ho"); dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"diagnostics/hf"); dbe_->removeContents();
    }
  return;
} // void HcalRecHitMonitor::clearME()

/* -------------------------------- */

void HcalRecHitMonitor::analyze(edm::Event const&e, edm::EventSetup const&s)
{
  if (debug_>0)  std::cout <<"HcalRecHitMonitor::analyze"<<std::endl;

  if (!IsAllowedCalibType()) return;
  if (LumiInOrder(e.luminosityBlock())==false) return;

  // Get objects
  edm::Handle<HBHERecHitCollection> hbhe_rechit;
  edm::Handle<HORecHitCollection> ho_rechit;
  edm::Handle<HFRecHitCollection> hf_rechit;

  if (!(e.getByLabel(hbheRechitLabel_,hbhe_rechit)))
    {
      LogWarning("HcalHotCellMonitor")<< hbheRechitLabel_<<" hbhe_rechit not available";
      return;
    }

  if (!(e.getByLabel(hfRechitLabel_,hf_rechit)))
    {
      LogWarning("HcalHotCellMonitor")<< hfRechitLabel_<<" hf_rechit not available";
      return;
    }
  
  if (!(e.getByLabel(hoRechitLabel_,ho_rechit)))
    {
      LogWarning("HcalHotCellMonitor")<< hoRechitLabel_<<" ho_rechit not available";
      return;
    }


  h_LumiPlot_LS_allevents->Fill(currentLS);
  h_LumiPlot_BX_allevents->Fill(e.bunchCrossing());
  processEvent(*hbhe_rechit, *ho_rechit, *hf_rechit, e.bunchCrossing(), e);

  HcalBaseDQMonitor::analyze(e,s);
} // void HcalRecHitMonitor::analyze()


void HcalRecHitMonitor::processEvent(const HBHERecHitCollection& hbHits,
				     const HORecHitCollection& hoHits,
				     const HFRecHitCollection& hfHits,
				     int BCN,
				     const edm::Event & iEvent
				     )
{

  
  if (debug_>1) std::cout <<"<HcalRecHitMonitor::processEvent> Processing event..."<<endl;
  /*
    bool passedHLT=false;
  edm::Handle<edm::TriggerResults> hltResults;
  iEvent.getByLabel("HLT",hltResults);
  const edm::TriggerNames & triggerNames = iEvent.triggerNames(*hltResults);
  for (unsigned int i=0;i<hltResults->size();++i)
    {
      string trigName = triggerNames.triggerName(i);
      if (trigName!="HLT_L1_HFtech")
	continue;
      if (hltResults->accept(i))
	{
	  passedHLT=true;
	  break;
	}
    }
  */

  bool BPTX=false;
  bool passedL1=false;

  if (Online_)
    {
      bool BPTX=false;
      edm::Handle<L1GlobalTriggerReadoutRecord> gtRecord;
      if (!iEvent.getByLabel(l1gtLabel_,gtRecord))
	{
	  edm::LogWarning("HcalMonitorTasks")<<" HcalRecHitMonitor:  l1GtUnpack L1GlobalTriggerReadoutRecord not found";
	  return;
	}

      bool passedL1=false;
      if (gtRecord.isValid())
	{
	  const DecisionWord dWord = gtRecord->decisionWord();
	  const TechnicalTriggerWord tWord = gtRecord->technicalTriggerWord();
	  for (std::vector<int>::const_iterator i=BPTXBits_.begin();i!=BPTXBits_.end();++i)
	    {
	      if (!(tWord.at(*i))) continue;
	      BPTX=true;
	      break;
	    }
	  for (std::vector<int>::const_iterator i=L1TriggerBits_.begin();i!=L1TriggerBits_.end();++i)
	    {
	      if (!(tWord.at(*i))) continue;
	      passedL1=true;
	      break;
	    }
	}
      if (debug_>2 && passedL1)  std::cout <<"\t<HcalRecHitMonitor::processEvent>  Passed L1 trigger  "<<std::endl;
      if (debug_>2 && BPTX)  std::cout <<"\t<HcalRecHitMonitor::processEvent>  Passed BPTX trigger  "<<std::endl;

    }

  processEvent_rechit(hbHits, hoHits, hfHits,passedL1,BPTX,BCN);

  return;
} // void HcalRecHitMonitor::processEvent(...)


/* --------------------------------------- */


void HcalRecHitMonitor::processEvent_rechit( const HBHERecHitCollection& hbheHits,
					     const HORecHitCollection& hoHits,
					     const HFRecHitCollection& hfHits,
					     bool passedHLT,
					     bool BPTX,
					     int BCN)
{
  // Gather rechit info
 
  //const float area[]={0.111,0.175,0.175,0.175,0.175,0.175,0.174,0.178,0.172,0.175,0.178,0.346,0.604};

  if (debug_>1) std::cout <<"<HcalRecHitMonitor::processEvent_rechitenergy> Processing rechits..."<<endl;
  
  // loop over HBHE
  
  int     hbocc=0;
  int     heocc=0;
  int     hboccthresh=0;
  int     heoccthresh=0;

  double en_HFP=0, en_HFM=0, en_HEP=0, en_HEM=0;
  double time_HFP=0, time_HFM=0, time_HEP=0, time_HEM=0;
  double rawtime_HFP=0, rawtime_HFM=0, rawtime_HEP=0, rawtime_HEM=0;
  int hepocc=0, hemocc=0, hfpocc=0, hfmocc=0;

  for (unsigned int i=0;i<4;++i)
    {
      OccupancyByDepth.depth[i]->update();
      OccupancyThreshByDepth.depth[i]->update();
      SumEnergyByDepth.depth[i]->update();
      SqrtSumEnergy2ByDepth.depth[i]->update();
      SumTimeByDepth.depth[i]->update();
    }
    
  h_HBflagcounter->update();
  h_HEflagcounter->update();
  h_HFflagcounter->update();
  h_HOflagcounter->update();
  
  
  for (HBHERecHitCollection::const_iterator HBHEiter=hbheHits.begin(); HBHEiter!=hbheHits.end(); ++HBHEiter) 
    { // loop over all hits
      float en = HBHEiter->energy();
      float ti = HBHEiter->time();
      HcalDetId id(HBHEiter->detid().rawId());
      int ieta = id.ieta();
      int iphi = id.iphi();
      int depth = id.depth();
      HcalSubdetector subdet = id.subdet();
      double fEta=fabs(0.5*(theHFEtaBounds[abs(ieta)-29]+theHFEtaBounds[abs(ieta)-28]));

      int calcEta = CalcEtaBin(subdet,ieta,depth);
      int rbxindex=logicalMap->getHcalFrontEndId(HBHEiter->detid()).rbxIndex();
      int rm= logicalMap->getHcalFrontEndId(HBHEiter->detid()).rm();


      h_HBHE_FlagCorr->Fill(HBHEiter->flagField(HcalCaloFlagLabels::HBHEPulseShape),HBHEiter->flagField(HcalCaloFlagLabels::HBHEHpdHitMultiplicity)); 

      if (subdet==HcalBarrel)
	{
	  //Looping over HB searching for flags --- cris
	  for (int f=0;f<32;f++)
	    {
	      // Let's display HSCP just to see if these bits are set
	      /*
	       if(f == HcalCaloFlagLabels::HSCP_R1R2)
		continue;
              if(f == HcalCaloFlagLabels::HSCP_FracLeader)
                continue;
              if(f == HcalCaloFlagLabels::HSCP_OuterEnergy)
                continue;
              if(f == HcalCaloFlagLabels::HSCP_ExpFit)
                continue;
	      */
	      if (HBHEiter->flagField(f))
		HBflagcounter_[f]++;
	    }

	  if (HBHEiter->flagField(HcalCaloFlagLabels::HBHEHpdHitMultiplicity))
	    h_FlagMap_HPDMULT->Fill(rbxindex,rm);
	  if (HBHEiter->flagField(HcalCaloFlagLabels::HBHEPulseShape))
	    h_FlagMap_PULSESHAPE->Fill(rbxindex,rm);
	  if (HBHEiter->flagField(HcalCaloFlagLabels::TimingSubtractedBit))
	    h_FlagMap_TIMESUBTRACT->Fill(rbxindex,rm);
	  else if (HBHEiter->flagField(HcalCaloFlagLabels::TimingAddedBit))
	    h_FlagMap_TIMEADD->Fill(rbxindex,rm);
	  else if (HBHEiter->flagField(HcalCaloFlagLabels::TimingErrorBit))
	    h_FlagMap_TIMEERROR->Fill(rbxindex,rm);
	  ++occupancy_[calcEta][iphi-1][depth-1];
	  energy_[calcEta][iphi-1][depth-1]+=en;
          energy2_[calcEta][iphi-1][depth-1]+=pow(en,2);
	  time_[calcEta][iphi-1][depth-1]+=ti;
	  if (en>=HBenergyThreshold_ && en/cosh(fEta)>HBETThreshold_) 
	    {
	      ++occupancy_thresh_[calcEta][iphi-1][depth-1];
	      energy_thresh_[calcEta][iphi-1][depth-1]+=en;
	      time_thresh_[calcEta][iphi-1][depth-1]+=ti;
	    }

	      ++hbocc;
	      if (ti<RECHITMON_TIME_MIN || ti>RECHITMON_TIME_MAX)
		h_HBTime->Fill(ti);
	      else
		++HBtime_[int(ti-RECHITMON_TIME_MIN)];
	      if (en>=HBenergyThreshold_)
		{
		  ++hboccthresh;
		  if (ti<RECHITMON_TIME_MIN || ti>RECHITMON_TIME_MAX)
		    h_HBThreshTime->Fill(ti);
		  else
		    ++HBtime_thresh_[int(ti-RECHITMON_TIME_MIN)];
		} // if (en>=HBenergyThreshold_)
	} // if (id.subdet()==HcalBarrel)

      else if (subdet==HcalEndcap)
	{
	  if (en>collisionHEthresh_ && en/cosh(fEta)>collisionHEETthresh_ && ieta>0)
	    {
	      en_HEP+=en;
	      time_HEP+=ti*en;
	      rawtime_HEP+=ti;
	      hepocc++;
	    }
	  else if (en>collisionHEthresh_ && en/cosh(fEta)>collisionHEETthresh_ && ieta<0)
	    {
	      en_HEM+=en;
	      time_HEM+=ti*en;
	      rawtime_HEM+=ti;
	      hemocc++;
	    }

	  //Looping over HE searching for flags --- cris
	  for (int f=0;f<32;f++)
            {
              if (HBHEiter->flagField(f))
                HEflagcounter_[f]++;
            }

	  if (HBHEiter->flagField(HcalCaloFlagLabels::HBHEHpdHitMultiplicity))
	    h_FlagMap_HPDMULT->Fill(rbxindex,rm);
	  if (HBHEiter->flagField(HcalCaloFlagLabels::HBHEPulseShape))
	    h_FlagMap_PULSESHAPE->Fill(rbxindex,rm);
	  if (HBHEiter->flagField(HcalCaloFlagLabels::TimingSubtractedBit))
	    h_FlagMap_TIMESUBTRACT->Fill(rbxindex,rm);
	  else if (HBHEiter->flagField(HcalCaloFlagLabels::TimingAddedBit))
	    h_FlagMap_TIMEADD->Fill(rbxindex,rm);
	  else if (HBHEiter->flagField(HcalCaloFlagLabels::TimingErrorBit))
	    h_FlagMap_TIMEERROR->Fill(rbxindex,rm);


	  ++occupancy_[calcEta][iphi-1][depth-1];
	  energy_[calcEta][iphi-1][depth-1]+=en;
          energy2_[calcEta][iphi-1][depth-1]+=pow(en,2);
	  time_[calcEta][iphi-1][depth-1]+=ti;
	  if (en>=HEenergyThreshold_ && en/cosh(fEta)>HBETThreshold_)
	    {
	      ++occupancy_thresh_[calcEta][iphi-1][depth-1];
	      energy_thresh_[calcEta][iphi-1][depth-1]+=en;
	      time_thresh_[calcEta][iphi-1][depth-1]+=ti;
	    }
	    {
	      ++heocc;
	      if (ti<-100 || ti>200)
		h_HETime->Fill(ti);
	      else
		++HEtime_[int(ti+100)];
	      if (en>=HEenergyThreshold_)
		{
		  ++heoccthresh;
		  if (ti<-100 || ti>200)
		    h_HEThreshTime->Fill(ti);
		  else
		    ++HEtime_thresh_[int(ti+100)];
		} // if (en>=HEenergyThreshold_)
	    }


	} // else if (id.subdet()==HcalEndcap)
     
    } //for (HBHERecHitCollection::const_iterator HBHEiter=...)

  if (Online_ && passedHLT && BPTX==true)
    {
      if (hepocc >0 && hemocc>0)
	{
	  h_HErawenergydifference->Fill(en_HEP/hepocc-en_HEM/hemocc);
	  h_HErawtimedifference->Fill(rawtime_HEP/hepocc-rawtime_HEM/hemocc);
	}
      // fill overflow, underflow bins if one side unoccupied?  Try it for time plots only right now
      // for now, fill upper, lower bins, not over/underflow
      else if (hepocc>0)
	h_HErawtimedifference->Fill(250000);
      else if (hemocc>0)
	h_HErawtimedifference->Fill(-250000);
      
      if (en_HEP !=0 && en_HEM != 0)
	{
	  h_HEtimedifference->Fill((time_HEP/en_HEP)-(time_HEM/en_HEM));
	  h_HEenergydifference->Fill((en_HEP-en_HEM)/(en_HEP+en_HEM));
	}
    } // if passedHLT

  else if (Online_ && passedHLT && BPTX==false)
    {
      if (hepocc >0 && hemocc>0)
	{
	  h_HEnotBPTXrawenergydifference->Fill(en_HEP/hepocc-en_HEM/hemocc);
	  h_HEnotBPTXrawtimedifference->Fill(rawtime_HEP/hepocc-rawtime_HEM/hemocc);
	}
      // fill overflow, underflow bins if one side unoccupied?  Try it for time plots only right now
      // for now, fill upper, lower bins, not over/underflow
      else if (hepocc>0)
	h_HEnotBPTXrawtimedifference->Fill(250000);
      else if (hemocc>0)
	h_HEnotBPTXrawtimedifference->Fill(-250000);
      
      if (en_HEP !=0 && en_HEM != 0)
	{
	  h_HEnotBPTXtimedifference->Fill((time_HEP/en_HEP)-(time_HEM/en_HEM));
	  h_HEnotBPTXenergydifference->Fill((en_HEP-en_HEM)/(en_HEP+en_HEM));
	}
    } // if passedHLT
  
  ++HB_occupancy_[hbocc/10];
  ++HE_occupancy_[heocc/10];
  ++HB_occupancy_thresh_[hboccthresh/10];
  ++HE_occupancy_thresh_[heoccthresh/10];
  h_HBsizeVsLS->Fill(currentLS,hbocc);
  h_HEsizeVsLS->Fill(currentLS,heocc);

  // loop over HO

  h_HOsizeVsLS->Fill(currentLS,hoHits.size());
  int hoocc=0;
  int hooccthresh=0;
  for (HORecHitCollection::const_iterator HOiter=hoHits.begin(); HOiter!=hoHits.end(); ++HOiter) 
    { // loop over all hits
      float en = HOiter->energy();
      float ti = HOiter->time();
      
      HcalDetId id(HOiter->detid().rawId());
      int ieta = id.ieta();
      int iphi = id.iphi();
      int depth = id.depth();
      int calcEta = CalcEtaBin(HcalOuter,ieta,depth);
      double fEta=fabs(0.5*(theHFEtaBounds[abs(ieta)-29]+theHFEtaBounds[abs(ieta)-28]));
      int rbxindex=logicalMap->getHcalFrontEndId(HOiter->detid()).rbxIndex();
      int rm= logicalMap->getHcalFrontEndId(HOiter->detid()).rm();
      
      if (HOiter->flagField(HcalCaloFlagLabels::TimingSubtractedBit))
	h_FlagMap_TIMESUBTRACT->Fill(rbxindex,rm);
      else if (HOiter->flagField(HcalCaloFlagLabels::TimingAddedBit))
	h_FlagMap_TIMEADD->Fill(rbxindex,rm);
      else if (HOiter->flagField(HcalCaloFlagLabels::TimingErrorBit))
	h_FlagMap_TIMEERROR->Fill(rbxindex,rm);
      
      
      //Looping over HO searching for flags --- cris
      for (int f=0;f<32;f++)
	{
	  if (HOiter->flagField(f))
	    HOflagcounter_[f]++;
	}
      
      ++occupancy_[calcEta][iphi-1][depth-1];
      energy_[calcEta][iphi-1][depth-1]+=en;
      energy2_[calcEta][iphi-1][depth-1]+=pow(en,2);
      time_[calcEta][iphi-1][depth-1]+=ti;
      
      if (en>=HOenergyThreshold_  && en/cosh(fEta)>HOETThreshold_)
	{
	  ++occupancy_thresh_[calcEta][iphi-1][depth-1];
	  energy_thresh_[calcEta][iphi-1][depth-1]+=en;
	  time_thresh_[calcEta][iphi-1][depth-1]+=ti;
	}
      {
	++hoocc;
	if (ti<-100 || ti>200)
	  h_HOTime->Fill(ti);
	else
	  ++HOtime_[int(ti+100)];
	if (en>=HOenergyThreshold_)
	  {
	    ++hooccthresh;
	    if (ti<-100 || ti>200)
	      h_HOThreshTime->Fill(ti);
	    else
	      ++HOtime_thresh_[int(ti+100)];
	  } // if (en>=HOenergyThreshold_)
      } 
    } // loop over all HO hits
  {
    ++HO_occupancy_[hoocc/10];
    ++HO_occupancy_thresh_[hooccthresh/10];
  }
 
  // loop over HF
  h_HFsizeVsLS->Fill(currentLS,hfHits.size());

  double EtPlus =0, EtMinus=0;
  double tPlus=0, tMinus=0;
  double ePlus=0, eMinus=0;

  int hfocc=0;
  int hfoccthresh=0;
  for (HFRecHitCollection::const_iterator HFiter=hfHits.begin(); HFiter!=hfHits.end(); ++HFiter) 
    { // loop over all hits
      float en = HFiter->energy();
      float ti = HFiter->time();

      HcalDetId id(HFiter->detid().rawId());
	 
      int ieta = id.ieta();
      int iphi = id.iphi();
      int depth = id.depth();

      double fEta=fabs(0.5*(theHFEtaBounds[abs(ieta)-29]+theHFEtaBounds[abs(ieta)-28]));
      int calcEta = CalcEtaBin(HcalForward,ieta,depth);

      if (ieta>0)
	{
	  if (en>0)
	    {
	      tPlus+=en*ti;
	      ePlus+=en;
	    }
	  EtPlus+=en/cosh(fEta);
	}
      else if (ieta<0)
	{
	  if (en>0)
	    {
	      tMinus+=en*ti;
	      eMinus+=en;
	    }
	  EtMinus+=en/cosh(fEta);
	}

      if (en>collisionHFthresh_ && en/cosh(fEta)>collisionHFETthresh_ && ieta>0)
	{
	  en_HFP+=en;
	  time_HFP+=ti*en;
	  rawtime_HFP+=ti;
	  hfpocc++;
	}
      else if (en>collisionHFthresh_ && en/cosh(fEta)>collisionHFETthresh_ && ieta<0)
	{
	  en_HFM+=en;
	  time_HFM+=ti*en;
	  rawtime_HFM+=ti;
	  hfmocc++;
	}
	 
      int rbxindex=logicalMap->getHcalFrontEndId(HFiter->detid()).rbxIndex();
      int rm= logicalMap->getHcalFrontEndId(HFiter->detid()).rm(); 
	 
      h_HF_FlagCorr->Fill(HFiter->flagField(HcalCaloFlagLabels::HFDigiTime),HFiter->flagField(HcalCaloFlagLabels::HFLongShort)); 
      if (HFiter->flagField(HcalCaloFlagLabels::TimingSubtractedBit))
	h_FlagMap_TIMESUBTRACT->Fill(rbxindex,rm);
      else if (HFiter->flagField(HcalCaloFlagLabels::TimingAddedBit))
	h_FlagMap_TIMEADD->Fill(rbxindex,rm);
      else if (HFiter->flagField(HcalCaloFlagLabels::TimingErrorBit))
	h_FlagMap_TIMEERROR->Fill(rbxindex,rm);

      if (HFiter->flagField(HcalCaloFlagLabels::HFDigiTime))
	h_FlagMap_DIGITIME->Fill(rbxindex,rm);
      if (HFiter->flagField(HcalCaloFlagLabels::HFLongShort))
	h_FlagMap_LONGSHORT->Fill(rbxindex,rm);

      //Looping over HF searching for flags --- cris
      for (int f=0;f<32;f++)
	{
	  if (HFiter->flagField(f))
	    HFflagcounter_[f]++;
	}


      ++occupancy_[calcEta][iphi-1][depth-1];
      energy_[calcEta][iphi-1][depth-1]+=en;
      energy2_[calcEta][iphi-1][depth-1]+=pow(en,2);
      time_[calcEta][iphi-1][depth-1]+=ti;


      //if (en/cosh(etaBounds[abs(ieta)-29]/area[abs(ieta)-29])>=HFenergyThreshold_)
      if (en>=HFenergyThreshold_ && en/cosh(fEta)>=HFETThreshold_)
	{
	  ++occupancy_thresh_[calcEta][iphi-1][depth-1];
	  energy_thresh_[calcEta][iphi-1][depth-1]+=en;
	  time_thresh_[calcEta][iphi-1][depth-1]+=ti;
	}
      {
	++hfocc;
	if (ti<-100 || ti>200)
	  h_HFTime->Fill(ti);
	else
	  ++HFtime_[int(ti+100)];
	if (en>=HFenergyThreshold_)
	  {
	    ++hfoccthresh;
	    if (ti<-100 || ti>200)
	      h_HFThreshTime->Fill(ti);
	    else
	      ++HFtime_thresh_[int(ti+100)];
	  } // if (en>=HFenergyThreshold_)
      }
    } // loop over all HF hits
     

  if (ePlus>0) tPlus/=ePlus;
  if (eMinus>0) tMinus/=eMinus;
     
  double mintime=min(tPlus,tMinus);
  double minHT=min(EtMinus,EtPlus);
  h_LumiPlot_MinTime_vs_MinHT->Fill(minHT, mintime);

  h_LumiPlot_SumHT_HFPlus_vs_HFMinus->Fill(EtMinus,EtPlus);
  h_LumiPlot_SumEnergy_HFPlus_vs_HFMinus->Fill(eMinus,ePlus);
  h_LumiPlot_timeHFPlus_vs_timeHFMinus->Fill(tMinus,tPlus);


  if (EtMinus>1 && 
      EtPlus>-1 && 
      fabs(en_HFP/hfpocc-en_HFM/hfmocc)<timediffThresh_
      )
    {
      h_LumiPlot_EventsPerLS->Fill(currentLS);
      h_LumiPlot_BX_goodevents->Fill(BCN);
    }

  if (EtMinus>1 && EtPlus>1)
    {
      h_LumiPlot_EventsPerLS_notimecut->Fill(currentLS);
      h_LumiPlot_BX_goodevents_notimecut->Fill(BCN);
    }

  if (Online_ && passedHLT && BPTX==true)
    {
      if (hfpocc >0 && hfmocc>0)
	{
	  h_HFrawenergydifference->Fill(en_HFP/hfpocc-en_HFM/hfmocc);
	  h_HFrawtimedifference->Fill(rawtime_HFP/hfpocc-rawtime_HFM/hfmocc);
	}
      // fill overflow, underflow bins if one side unoccupied?  Try it for time plots only right now
      else if (hfpocc>0)
	h_HFrawtimedifference->Fill(2500);
      else if (hfmocc>0)
	h_HFrawtimedifference->Fill(-2500);

      if (en_HFP !=0 && en_HFM != 0)
	{
	  h_HFtimedifference->Fill((time_HFP/en_HFP)-(time_HFM/en_HFM));
	  h_HFenergydifference->Fill((en_HFP-en_HFM)/(en_HFP+en_HFM));
	}
    } // if (passedHLT)

  else if (Online_ && passedHLT && BPTX==false)
    {
      if (hfpocc >0 && hfmocc>0)
	{
	  h_HFnotBPTXrawenergydifference->Fill(en_HFP/hfpocc-en_HFM/hfmocc);
	  h_HFnotBPTXrawtimedifference->Fill(rawtime_HFP/hfpocc-rawtime_HFM/hfmocc);
	}
      // fill overflow, underflow bins if one side unoccupied?  Try it for time plots only right now
      else if (hfpocc>0)
	h_HFnotBPTXrawtimedifference->Fill(2500);
      else if (hfmocc>0)
	h_HFnotBPTXrawtimedifference->Fill(-2500);
	 
      //cout <<"HF occ + = "<<hfpocc<<"  - = "<<hfmocc<<endl;
      if (en_HFP !=0 && en_HFM != 0)
	{
	  h_HFnotBPTXtimedifference->Fill((time_HFP/en_HFP)-(time_HFM/en_HFM));
	  h_HFnotBPTXenergydifference->Fill((en_HFP-en_HFM)/(en_HFP+en_HFM));
	}
    } // passsed HLT, !101

  ++HF_occupancy_[hfocc/10];
  ++HF_occupancy_thresh_[hfoccthresh/10];
   
 return;
} // void HcalRecHitMonitor::processEvent_rechitenergy

/* --------------------------------------- */


void HcalRecHitMonitor::endLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
                                           const edm::EventSetup& c)

{
  // don't fill lumi block information if it's already been filled
  if (LumiInOrder(lumiSeg.luminosityBlock())==false) return;
  fill_Nevents();
  return;
} //endLuminosityBlock


void HcalRecHitMonitor::fill_Nevents(void)
{
  // looking at the contents of HbFlagcounters
  if (debug_>0)
    {
      for (int k = 0; k < 32; k++){
	std::cout << "<HcalRecHitMonitor::fill_Nevents>  HF Flag counter:  Bin #" << k+1 << " = "<< HFflagcounter_[k] << endl;
      }
    }

  for (int i=0;i<32;i++)
    {
      h_HBflagcounter->Fill(i,HBflagcounter_[i]);
      h_HEflagcounter->Fill(i,HEflagcounter_[i]);
      h_HOflagcounter->Fill(i,HOflagcounter_[i]);
      h_HFflagcounter->Fill(i,HFflagcounter_[i]);
      HBflagcounter_[i]=0;
      HEflagcounter_[i]=0;
      HOflagcounter_[i]=0;
      HFflagcounter_[i]=0;
    }

  // Fill Occupancy & Sum Energy, Time plots
  int myieta=-1;
  if (ievt_>0)
    {
      for (int mydepth=0;mydepth<4;++mydepth)
	{
	  for (int eta=0;eta<OccupancyByDepth.depth[mydepth]->getNbinsX();++eta)
	    {
	      myieta=eta-42;
	      if (myieta<-29)
		++myieta;
	      else if (myieta>29)
		--myieta;
	      
	      for (int phi=0;phi<72;++phi)
		{
		  if (occupancy_[eta][phi][mydepth]>0)
		    {
		      h_rechitieta->Fill(myieta,occupancy_[eta][phi][mydepth]);
		      h_rechitiphi->Fill(phi+1,occupancy_[eta][phi][mydepth]);
		    }
		  if (occupancy_thresh_[eta][phi][mydepth]>0)
		    {
		      h_rechitieta_thresh->Fill(myieta,occupancy_thresh_[eta][phi][mydepth]);
		      h_rechitiphi_thresh->Fill(phi+1,occupancy_thresh_[eta][phi][mydepth]);
		    }
		  OccupancyByDepth.depth[mydepth]->setBinContent(eta+1,phi+1,occupancy_[eta][phi][mydepth]);
		  OccupancyThreshByDepth.depth[mydepth]->setBinContent(eta+1,phi+1,occupancy_thresh_[eta][phi][mydepth]);
		  SumEnergyByDepth.depth[mydepth]->setBinContent(eta+1,phi+1,energy_[eta][phi][mydepth]);
                  SqrtSumEnergy2ByDepth.depth[mydepth]->setBinContent(eta+1,phi+1,sqrt(energy2_[eta][phi][mydepth]));
		  SumEnergyThreshByDepth.depth[mydepth]->setBinContent(eta+1,phi+1,energy_thresh_[eta][phi][mydepth]);
		  SumTimeByDepth.depth[mydepth]->setBinContent(eta+1,phi+1,time_[eta][phi][mydepth]);
		  SumTimeThreshByDepth.depth[mydepth]->setBinContent(eta+1,phi+1,time_thresh_[eta][phi][mydepth]);
		} // for (int phi=0;phi<72;++phi)
	    } // for (int eta=0;eta<OccupancyByDepth...;++eta)
	} // for (int mydepth=0;...)

      FillUnphysicalHEHFBins(OccupancyByDepth);
      FillUnphysicalHEHFBins(OccupancyThreshByDepth);
      FillUnphysicalHEHFBins(SumEnergyByDepth);
      FillUnphysicalHEHFBins(SqrtSumEnergy2ByDepth);
      FillUnphysicalHEHFBins(SumEnergyThreshByDepth);
      FillUnphysicalHEHFBins(SumTimeByDepth);
      FillUnphysicalHEHFBins(SumTimeThreshByDepth);

    } // if (ievt_>0)

  // Fill subdet plots

  for (int i=0;i<(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN);++i)
    {
      if (HBtime_[i]!=0)
	{
	  h_HBTime->setBinContent(i+1,HBtime_[i]);
	}
      if (HBtime_thresh_[i]!=0)
	{
	  h_HBThreshTime->setBinContent(i+1,HBtime_thresh_[i]);
	}
      if (HEtime_[i]!=0)
	{

	  h_HETime->setBinContent(i+1,HEtime_[i]);
	}
      if (HEtime_thresh_[i]!=0)
	{
	  h_HEThreshTime->setBinContent(i+1,HEtime_thresh_[i]);
	}
      if (HOtime_[i]!=0)
	{
	  h_HOTime->setBinContent(i+1,HOtime_[i]);
	}
      if (HOtime_thresh_[i]!=0)
	{
	  h_HOThreshTime->setBinContent(i+1,HOtime_thresh_[i]);
	}
      if (HFtime_[i]!=0)
	{
	  h_HFTime->setBinContent(i+1,HFtime_[i]);
	}
      if (HFtime_thresh_[i]!=0)
	{
	  h_HFThreshTime->setBinContent(i+1,HFtime_thresh_[i]);
	}
    } // for (int  i=0;i<(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN);++i)

  for (int i=0;i<260;++i)
    {
      if (HB_occupancy_[i]>0)
	{
	  h_HBOccupancy->setBinContent(i+1,HB_occupancy_[i]);
	}
      if (HB_occupancy_thresh_[i]>0)
	{
	  h_HBThreshOccupancy->setBinContent(i+1,HB_occupancy_thresh_[i]);
	}
      if (HE_occupancy_[i]>0)
	{
	  h_HEOccupancy->setBinContent(i+1,HE_occupancy_[i]);
	}
      if (HE_occupancy_thresh_[i]>0)
	{
	  h_HEThreshOccupancy->setBinContent(i+1,HE_occupancy_thresh_[i]);
	}
    }//for (int i=0;i<260;++i)

  for (int i=0;i<217;++i)
    {
      if (HO_occupancy_[i]>0)
	{
	  h_HOOccupancy->setBinContent(i+1,HO_occupancy_[i]);
	}
      if (HO_occupancy_thresh_[i]>0)
	{
	  h_HOThreshOccupancy->setBinContent(i+1,HO_occupancy_thresh_[i]);
	}
    }//  for (int i=0;i<217;++i)

  for (int i=0;i<173;++i)
    {
      if (HF_occupancy_[i]>0)
	{
	  h_HFOccupancy->setBinContent(i+1,HF_occupancy_[i]);
	}
      if (HF_occupancy_thresh_[i]>0)
	{
	  h_HFThreshOccupancy->setBinContent(i+1,HF_occupancy_thresh_[i]);
	}
    }//  for (int i=0;i<173;++i)

  //zeroCounters();

  if (debug_>0)
    std::cout <<"<HcalRecHitMonitor::fill_Nevents> FILLED REC HIT CELL PLOTS"<<endl;

} // void HcalRecHitMonitor::fill_Nevents(void)


void HcalRecHitMonitor::zeroCounters(void)
{
  // Set all histogram counters back to zero

  for (int i=0;i<32;++i)
    {
      HBflagcounter_[i]=0;
      HEflagcounter_[i]=0;
      HOflagcounter_[i]=0;
      HFflagcounter_[i]=0;

    }
  // TH2F counters
  for (int i=0;i<85;++i)
    {
      for (int j=0;j<72;++j)
	{
	  for (int k=0;k<4;++k)
	    {
	      occupancy_[i][j][k]=0;
	      occupancy_thresh_[i][j][k]=0;
	      energy_[i][j][k]=0;
              energy2_[i][j][k]=0;
	      energy_thresh_[i][j][k]=0;
	      time_[i][j][k]=0;
	      time_thresh_[i][j][k]=0;
	    }
	} // for (int j=0;j<PHIBINS;++j)
    } // for (int i=0;i<87;++i)

  // TH1F counters
  
  for (int i=0;i<200;++i)
    {
      HFenergyLong_[i]=0;
      HFenergyLong_thresh_[i]=0;
      HFenergyShort_[i]=0;
      HFenergyShort_thresh_[i]=0;
    }

  // time
  for (int i=0;i<(RECHITMON_TIME_MAX-RECHITMON_TIME_MIN);++i)
    {
      HBtime_[i]=0;
      HBtime_thresh_[i]=0;
      HEtime_[i]=0;
      HEtime_thresh_[i]=0;
      HOtime_[i]=0;
      HOtime_thresh_[i]=0;
      HFtime_[i]=0;
      HFtime_thresh_[i]=0;
      HFtimeLong_[i]=0;
      HFtimeLong_thresh_[i]=0;
      HFtimeShort_[i]=0;
      HFtimeShort_thresh_[i]=0;
    }

  // occupancy
  for (int i=0;i<865;++i)
    {
      if (i<=260)
	{
	  HB_occupancy_[i]=0;
	  HE_occupancy_[i]=0;
	  HB_occupancy_thresh_[i]=0;
	  HE_occupancy_thresh_[i]=0;
	}
      if (i<=217)
	{
	  HO_occupancy_[i]=0;
	  HO_occupancy_thresh_[i]=0;
	}
      if (i<=173)
	{
	  HF_occupancy_[i]=0;
	  HF_occupancy_thresh_[i]=0;
	}

      HFlong_occupancy_[i] =0;
      HFshort_occupancy_[i]=0;
      HFlong_occupancy_thresh_[i] =0;
      HFshort_occupancy_thresh_[i]=0;
    } // for (int i=0;i<865;++i)

  return;
} //void HcalRecHitMonitor::zeroCounters(void)

DEFINE_ANOTHER_FWK_MODULE(HcalRecHitMonitor);
