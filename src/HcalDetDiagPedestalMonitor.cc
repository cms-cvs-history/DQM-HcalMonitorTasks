// -*- C++ -*-
//
// Package:    HcalDetDiagPedestalMonitor
// Class:      HcalDetDiagPedestalMonitor
// 
/**\class HcalDetDiagPedestalMonitor HcalDetDiagPedestalMonitor.cc DQM/HcalDetDiagPedestalMonitor/src/HcalDetDiagPedestalMonitor.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Dmitry Vishnevskiy,591 R-013,+41227674265,
//         Created:  Tue Mar  9 12:59:18 CET 2010
// $Id$
//
//
// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQM/HcalMonitorTasks/interface/HcalBaseMonitor.h"

#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"

#include <math.h>
using namespace edm;
using namespace std;
// this is to retrieve HCAL digi's
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
// to retrive trigger information (local runs only)
#include "TBDataFormats/HcalTBObjects/interface/HcalTBTriggerData.h"

#include "CalibCalorimetry/HcalAlgos/interface/HcalLogicalMapGenerator.h"
#include "CondFormats/HcalObjects/interface/HcalLogicalMap.h"

#include "DQM/HcalMonitorTasks/interface/HcalBaseDQMonitor.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DQM/HcalMonitorTasks/interface/HcalEtaPhiHists.h"

#include "TFile.h"
#include "TTree.h"

#include "DataFormats/FEDRawData/interface/FEDRawData.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/HcalDigi/interface/HcalCalibrationEventTypes.h"
#include "EventFilter/HcalRawToDigi/interface/HcalDCCHeader.h"

class HcalDetDiagPedestalData{
public: 
   HcalDetDiagPedestalData(){ 
             reset();
	     IsRefetence=false;
	     status=0; 
             nChecks=nMissing=nBadPed=nBadRms=nUnstable=0;  
	  }
   void   reset(){
             for(int i=0;i<128;i++) adc[i]=0; 
	     overflow=0;
          }	  
   void   add_statistics(unsigned int val){
             if(val<25) adc[val&0x7F]++; else overflow++;    
	  }
   void   set_reference(float val,float rms){
             ref_ped=val; ref_rms=rms;
	     IsRefetence=true;
          }	  
   void   change_status(int val){
             status|=val;
          }	  
   int    get_status(){
             return status;
          }	  
   bool   get_reference(double *val,double *rms){
             *val=ref_ped; *rms=ref_rms;
	     return IsRefetence;
          }	  
   bool   get_average(double *ave,double *rms){
             double Sum=0,nSum=0; 
	     int from,to,max=adc[0],maxi=0;
	     for(int i=1;i<25;i++) if(adc[i]>max){ max=adc[i]; maxi=i;} 
	     from=0; to=maxi+6;
             for(int i=from;i<=to;i++){
                Sum+=i*adc[i];
	        nSum+=adc[i];
             } 
             if(nSum>0) *ave=Sum/nSum; else return false;
             Sum=0;
             for(int i=from;i<=to;i++) Sum+=adc[i]*(i-*ave)*(i-*ave);
             *rms=sqrt(Sum/nSum);
             return true; 
          }
   int    get_statistics(){
             int nSum=0;  
             for(int i=0;i<25;i++) nSum+=adc[i];
	     return nSum;
	  } 
   int    get_overflow(){
             return overflow;
          }   
   int   nChecks;
   int   nMissing;
   int   nUnstable;
   int   nBadPed;
   int   nBadRms;
private:   
   int   adc[128];
   int   overflow;
   bool  IsRefetence;
   float ref_ped;
   float ref_rms;
   int   status;
};


class HcalDetDiagPedestalMonitor : public HcalBaseDQMonitor {
   public:
      explicit HcalDetDiagPedestalMonitor(const edm::ParameterSet&);
      ~HcalDetDiagPedestalMonitor();


   private:
      void SaveReference();
      void LoadReference();
      void CheckStatus();
      void fillHistos();
      void SaveHTML();

      const HcalElectronicsMap  *emap;
      edm::InputTag inputLabelDigi_;

      void beginRun(const edm::Run& run, const edm::EventSetup& c);  
      void endRun(const edm::Run& run, const edm::EventSetup& c);
      void beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg,const edm::EventSetup& c) ;
      void endLuminosityBlock(const edm::LuminosityBlock& lumiSeg,const edm::EventSetup& c);
      void analyze(const edm::Event&, const edm::EventSetup&);

      int         ievt_;
      int         run_number;
      int         dataset_seq_number;
      bool        IsReference;
      bool        LocalRun;

      double      HBMeanTreshold;
      double      HBRmsTreshold;
      double      HEMeanTreshold;
      double      HERmsTreshold;
      double      HOMeanTreshold;
      double      HORmsTreshold;
      double      HFMeanTreshold;
      double      HFRmsTreshold;

      std::string ReferenceData;
      std::string ReferenceRun;
      std::string OutputFilePath;
      bool        enableHTML;
      std::string BaseHTMLpath;

      std::string baseFolder_;
      std::string prefixME_;
      bool        Online_;

      MonitorElement *meEVT_;
      MonitorElement *RefRun_;
      MonitorElement *PedestalsAve4HB;
      MonitorElement *PedestalsAve4HE;
      MonitorElement *PedestalsAve4HO;
      MonitorElement *PedestalsAve4HF;
      MonitorElement *PedestalsAve4Simp;
 
      MonitorElement *PedestalsAve4HBref;
      MonitorElement *PedestalsAve4HEref;
      MonitorElement *PedestalsAve4HOref;
      MonitorElement *PedestalsAve4HFref;
      MonitorElement *PedestalsRmsHB;
      MonitorElement *PedestalsRmsHE;
      MonitorElement *PedestalsRmsHO;
      MonitorElement *PedestalsRmsHF;
      MonitorElement *PedestalsRmsSimp;
  
      MonitorElement *PedestalsRmsHBref;
      MonitorElement *PedestalsRmsHEref;
      MonitorElement *PedestalsRmsHOref;
      MonitorElement *PedestalsRmsHFref;
  
      MonitorElement *Pedestals2DRmsHBHEHF;
      MonitorElement *Pedestals2DRmsHO;
      MonitorElement *Pedestals2DHBHEHF;
      MonitorElement *Pedestals2DHO;
      MonitorElement *Pedestals2DErrorHBHEHF;
      MonitorElement *Pedestals2DErrorHO;
 
      EtaPhiHists* ProblemCellsByDepth_missing;
      EtaPhiHists* ProblemCellsByDepth_unstable;
      EtaPhiHists* ProblemCellsByDepth_badped;
      EtaPhiHists* ProblemCellsByDepth_badrms;
      std::vector<std::string> problemnames_;
 
      HcalDetDiagPedestalData hb_data[85][72][4][4];
      HcalDetDiagPedestalData he_data[85][72][4][4];
      HcalDetDiagPedestalData ho_data[85][72][4][4];
      HcalDetDiagPedestalData hf_data[85][72][4][4];
};

HcalDetDiagPedestalMonitor::HcalDetDiagPedestalMonitor(const edm::ParameterSet& iConfig){
  ievt_=0;
  emap=0;
  dataset_seq_number=1;
  run_number=-1;
  IsReference=false;
  LocalRun=false;

  inputLabelDigi_  = iConfig.getUntrackedParameter<edm::InputTag>("digiLabel");
  enableHTML       = iConfig.getUntrackedParameter<bool>  ("enableHTML",false);
  BaseHTMLpath     = iConfig.getUntrackedParameter<string>("BaseHTMLpath","");
  ReferenceData    = iConfig.getUntrackedParameter<string>("PedestalReferenceData" ,"");
  OutputFilePath   = iConfig.getUntrackedParameter<string>("OutputFilePath", "");
  Online_          = iConfig.getUntrackedParameter<bool>  ("online",false);
  prefixME_        = iConfig.getUntrackedParameter<string>("subSystemFolder","Hcal/");
  subdir_          = iConfig.getUntrackedParameter<string>("TaskFolder","DetDiagPedestalMonitor_Hcal/");
  if (subdir_.size()>0 && subdir_.substr(subdir_.size()-1,subdir_.size())!="/")
    subdir_.append("/");
  subdir_=prefixME_+subdir_;
  HBMeanTreshold   = iConfig.getUntrackedParameter<double>("HBMeanPedestalTreshold" , 0.2);
  HBRmsTreshold    = iConfig.getUntrackedParameter<double>("HBRmsPedestalTreshold"  , 0.3);
  HEMeanTreshold   = iConfig.getUntrackedParameter<double>("HEMeanPedestalTreshold" , 0.2);
  HERmsTreshold    = iConfig.getUntrackedParameter<double>("HERmsPedestalTreshold"  , 0.3);
  HOMeanTreshold   = iConfig.getUntrackedParameter<double>("HOMeanPedestalTreshold" , 0.2);
  HORmsTreshold    = iConfig.getUntrackedParameter<double>("HORmsPedestalTreshold"  , 0.3);
  HFMeanTreshold   = iConfig.getUntrackedParameter<double>("HFMeanPedestalTreshold" , 0.2);
  HFRmsTreshold    = iConfig.getUntrackedParameter<double>("HFRmsPedestalTreshold"  , 0.3);
}
HcalDetDiagPedestalMonitor::~HcalDetDiagPedestalMonitor(){}

void HcalDetDiagPedestalMonitor::beginRun(const edm::Run& run, const edm::EventSetup& c){
  edm::ESHandle<HcalDbService> conditions_;
  c.get<HcalDbRecord>().get(conditions_);
  emap=conditions_->getHcalMapping();

  HcalBaseDQMonitor::setup();
  if (!dbe_) return;
  std::string name;
 
  dbe_->setCurrentFolder(subdir_);   
  meEVT_ = dbe_->bookInt("HcalDetDiagPedestalMonitor Event Number");

  ProblemCellsByDepth_missing = new EtaPhiHists();
  ProblemCellsByDepth_missing->setup(dbe_," Problem Missing Channels");
  for(unsigned int i=0;i<ProblemCellsByDepth_missing->depth.size();i++)
          problemnames_.push_back(ProblemCellsByDepth_missing->depth[i]->getName());
  ProblemCellsByDepth_unstable = new EtaPhiHists();
  ProblemCellsByDepth_unstable->setup(dbe_," Problem Unstable Channels");
  for(unsigned int i=0;i<ProblemCellsByDepth_unstable->depth.size();i++)
          problemnames_.push_back(ProblemCellsByDepth_unstable->depth[i]->getName());
  ProblemCellsByDepth_badped = new EtaPhiHists();
  ProblemCellsByDepth_badped->setup(dbe_," Problem Bad Pedestal Value");
  for(unsigned int i=0;i<ProblemCellsByDepth_badped->depth.size();i++)
          problemnames_.push_back(ProblemCellsByDepth_badped->depth[i]->getName());
  ProblemCellsByDepth_badrms = new EtaPhiHists();
  ProblemCellsByDepth_badrms->setup(dbe_," Problem Bad Rms Value");
  for(unsigned int i=0;i<ProblemCellsByDepth_badrms->depth.size();i++)
          problemnames_.push_back(ProblemCellsByDepth_badrms->depth[i]->getName());

  dbe_->setCurrentFolder(subdir_+"Summary Plots");
  name="HB Pedestal Distribution (average over 4 caps)";           PedestalsAve4HB = dbe_->book1D(name,name,200,0,6);
  name="HE Pedestal Distribution (average over 4 caps)";           PedestalsAve4HE = dbe_->book1D(name,name,200,0,6);
  name="HO Pedestal Distribution (average over 4 caps)";           PedestalsAve4HO = dbe_->book1D(name,name,200,0,6);
  name="HF Pedestal Distribution (average over 4 caps)";           PedestalsAve4HF = dbe_->book1D(name,name,200,0,6);
  name="SIPM Pedestal Distribution (average over 4 caps)";         PedestalsAve4Simp = dbe_->book1D(name,name,200,5,15);
     
  name="HB Pedestal-Reference Distribution (average over 4 caps)"; PedestalsAve4HBref= dbe_->book1D(name,name,1500,-3,3);
  name="HE Pedestal-Reference Distribution (average over 4 caps)"; PedestalsAve4HEref= dbe_->book1D(name,name,1500,-3,3);
  name="HO Pedestal-Reference Distribution (average over 4 caps)"; PedestalsAve4HOref= dbe_->book1D(name,name,1500,-3,3);
  name="HF Pedestal-Reference Distribution (average over 4 caps)"; PedestalsAve4HFref= dbe_->book1D(name,name,1500,-3,3);
    
  name="HB Pedestal RMS Distribution (individual cap)";            PedestalsRmsHB = dbe_->book1D(name,name,200,0,2);
  name="HE Pedestal RMS Distribution (individual cap)";            PedestalsRmsHE = dbe_->book1D(name,name,200,0,2);
  name="HO Pedestal RMS Distribution (individual cap)";            PedestalsRmsHO = dbe_->book1D(name,name,200,0,2);
  name="HF Pedestal RMS Distribution (individual cap)";            PedestalsRmsHF = dbe_->book1D(name,name,200,0,2);
  name="SIPM Pedestal RMS Distribution (individual cap)";          PedestalsRmsSimp = dbe_->book1D(name,name,200,0,4);
     
  name="HB Pedestal_rms-Reference_rms Distribution";               PedestalsRmsHBref = dbe_->book1D(name,name,1500,-3,3);
  name="HE Pedestal_rms-Reference_rms Distribution";               PedestalsRmsHEref = dbe_->book1D(name,name,1500,-3,3);
  name="HO Pedestal_rms-Reference_rms Distribution";               PedestalsRmsHOref = dbe_->book1D(name,name,1500,-3,3);
  name="HF Pedestal_rms-Reference_rms Distribution";               PedestalsRmsHFref = dbe_->book1D(name,name,1500,-3,3);
     
  name="HBHEHF pedestal mean map";       Pedestals2DHBHEHF      = dbe_->book2D(name,name,87,-43,43,74,0,73);
  name="HO pedestal mean map";           Pedestals2DHO          = dbe_->book2D(name,name,33,-16,16,74,0,73);
  name="HBHEHF pedestal rms map";        Pedestals2DRmsHBHEHF   = dbe_->book2D(name,name,87,-43,43,74,0,73);
  name="HO pedestal rms map";            Pedestals2DRmsHO       = dbe_->book2D(name,name,33,-16,16,74,0,73);
  name="HBHEHF pedestal problems map";   Pedestals2DErrorHBHEHF = dbe_->book2D(name,name,87,-43,43,74,0,73);
  name="HO pedestal problems map";       Pedestals2DErrorHO     = dbe_->book2D(name,name,33,-16,16,74,0,73);

  Pedestals2DHBHEHF->setAxisRange(1,5,3);
  Pedestals2DHO->setAxisRange(1,5,3);
  Pedestals2DRmsHBHEHF->setAxisRange(0,2,3);
  Pedestals2DRmsHO->setAxisRange(0,2,3);

  Pedestals2DHBHEHF->setAxisTitle("i#eta",1);
  Pedestals2DHBHEHF->setAxisTitle("i#phi",2);
  Pedestals2DHO->setAxisTitle("i#eta",1);
  Pedestals2DHO->setAxisTitle("i#phi",2);
  Pedestals2DRmsHBHEHF->setAxisTitle("i#eta",1);
  Pedestals2DRmsHBHEHF->setAxisTitle("i#phi",2);
  Pedestals2DRmsHO->setAxisTitle("i#eta",1);
  Pedestals2DRmsHO->setAxisTitle("i#phi",2);
  Pedestals2DErrorHBHEHF->setAxisTitle("i#eta",1);
  Pedestals2DErrorHBHEHF->setAxisTitle("i#phi",2);
  Pedestals2DErrorHO->setAxisTitle("i#eta",1);
  Pedestals2DErrorHO->setAxisTitle("i#phi",2);
  PedestalsAve4HB->setAxisTitle("ADC counts",1);
  PedestalsAve4HE->setAxisTitle("ADC counts",1);
  PedestalsAve4HO->setAxisTitle("ADC counts",1);
  PedestalsAve4HF->setAxisTitle("ADC counts",1);
  PedestalsAve4Simp->setAxisTitle("ADC counts",1);
  PedestalsAve4HBref->setAxisTitle("ADC counts",1);
  PedestalsAve4HEref->setAxisTitle("ADC counts",1);
  PedestalsAve4HOref->setAxisTitle("ADC counts",1);
  PedestalsAve4HFref->setAxisTitle("ADC counts",1);
  PedestalsRmsHB->setAxisTitle("ADC counts",1);
  PedestalsRmsHE->setAxisTitle("ADC counts",1);
  PedestalsRmsHO->setAxisTitle("ADC counts",1);
  PedestalsRmsHF->setAxisTitle("ADC counts",1);
  PedestalsRmsSimp->setAxisTitle("ADC counts",1);
  PedestalsRmsHBref->setAxisTitle("ADC counts",1);
  PedestalsRmsHEref->setAxisTitle("ADC counts",1);
  PedestalsRmsHOref->setAxisTitle("ADC counts",1);
  PedestalsRmsHFref->setAxisTitle("ADC counts",1);

  ReferenceRun="UNKNOWN";
  LoadReference();
  dbe_->setCurrentFolder(subdir_);
  RefRun_= dbe_->bookString("HcalDetDiagLaserMonitor Reference Run",ReferenceRun);
}



void HcalDetDiagPedestalMonitor::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup){
  HcalBaseDQMonitor::analyze(iEvent, iSetup); // increments counters
int  eta,phi,depth,nTS;
static bool PEDseq;
static int  lastPEDorbit,nChecksPED;
   if(ievt_==0){ PEDseq=false; lastPEDorbit=-1;nChecksPED=0; }
   int orbit=iEvent.orbitNumber();

   bool PedestalEvent=false;

   // for local runs 
   edm::Handle<HcalTBTriggerData> trigger_data;
   iEvent.getByType(trigger_data);
   if(trigger_data.isValid()){
       if(trigger_data->triggerWord()==5) PedestalEvent=true;
       LocalRun=true;
   }
   if(LocalRun && !PedestalEvent) return; 


  if(!LocalRun && Online_){
      if(PEDseq && (orbit-lastPEDorbit)>(11223*10)){
         PEDseq=false;
         fillHistos();
         CheckStatus();
         nChecksPED++;
         if(nChecksPED==1 || (nChecksPED>1 && ((nChecksPED-1)%8)==0)){
             SaveReference();
             SaveHTML();
         }
         for(int i=0;i<85;i++)for(int j=0;j<72;j++)for(int k=0;k<4;k++)for(int l=0;l<4;l++) hb_data[i][j][k][l].reset();
         for(int i=0;i<85;i++)for(int j=0;j<72;j++)for(int k=0;k<4;k++)for(int l=0;l<4;l++) he_data[i][j][k][l].reset();
         for(int i=0;i<85;i++)for(int j=0;j<72;j++)for(int k=0;k<4;k++)for(int l=0;l<4;l++) ho_data[i][j][k][l].reset();
         for(int i=0;i<85;i++)for(int j=0;j<72;j++)for(int k=0;k<4;k++)for(int l=0;l<4;l++) hf_data[i][j][k][l].reset();
      }
   }

  
   // Abort Gap pedestals 
   int calibType = -1 ;
   if(LocalRun==false){
       edm::Handle<FEDRawDataCollection> rawdata;
       iEvent.getByType(rawdata);
       //checking FEDs for calibration information
       for (int i=FEDNumbering::MINHCALFEDID;i<=FEDNumbering::MAXHCALFEDID; i++){
         const FEDRawData& fedData = rawdata->FEDData(i) ;
	 if ( fedData.size() < 24 ) continue ;
	 int value = ((const HcalDCCHeader*)(fedData.data()))->getCalibType() ;
	 if ( calibType < 0 )  calibType = value ;
         if(value==hc_Pedestal){   PEDseq=true;  lastPEDorbit=orbit; break;} 
       }
   }
   if(!LocalRun && calibType!=hc_Pedestal) return; 

   ievt_++;
   meEVT_->Fill(ievt_);
   run_number=iEvent.id().run();

   edm::Handle<HBHEDigiCollection> hbhe; 
   iEvent.getByLabel(inputLabelDigi_,hbhe);
   if(hbhe.isValid()){
	 if(hbhe->size()<30 && calibType==hc_Pedestal){
             ievt_--;
             meEVT_->Fill(ievt_);
             return;	 
	 }
         for(HBHEDigiCollection::const_iterator digi=hbhe->begin();digi!=hbhe->end();digi++){
             eta=digi->id().ieta(); phi=digi->id().iphi(); depth=digi->id().depth(); nTS=digi->size();
             if(nTS>8) nTS=8;
	     if(nTS<8) continue;
	     if(digi->id().subdet()==HcalBarrel){
		for(int i=0;i<nTS;i++) hb_data[eta+42][phi-1][depth-1][digi->sample(i).capid()].add_statistics(digi->sample(i).adc());
	     }	 
             if(digi->id().subdet()==HcalEndcap){
		for(int i=0;i<nTS;i++) he_data[eta+42][phi-1][depth-1][digi->sample(i).capid()].add_statistics(digi->sample(i).adc());
	     }
         }   
   }
   edm::Handle<HODigiCollection> ho; 
   iEvent.getByLabel(inputLabelDigi_,ho);
   if(ho.isValid()){
         for(HODigiCollection::const_iterator digi=ho->begin();digi!=ho->end();digi++){
             eta=digi->id().ieta(); phi=digi->id().iphi(); depth=digi->id().depth(); nTS=digi->size();
	     if(nTS>8) nTS=8;
	     if(nTS<8) continue;
             for(int i=0;i<nTS;i++) ho_data[eta+42][phi-1][depth-1][digi->sample(i).capid()].add_statistics(digi->sample(i).adc());
         }   
   }
   edm::Handle<HFDigiCollection> hf;
   iEvent.getByLabel(inputLabelDigi_,hf);
   if(hf.isValid()){
         for(HFDigiCollection::const_iterator digi=hf->begin();digi!=hf->end();digi++){
             eta=digi->id().ieta(); phi=digi->id().iphi(); depth=digi->id().depth(); nTS=digi->size();
	     if(nTS>8) nTS=8;
	     if(nTS<8) continue;
	     for(int i=0;i<nTS;i++) hf_data[eta+42][phi-1][depth-1][digi->sample(i).capid()].add_statistics(digi->sample(i).adc());
         }   
   }
}

void HcalDetDiagPedestalMonitor::fillHistos(){
   PedestalsRmsHB->Reset();
   PedestalsAve4HB->Reset();
   PedestalsRmsHE->Reset();
   PedestalsAve4HE->Reset();
   PedestalsRmsHO->Reset();
   PedestalsAve4HO->Reset();
   PedestalsRmsHF->Reset();
   PedestalsAve4HF->Reset();
   PedestalsRmsSimp->Reset();
   PedestalsAve4Simp->Reset();
   Pedestals2DRmsHBHEHF->Reset();
   Pedestals2DRmsHO->Reset();
   Pedestals2DHBHEHF->Reset();
   Pedestals2DHO->Reset();
   // HBHEHF summary map
   for(int eta=-42;eta<=42;eta++) for(int phi=1;phi<=72;phi++){ 
      double PED=0,RMS=0,nped=0,nrms=0,ave=0,rms=0;
      for(int depth=1;depth<=3;depth++){
         if(hb_data[eta+42][phi-1][depth-1][0].get_statistics()>100){
	    hb_data[eta+42][phi-1][depth-1][0].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++;
	    hb_data[eta+42][phi-1][depth-1][1].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
	    hb_data[eta+42][phi-1][depth-1][2].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
	    hb_data[eta+42][phi-1][depth-1][3].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
         }
         if(he_data[eta+42][phi-1][depth-1][0].get_statistics()>100){
	    he_data[eta+42][phi-1][depth-1][0].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++;
	    he_data[eta+42][phi-1][depth-1][1].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
	    he_data[eta+42][phi-1][depth-1][2].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
	    he_data[eta+42][phi-1][depth-1][3].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
         }
         if(hf_data[eta+42][phi-1][depth-1][0].get_statistics()>100){
	    hf_data[eta+42][phi-1][depth-1][0].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++;
	    hf_data[eta+42][phi-1][depth-1][1].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
	    hf_data[eta+42][phi-1][depth-1][2].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
	    hf_data[eta+42][phi-1][depth-1][3].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
         }
      }
      if(nped>0) Pedestals2DHBHEHF->Fill(eta,phi,PED/nped);
      if(nrms>0) Pedestals2DRmsHBHEHF->Fill(eta,phi,RMS/nrms); 
      if(nped>0 && abs(eta)>20) Pedestals2DHBHEHF->Fill(eta,phi+1,PED/nped);
      if(nrms>0 && abs(eta)>20) Pedestals2DRmsHBHEHF->Fill(eta,phi+1,RMS/nrms); 
   }
   // HO summary map
   for(int eta=-42;eta<=42;eta++) for(int phi=1;phi<=72;phi++){ 
      double PED=0,RMS=0,nped=0,nrms=0,ave,rms;
      if(ho_data[eta+42][phi-1][4-1][0].get_statistics()>100){
	 ho_data[eta+42][phi-1][4-1][0].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++;
	 ho_data[eta+42][phi-1][4-1][1].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
	 ho_data[eta+42][phi-1][4-1][2].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
	 ho_data[eta+42][phi-1][4-1][3].get_average(&ave,&rms); PED+=ave; nped++; RMS+=rms; nrms++; 
      }
      if(nped>0) Pedestals2DHO->Fill(eta,phi,PED/nped);
      if(nrms>0) Pedestals2DRmsHO->Fill(eta,phi,RMS/nrms); 
   }
   // HB histograms
   for(int eta=-16;eta<=16;eta++) for(int phi=1;phi<=72;phi++) for(int depth=1;depth<=2;depth++){
      if(hb_data[eta+42][phi-1][depth-1][0].get_statistics()>100){
          double ave,rms,sum=0;
	  hb_data[eta+42][phi-1][depth-1][0].get_average(&ave,&rms); sum+=ave; PedestalsRmsHB->Fill(rms);
	  hb_data[eta+42][phi-1][depth-1][1].get_average(&ave,&rms); sum+=ave; PedestalsRmsHB->Fill(rms);
	  hb_data[eta+42][phi-1][depth-1][2].get_average(&ave,&rms); sum+=ave; PedestalsRmsHB->Fill(rms);
	  hb_data[eta+42][phi-1][depth-1][3].get_average(&ave,&rms); sum+=ave; PedestalsRmsHB->Fill(rms);
	  PedestalsAve4HB->Fill(sum/4.0);
      }
   } 
   // HE histograms
   for(int eta=-29;eta<=29;eta++) for(int phi=1;phi<=72;phi++) for(int depth=1;depth<=3;depth++){
      if(he_data[eta+42][phi-1][depth-1][0].get_statistics()>100){
          double ave=0,rms=0,sum=0;
	  he_data[eta+42][phi-1][depth-1][0].get_average(&ave,&rms); sum+=ave; PedestalsRmsHE->Fill(rms);
	  he_data[eta+42][phi-1][depth-1][1].get_average(&ave,&rms); sum+=ave; PedestalsRmsHE->Fill(rms);
	  he_data[eta+42][phi-1][depth-1][2].get_average(&ave,&rms); sum+=ave; PedestalsRmsHE->Fill(rms);
	  he_data[eta+42][phi-1][depth-1][3].get_average(&ave,&rms); sum+=ave; PedestalsRmsHE->Fill(rms);
	  PedestalsAve4HE->Fill(sum/4.0);
      }
   } 
   // HO histograms
   for(int eta=-15;eta<=15;eta++) for(int phi=1;phi<=72;phi++) for(int depth=4;depth<=4;depth++){
      if(ho_data[eta+42][phi-1][depth-1][0].get_statistics()>100){
          double ave,rms,sum=0;
	  if((eta>=11 && eta<=15 && phi>=59 && phi<=70) || (eta>=5 && eta<=10 && phi>=47 && phi<=58)){
	     ho_data[eta+42][phi-1][depth-1][0].get_average(&ave,&rms); sum+=ave; PedestalsRmsSimp->Fill(rms);
	     ho_data[eta+42][phi-1][depth-1][1].get_average(&ave,&rms); sum+=ave; PedestalsRmsSimp->Fill(rms);
	     ho_data[eta+42][phi-1][depth-1][2].get_average(&ave,&rms); sum+=ave; PedestalsRmsSimp->Fill(rms);
	     ho_data[eta+42][phi-1][depth-1][3].get_average(&ave,&rms); sum+=ave; PedestalsRmsSimp->Fill(rms);
	     PedestalsAve4Simp->Fill(sum/4.0);	  
	  }else{
	     ho_data[eta+42][phi-1][depth-1][0].get_average(&ave,&rms); sum+=ave; PedestalsRmsHO->Fill(rms);
	     ho_data[eta+42][phi-1][depth-1][1].get_average(&ave,&rms); sum+=ave; PedestalsRmsHO->Fill(rms);
	     ho_data[eta+42][phi-1][depth-1][2].get_average(&ave,&rms); sum+=ave; PedestalsRmsHO->Fill(rms);
	     ho_data[eta+42][phi-1][depth-1][3].get_average(&ave,&rms); sum+=ave; PedestalsRmsHO->Fill(rms);
	     PedestalsAve4HO->Fill(sum/4.0);
	  }
      }
   } 
   // HF histograms
   for(int eta=-42;eta<=42;eta++) for(int phi=1;phi<=72;phi++) for(int depth=1;depth<=2;depth++){
      if(hf_data[eta+42][phi-1][depth-1][0].get_statistics()>100){
          double ave,rms,sum=0;
	  hf_data[eta+42][phi-1][depth-1][0].get_average(&ave,&rms); sum+=ave; PedestalsRmsHF->Fill(rms);
	  hf_data[eta+42][phi-1][depth-1][1].get_average(&ave,&rms); sum+=ave; PedestalsRmsHF->Fill(rms);
	  hf_data[eta+42][phi-1][depth-1][2].get_average(&ave,&rms); sum+=ave; PedestalsRmsHF->Fill(rms);
	  hf_data[eta+42][phi-1][depth-1][3].get_average(&ave,&rms); sum+=ave; PedestalsRmsHF->Fill(rms);
	  PedestalsAve4HF->Fill(sum/4.0);
      }
   } 
} 
void HcalDetDiagPedestalMonitor::CheckStatus(){
   for(int i=0;i<4;i++){
      ProblemCellsByDepth_missing->depth[i]->Reset();
      ProblemCellsByDepth_unstable->depth[i]->Reset();
      ProblemCellsByDepth_badped->depth[i]->Reset();
      ProblemCellsByDepth_badrms->depth[i]->Reset();
   }
   PedestalsRmsHBref->Reset();
   PedestalsAve4HBref->Reset();
   PedestalsRmsHEref->Reset();
   PedestalsAve4HEref->Reset();
   PedestalsRmsHOref->Reset();
   PedestalsAve4HOref->Reset();
   PedestalsRmsHFref->Reset();
   PedestalsAve4HFref->Reset();
     
   Pedestals2DErrorHBHEHF->Reset();
   Pedestals2DErrorHO->Reset();

   if(emap==0) return;
   
   std::vector <HcalElectronicsId> AllElIds = emap->allElectronicsIdPrecision();
   for (std::vector <HcalElectronicsId>::iterator eid = AllElIds.begin(); eid != AllElIds.end(); eid++) {
     DetId detid=emap->lookup(*eid);
     int eta=0,phi=0,depth=0;
     try{
       HcalDetId hid(detid);
       eta=hid.ieta();
       phi=hid.iphi();
       depth=hid.depth(); 
     }catch(...){ continue; } 
     int sd=detid.subdetId();
     if(sd==HcalBarrel){
          int ovf=hb_data[eta+42][phi-1][depth-1][0].get_overflow();
	  int stat=hb_data[eta+42][phi-1][depth-1][0].get_statistics()+ovf;
	  double status=0;
	  double ped[4],rms[4],ped_ref[4],rms_ref[4]; 
          hb_data[eta+42][phi-1][depth-1][0].nChecks++;
	  if(stat==0){ 
              status=1;
              int e=CalcEtaBin(sd,eta,depth)+1; 
              hb_data[eta+42][phi-1][depth-1][0].nMissing++;
              double val=hb_data[eta+42][phi-1][depth-1][0].nMissing/hb_data[eta+42][phi-1][depth-1][0].nChecks;
              ProblemCellsByDepth_missing->depth[depth-1]->setBinContent(e,phi,val);
          }
          if(status) hb_data[eta+42][phi-1][depth-1][0].change_status(1); 
	  if(stat>0 && stat!=(ievt_*2)){ status=(double)stat/(double)(ievt_*2); 
	      if(status<0.995){ 
                int e=CalcEtaBin(sd,eta,depth)+1; 
                hb_data[eta+42][phi-1][depth-1][0].nUnstable++;
                double val=hb_data[eta+42][phi-1][depth-1][0].nUnstable/hb_data[eta+42][phi-1][depth-1][0].nChecks;
                ProblemCellsByDepth_unstable->depth[depth-1]->setBinContent(e,phi,val);
	        hb_data[eta+42][phi-1][depth-1][0].change_status(2);
	      }
	  }
	  if(hb_data[eta+42][phi-1][depth-1][0].get_reference(&ped_ref[0],&rms_ref[0]) 
	                                                 && hb_data[eta+42][phi-1][depth-1][0].get_average(&ped[0],&rms[0])){
	     hb_data[eta+42][phi-1][depth-1][1].get_reference(&ped_ref[1],&rms_ref[1]);
	     hb_data[eta+42][phi-1][depth-1][2].get_reference(&ped_ref[2],&rms_ref[2]);
	     hb_data[eta+42][phi-1][depth-1][3].get_reference(&ped_ref[3],&rms_ref[3]);
	     hb_data[eta+42][phi-1][depth-1][1].get_average(&ped[1],&rms[1]);
	     hb_data[eta+42][phi-1][depth-1][2].get_average(&ped[2],&rms[2]);
	     hb_data[eta+42][phi-1][depth-1][3].get_average(&ped[3],&rms[3]);
	     double ave=(ped[0]+ped[1]+ped[2]+ped[3])/4.0; 
	     double ave_ref=(ped_ref[0]+ped_ref[1]+ped_ref[2]+ped_ref[3])/4.0; 
	     double deltaPed=ave-ave_ref; PedestalsAve4HBref->Fill(deltaPed); if(deltaPed<0) deltaPed=-deltaPed;
	     double deltaRms=rms[0]-rms_ref[0]; PedestalsRmsHBref->Fill(deltaRms); if(deltaRms<0) deltaRms=-deltaRms;
	     for(int i=1;i<4;i++){
	        double tmp=rms[0]-rms_ref[0]; PedestalsRmsHBref->Fill(tmp); if(tmp<0) tmp=-tmp;
		if(tmp>deltaRms) deltaRms=tmp;
	     }
	     if(deltaPed>HBMeanTreshold){ 
                 int e=CalcEtaBin(sd,eta,depth)+1; 
                 hb_data[eta+42][phi-1][depth-1][0].nBadPed++;
                 double val=hb_data[eta+42][phi-1][depth-1][0].nBadPed/hb_data[eta+42][phi-1][depth-1][0].nChecks;
                 ProblemCellsByDepth_badped->depth[depth-1]->setBinContent(e,phi,val);
                 Pedestals2DErrorHBHEHF->Fill(eta,phi,1);
             }
	     if(deltaRms>HBRmsTreshold){  
                 int e=CalcEtaBin(sd,eta,depth)+1; 
                 hb_data[eta+42][phi-1][depth-1][0].nBadRms++;
                 double val=hb_data[eta+42][phi-1][depth-1][0].nBadRms/hb_data[eta+42][phi-1][depth-1][0].nChecks;
                 ProblemCellsByDepth_badrms->depth[depth-1]->setBinContent(e,phi,val);
                 Pedestals2DErrorHBHEHF->Fill(eta,phi,1);
             }
	  } 
      }
      if(sd==HcalEndcap){
          int ovf=he_data[eta+42][phi-1][depth-1][0].get_overflow();
	  int stat=he_data[eta+42][phi-1][depth-1][0].get_statistics()+ovf;
	  double status=0; 
	  double ped[4],rms[4],ped_ref[4],rms_ref[4]; 
          he_data[eta+42][phi-1][depth-1][0].nChecks++;
	  if(stat==0){ 
              status=1;
              int e=CalcEtaBin(sd,eta,depth)+1; 
              he_data[eta+42][phi-1][depth-1][0].nMissing++;
              double val=he_data[eta+42][phi-1][depth-1][0].nMissing/he_data[eta+42][phi-1][depth-1][0].nChecks;
              ProblemCellsByDepth_missing->depth[depth-1]->setBinContent(e,phi,val);
          }
	  if(status) he_data[eta+42][phi-1][depth-1][0].change_status(1); 
	  if(stat>0 && stat!=(ievt_*2)){ status=(double)stat/(double)(ievt_*2);
	     if(status<0.995){ 
                int e=CalcEtaBin(sd,eta,depth)+1; 
                he_data[eta+42][phi-1][depth-1][0].nUnstable++;
                double val=he_data[eta+42][phi-1][depth-1][0].nUnstable/he_data[eta+42][phi-1][depth-1][0].nChecks;
                ProblemCellsByDepth_unstable->depth[depth-1]->setBinContent(e,phi,val);
	        he_data[eta+42][phi-1][depth-1][0].change_status(2); 
	     }
	  }
	  if(he_data[eta+42][phi-1][depth-1][0].get_reference(&ped_ref[0],&rms_ref[0]) 
	                                                 && he_data[eta+42][phi-1][depth-1][0].get_average(&ped[0],&rms[0])){
	     he_data[eta+42][phi-1][depth-1][1].get_reference(&ped_ref[1],&rms_ref[1]);
	     he_data[eta+42][phi-1][depth-1][2].get_reference(&ped_ref[2],&rms_ref[2]);
	     he_data[eta+42][phi-1][depth-1][3].get_reference(&ped_ref[3],&rms_ref[3]);
	     he_data[eta+42][phi-1][depth-1][1].get_average(&ped[1],&rms[1]);
	     he_data[eta+42][phi-1][depth-1][2].get_average(&ped[2],&rms[2]);
	     he_data[eta+42][phi-1][depth-1][3].get_average(&ped[3],&rms[3]);
	     double ave=(ped[0]+ped[1]+ped[2]+ped[3])/4.0; 
	     double ave_ref=(ped_ref[0]+ped_ref[1]+ped_ref[2]+ped_ref[3])/4.0; 
	     double deltaPed=ave-ave_ref; PedestalsAve4HEref->Fill(deltaPed); if(deltaPed<0) deltaPed=-deltaPed;
	     double deltaRms=rms[0]-rms_ref[0]; PedestalsRmsHEref->Fill(deltaRms); if(deltaRms<0) deltaRms=-deltaRms;
	     for(int i=1;i<4;i++){
	        double tmp=rms[0]-rms_ref[0]; PedestalsRmsHEref->Fill(tmp); if(tmp<0) tmp=-tmp;
		if(tmp>deltaRms) deltaRms=tmp;
	     }
	     if(deltaPed>HEMeanTreshold){
                 int e=CalcEtaBin(sd,eta,depth)+1; 
                 he_data[eta+42][phi-1][depth-1][0].nBadPed++;
                 double val=he_data[eta+42][phi-1][depth-1][0].nBadPed/he_data[eta+42][phi-1][depth-1][0].nChecks;
                 ProblemCellsByDepth_badped->depth[depth-1]->setBinContent(e,phi,val);
                 Pedestals2DErrorHBHEHF->Fill(eta,phi,1);
             }
	     if(deltaRms>HERmsTreshold){ 
                 int e=CalcEtaBin(sd,eta,depth)+1; 
                 he_data[eta+42][phi-1][depth-1][0].nBadRms++;
                 double val=he_data[eta+42][phi-1][depth-1][0].nBadRms/he_data[eta+42][phi-1][depth-1][0].nChecks;
                 ProblemCellsByDepth_badrms->depth[depth-1]->setBinContent(e,phi,val);
                 Pedestals2DErrorHBHEHF->Fill(eta,phi,1);
             }
	  } 
      }
      if(sd==HcalOuter){
          int ovf=ho_data[eta+42][phi-1][depth-1][0].get_overflow(); 
	  int stat=ho_data[eta+42][phi-1][depth-1][0].get_statistics()+ovf;
	  double status=0; 
	  double ped[4],rms[4],ped_ref[4],rms_ref[4]; 
          ho_data[eta+42][phi-1][depth-1][0].nChecks++;
	  if(stat==0){ 
              status=1; 
              int e=CalcEtaBin(sd,eta,depth)+1; 
              ho_data[eta+42][phi-1][depth-1][0].nMissing++;
              double val=ho_data[eta+42][phi-1][depth-1][0].nMissing/ho_data[eta+42][phi-1][depth-1][0].nChecks;
              ProblemCellsByDepth_missing->depth[depth-1]->setBinContent(e,phi,val);
          }
	  if(status) ho_data[eta+42][phi-1][depth-1][0].change_status(1); 
	  if(stat>0 && stat!=(ievt_*2)){ status=(double)stat/(double)(ievt_*2); 
	     if(status<0.995){ 
                int e=CalcEtaBin(sd,eta,depth)+1; 
                ho_data[eta+42][phi-1][depth-1][0].nUnstable++;
                double val=ho_data[eta+42][phi-1][depth-1][0].nUnstable/ho_data[eta+42][phi-1][depth-1][0].nChecks;
                ProblemCellsByDepth_unstable->depth[depth-1]->setBinContent(e,phi,val);
	        ho_data[eta+42][phi-1][depth-1][0].change_status(2); 
	     }
	  }
	  if(ho_data[eta+42][phi-1][depth-1][0].get_reference(&ped_ref[0],&rms_ref[0]) 
	                                                 && ho_data[eta+42][phi-1][depth-1][0].get_average(&ped[0],&rms[0])){
	     ho_data[eta+42][phi-1][depth-1][1].get_reference(&ped_ref[1],&rms_ref[1]);
	     ho_data[eta+42][phi-1][depth-1][2].get_reference(&ped_ref[2],&rms_ref[2]);
	     ho_data[eta+42][phi-1][depth-1][3].get_reference(&ped_ref[3],&rms_ref[3]);
	     ho_data[eta+42][phi-1][depth-1][1].get_average(&ped[1],&rms[1]);
	     ho_data[eta+42][phi-1][depth-1][2].get_average(&ped[2],&rms[2]);
	     ho_data[eta+42][phi-1][depth-1][3].get_average(&ped[3],&rms[3]);
	     
	     double THRESTHOLD=HORmsTreshold;
	     if((eta>=11 && eta<=15 && phi>=59 && phi<=70) || (eta>=5 && eta<=10 && phi>=47 && phi<=58))THRESTHOLD*=2; 
	     double ave=(ped[0]+ped[1]+ped[2]+ped[3])/4.0; 
	     double ave_ref=(ped_ref[0]+ped_ref[1]+ped_ref[2]+ped_ref[3])/4.0; 
	     double deltaPed=ave-ave_ref; PedestalsAve4HOref->Fill(deltaPed);if(deltaPed<0) deltaPed=-deltaPed;
	     double deltaRms=rms[0]-rms_ref[0]; PedestalsRmsHOref->Fill(deltaRms); if(deltaRms<0) deltaRms=-deltaRms;
	     for(int i=1;i<4;i++){
	        double tmp=rms[0]-rms_ref[0]; PedestalsRmsHOref->Fill(tmp); if(tmp<0) tmp=-tmp;
		if(tmp>deltaRms) deltaRms=tmp;
	     }
	     if(deltaPed>HOMeanTreshold){ 
                 int e=CalcEtaBin(sd,eta,depth)+1; 
                 ho_data[eta+42][phi-1][depth-1][0].nBadPed++;
                 double val=ho_data[eta+42][phi-1][depth-1][0].nBadPed/ho_data[eta+42][phi-1][depth-1][0].nChecks;
                 ProblemCellsByDepth_badped->depth[depth-1]->setBinContent(e,phi,val);
                 Pedestals2DErrorHO->Fill(eta,phi,1);
             }
	     if(deltaRms>THRESTHOLD){ 
                 int e=CalcEtaBin(sd,eta,depth)+1; 
                 ho_data[eta+42][phi-1][depth-1][0].nBadRms++;
                 double val=ho_data[eta+42][phi-1][depth-1][0].nBadRms/ho_data[eta+42][phi-1][depth-1][0].nChecks;
                 ProblemCellsByDepth_badrms->depth[depth-1]->setBinContent(e,phi,val);
                 Pedestals2DErrorHO->Fill(eta,phi,1);
             }
	  } 
      }
      if(sd==HcalForward){
          int ovf=hf_data[eta+42][phi-1][depth-1][0].get_overflow();
	  int stat=hf_data[eta+42][phi-1][depth-1][0].get_statistics()+ovf;
	  double status=0; 
	  double ped[4],rms[4],ped_ref[4],rms_ref[4]; 
          hf_data[eta+42][phi-1][depth-1][0].nChecks++;
	  if(stat==0){ 
             status=1;                    
             int e=CalcEtaBin(sd,eta,depth)+1; 
             hf_data[eta+42][phi-1][depth-1][0].nMissing++;
             double val=hf_data[eta+42][phi-1][depth-1][0].nMissing/hf_data[eta+42][phi-1][depth-1][0].nChecks;
             ProblemCellsByDepth_missing->depth[depth-1]->setBinContent(e,phi,val);
          }
	  if(status) hf_data[eta+42][phi-1][depth-1][0].change_status(1); 
	  if(stat>0 && stat!=(ievt_*2)){ status=(double)stat/(double)(ievt_*2); 
	     if(status<0.995){ 
                int e=CalcEtaBin(sd,eta,depth)+1; 
                hf_data[eta+42][phi-1][depth-1][0].nUnstable++;
                double val=hf_data[eta+42][phi-1][depth-1][0].nUnstable/hf_data[eta+42][phi-1][depth-1][0].nChecks;
                ProblemCellsByDepth_unstable->depth[depth-1]->setBinContent(e,phi,val);
	        hf_data[eta+42][phi-1][depth-1][0].change_status(2); 
	     }
	  }
	  if(hf_data[eta+42][phi-1][depth-1][0].get_reference(&ped_ref[0],&rms_ref[0]) 
	                                                 && hf_data[eta+42][phi-1][depth-1][0].get_average(&ped[0],&rms[0])){
	     hf_data[eta+42][phi-1][depth-1][1].get_reference(&ped_ref[1],&rms_ref[1]);
	     hf_data[eta+42][phi-1][depth-1][2].get_reference(&ped_ref[2],&rms_ref[2]);
	     hf_data[eta+42][phi-1][depth-1][3].get_reference(&ped_ref[3],&rms_ref[3]);
	     hf_data[eta+42][phi-1][depth-1][1].get_average(&ped[1],&rms[1]);
	     hf_data[eta+42][phi-1][depth-1][2].get_average(&ped[2],&rms[2]);
	     hf_data[eta+42][phi-1][depth-1][3].get_average(&ped[3],&rms[3]);
	     double ave=(ped[0]+ped[1]+ped[2]+ped[3])/4.0; 
	     double ave_ref=(ped_ref[0]+ped_ref[1]+ped_ref[2]+ped_ref[3])/4.0; 
	     double deltaPed=ave-ave_ref; PedestalsAve4HFref->Fill(deltaPed); if(deltaPed<0) deltaPed=-deltaPed;
	     double deltaRms=rms[0]-rms_ref[0]; PedestalsRmsHFref->Fill(deltaRms); if(deltaRms<0) deltaRms=-deltaRms;
	     for(int i=1;i<4;i++){
	        double tmp=rms[0]-rms_ref[0]; PedestalsRmsHFref->Fill(tmp); if(tmp<0) tmp=-tmp;
		if(tmp>deltaRms) deltaRms=tmp;
	     }
	     if(deltaPed>HFMeanTreshold){ 
                 int e=CalcEtaBin(sd,eta,depth)+1; 
                 hf_data[eta+42][phi-1][depth-1][0].nBadPed++;
                 double val=hf_data[eta+42][phi-1][depth-1][0].nBadPed/hf_data[eta+42][phi-1][depth-1][0].nChecks;
                 ProblemCellsByDepth_badped->depth[depth-1]->setBinContent(e,phi,val);
                 Pedestals2DErrorHBHEHF->Fill(eta,phi,1);
             }
	     if(deltaRms>HFRmsTreshold){ 
                 int e=CalcEtaBin(sd,eta,depth)+1; 
                 hf_data[eta+42][phi-1][depth-1][0].nBadRms++;
                 double val=hf_data[eta+42][phi-1][depth-1][0].nBadRms/hf_data[eta+42][phi-1][depth-1][0].nChecks;
                 ProblemCellsByDepth_badrms->depth[depth-1]->setBinContent(e,phi,val);
                 Pedestals2DErrorHBHEHF->Fill(eta,phi,1);
             }
	  } 
      }
   }
}

void HcalDetDiagPedestalMonitor::endRun(const edm::Run& run, const edm::EventSetup& c){
    if((LocalRun || !Online_) && ievt_>500){
       fillHistos();
       CheckStatus();
       SaveReference();
       SaveHTML();
    }
}


void HcalDetDiagPedestalMonitor::SaveReference(){
double ped[4],rms[4];
int    Eta,Phi,Depth,Statistic,Status=0;
char   Subdet[10],str[500];
       sprintf(str,"%sHcalDetDiagPedestalData_run%06i_%i.root",OutputFilePath.c_str(),run_number,dataset_seq_number);
       TFile *theFile = new TFile(str, "RECREATE");
       if(!theFile->IsOpen()) return;
       theFile->cd();
       sprintf(str,"%d",run_number); TObjString run(str);    run.Write("run number");
       sprintf(str,"%d",ievt_);      TObjString events(str); events.Write("Total events processed");
       
       TTree *tree   =new TTree("HCAL Pedestal data","HCAL Pedestal data");
       if(tree==0)   return;
       tree->Branch("Subdet",   &Subdet,         "Subdet/C");
       tree->Branch("eta",      &Eta,            "Eta/I");
       tree->Branch("phi",      &Phi,            "Phi/I");
       tree->Branch("depth",    &Depth,          "Depth/I");
       tree->Branch("statistic",&Statistic,      "Statistic/I");
       tree->Branch("status",   &Status,         "Status/I");
       tree->Branch("cap0_ped", &ped[0],         "cap0_ped/D");
       tree->Branch("cap0_rms", &rms[0],         "cap0_rms/D");
       tree->Branch("cap1_ped", &ped[1],         "cap1_ped/D");
       tree->Branch("cap1_rms", &rms[1],         "cap1_rms/D");
       tree->Branch("cap2_ped", &ped[2],         "cap2_ped/D");
       tree->Branch("cap2_rms", &rms[2],         "cap2_rms/D");
       tree->Branch("cap3_ped", &ped[3],         "cap3_ped/D");
       tree->Branch("cap3_rms", &rms[3],         "cap3_rms/D");
       sprintf(Subdet,"HB");
       for(int eta=-16;eta<=16;eta++) for(int phi=1;phi<=72;phi++) for(int depth=1;depth<=2;depth++){
          if((Statistic=hb_data[eta+42][phi-1][depth-1][0].get_statistics())>100){
             Eta=eta; Phi=phi; Depth=depth;
	     Status=hb_data[eta+42][phi-1][depth-1][0].get_status();
	     hb_data[eta+42][phi-1][depth-1][0].get_average(&ped[0],&rms[0]);
	     hb_data[eta+42][phi-1][depth-1][1].get_average(&ped[1],&rms[1]);
	     hb_data[eta+42][phi-1][depth-1][2].get_average(&ped[2],&rms[2]);
	     hb_data[eta+42][phi-1][depth-1][3].get_average(&ped[3],&rms[3]);
	     tree->Fill();
          }
       } 
       sprintf(Subdet,"HE");
       for(int eta=-29;eta<=29;eta++) for(int phi=1;phi<=72;phi++) for(int depth=1;depth<=3;depth++){
         if((Statistic=he_data[eta+42][phi-1][depth-1][0].get_statistics())>100){
            Eta=eta; Phi=phi; Depth=depth;
	    Status=he_data[eta+42][phi-1][depth-1][0].get_status();
	    he_data[eta+42][phi-1][depth-1][0].get_average(&ped[0],&rms[0]);
	    he_data[eta+42][phi-1][depth-1][1].get_average(&ped[1],&rms[1]);
	    he_data[eta+42][phi-1][depth-1][2].get_average(&ped[2],&rms[2]);
	    he_data[eta+42][phi-1][depth-1][3].get_average(&ped[3],&rms[3]);
	    tree->Fill();
         }
      } 
      sprintf(Subdet,"HO");
      for(int eta=-15;eta<=15;eta++) for(int phi=1;phi<=72;phi++) for(int depth=4;depth<=4;depth++){
         if((Statistic=ho_data[eta+42][phi-1][depth-1][0].get_statistics())>100){
             Eta=eta; Phi=phi; Depth=depth;
	     Status=ho_data[eta+42][phi-1][depth-1][0].get_status();
	     ho_data[eta+42][phi-1][depth-1][0].get_average(&ped[0],&rms[0]);
	     ho_data[eta+42][phi-1][depth-1][1].get_average(&ped[1],&rms[1]);
	     ho_data[eta+42][phi-1][depth-1][2].get_average(&ped[2],&rms[2]);
	     ho_data[eta+42][phi-1][depth-1][3].get_average(&ped[3],&rms[3]);
	     tree->Fill();
         }
      } 
      sprintf(Subdet,"HF");
      for(int eta=-42;eta<=42;eta++) for(int phi=1;phi<=72;phi++) for(int depth=1;depth<=2;depth++){
         if((Statistic=hf_data[eta+42][phi-1][depth-1][0].get_statistics())>100){
             Eta=eta; Phi=phi; Depth=depth;
	     Status=hf_data[eta+42][phi-1][depth-1][0].get_status();
	     hf_data[eta+42][phi-1][depth-1][0].get_average(&ped[0],&rms[0]);
	     hf_data[eta+42][phi-1][depth-1][1].get_average(&ped[1],&rms[1]);
	     hf_data[eta+42][phi-1][depth-1][2].get_average(&ped[2],&rms[2]);
	     hf_data[eta+42][phi-1][depth-1][3].get_average(&ped[3],&rms[3]);
	     tree->Fill();
         }
      }
      theFile->Write();
      theFile->Close();
      dataset_seq_number++;
}

void HcalDetDiagPedestalMonitor::LoadReference(){
double ped[4],rms[4];
int Eta,Phi,Depth;
char subdet[10];
TFile *f;
      try{ 
         f = new TFile(ReferenceData.c_str(),"READ");
      }catch(...){ return ;}
      if(!f->IsOpen()) return ;
      TObjString *STR=(TObjString *)f->Get("run number");
      
      if(STR){ string Ref(STR->String()); ReferenceRun=Ref;}
      
      TTree*  t=(TTree*)f->Get("HCAL Pedestal data");
      if(!t) return;
      t->SetBranchAddress("Subdet",   subdet);
      t->SetBranchAddress("eta",      &Eta);
      t->SetBranchAddress("phi",      &Phi);
      t->SetBranchAddress("depth",    &Depth);
      t->SetBranchAddress("cap0_ped", &ped[0]);
      t->SetBranchAddress("cap0_rms", &rms[0]);
      t->SetBranchAddress("cap1_ped", &ped[1]);
      t->SetBranchAddress("cap1_rms", &rms[1]);
      t->SetBranchAddress("cap2_ped", &ped[2]);
      t->SetBranchAddress("cap2_rms", &rms[2]);
      t->SetBranchAddress("cap3_ped", &ped[3]);
      t->SetBranchAddress("cap3_rms", &rms[3]);
      for(int ievt=0;ievt<t->GetEntries();ievt++){
         t->GetEntry(ievt);
	 if(strcmp(subdet,"HB")==0){
	    hb_data[Eta+42][Phi-1][Depth-1][0].set_reference(ped[0],rms[0]);
	    hb_data[Eta+42][Phi-1][Depth-1][1].set_reference(ped[1],rms[1]);
	    hb_data[Eta+42][Phi-1][Depth-1][2].set_reference(ped[2],rms[2]);
	    hb_data[Eta+42][Phi-1][Depth-1][3].set_reference(ped[3],rms[3]);
	 }
	 if(strcmp(subdet,"HE")==0){
	    he_data[Eta+42][Phi-1][Depth-1][0].set_reference(ped[0],rms[0]);
	    he_data[Eta+42][Phi-1][Depth-1][1].set_reference(ped[1],rms[1]);
	    he_data[Eta+42][Phi-1][Depth-1][2].set_reference(ped[2],rms[2]);
	    he_data[Eta+42][Phi-1][Depth-1][3].set_reference(ped[3],rms[3]);
	 }
	 if(strcmp(subdet,"HO")==0){
	    ho_data[Eta+42][Phi-1][Depth-1][0].set_reference(ped[0],rms[0]);
	    ho_data[Eta+42][Phi-1][Depth-1][1].set_reference(ped[1],rms[1]);
	    ho_data[Eta+42][Phi-1][Depth-1][2].set_reference(ped[2],rms[2]);
	    ho_data[Eta+42][Phi-1][Depth-1][3].set_reference(ped[3],rms[3]);
	 }
	 if(strcmp(subdet,"HF")==0){
	    hf_data[Eta+42][Phi-1][Depth-1][0].set_reference(ped[0],rms[0]);
	    hf_data[Eta+42][Phi-1][Depth-1][1].set_reference(ped[1],rms[1]);
	    hf_data[Eta+42][Phi-1][Depth-1][2].set_reference(ped[2],rms[2]);
	    hf_data[Eta+42][Phi-1][Depth-1][3].set_reference(ped[3],rms[3]);
	 }
      }
      f->Close();
      IsReference=true;
} 

void HcalDetDiagPedestalMonitor::SaveHTML(){
   if(!enableHTML) return;

}
void HcalDetDiagPedestalMonitor::beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg,const edm::EventSetup& c){}
void HcalDetDiagPedestalMonitor::endLuminosityBlock(const edm::LuminosityBlock& lumiSeg,const edm::EventSetup& c){}

//define this as a plug-in
DEFINE_ANOTHER_FWK_MODULE(HcalDetDiagPedestalMonitor);
