#ifndef DQM_HCALMONITORTASKS_HCALRAWDATAMONITOR_H
#define DQM_HCALMONITORTASKS_HCALRAWDATAMONITOR_H

#define DEPTHBINS      4
#define  IETAMIN     -43
#define  IETAMAX      43
#define  IPHIMIN       0
#define  IPHIMAX      71
#define  HBHE_LO_DCC 700
#define  HBHE_HI_DCC 717
#define  HF_LO_DCC   718
#define  HF_HI_DCC   724
#define  HO_LO_DCC   725
#define  HO_HI_DCC   731
#define  NUMDCCS      32
#define  NUMSPIGS     15
#define  HTRCHANMAX   24
//Dimensions of 'LED' plots, grouping bits by hardware space
//  NUMBER_HDWARE = 1 + ((NUMBER +1)*(NUM_HRDWARE_PIECES))
#define  TWO___FED   (1+((2+1)*NUMDCCS)   )
#define  THREE_FED   (1+((3+1)*NUMDCCS)   )
#define  TWO__SPGT   (1+((2+1)*NUMSPIGS)  )
#define  THREE_SPG   (1+((3+1)*NUMSPIGS)  ) 
#define  TWO_CHANN   (1+((2+1)*HTRCHANMAX))

#include "DQM/HcalMonitorTasks/interface/HcalBaseDQMonitor.h"
#include "EventFilter/HcalRawToDigi/interface/HcalUnpacker.h"
#include "EventFilter/HcalRawToDigi/interface/HcalHTRData.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"
#include <math.h>

/** \class HcalRawDataMonitor
 *
 * $Date: 2010/03/01 23:52:57 $
 * $Revision: 1.1.2.1 $
 * \author J. St. John - Boston University
 */
class HcalRawDataMonitor: public HcalBaseDQMonitor {
 public:
  HcalRawDataMonitor(const edm::ParameterSet& ps);
  //Constructor with no arguments
  HcalRawDataMonitor(){};
  ~HcalRawDataMonitor();
 protected:

  // Analyze
  void analyze(const edm::Event& e, const edm::EventSetup& c);

  // BeginJob
  void beginJob();

  // BeginRun
  void beginRun(const edm::Run& run, const edm::EventSetup& c);

  // Begin LumiBlock
  void beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
                            const edm::EventSetup& c) ;

  // End LumiBlock
  void endLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
                          const edm::EventSetup& c);

  // EndJob
  void endJob(void);

  // EndRun
  void endRun(const edm::Run& run, const edm::EventSetup& c);

  // setup
  void setup(void);

  void SetupEtaPhiHists(EtaPhiHists & hh, std::string Name, std::string Units)
  {
    hh.setup(dbe_, Name, Units);
    return;
  }

  std::vector<int> AllowedCalibTypes_;
  bool Online_;
  bool mergeRuns_;
  bool enableCleanup_;
  int debug_;
  std::string prefixME_;
  std::string subdir_;

  int currentLS;
  DQMStore* dbe_;
  int ievt_;
  int levt_; // number of events in current lumi block
  int tevt_; // number of events overall
  MonitorElement* meIevt_;
  MonitorElement* meTevt_;
  MonitorElement* meLevt_;
  bool eventAllowed_;
  bool skipOutOfOrderLS_;
  bool makeDiagnostics_;

  // check that each subdetector is present
  bool HBpresent_, HEpresent_, HOpresent_, HFpresent_;

  // Define problem-tracking monitor elements -- keep here, or in the client?
  MonitorElement *ProblemsVsLB;
  MonitorElement *ProblemsVsLB_HB, *ProblemsVsLB_HE;
  MonitorElement *ProblemsVsLB_HO, *ProblemsVsLB_HF;

  int NLumiBlocks_;

 private:
  MonitorElement* meCh_DataIntegrityFED00_;   //DataIntegrity for channels in FED 00
  MonitorElement* meCh_DataIntegrityFED01_;   //DataIntegrity for channels in FED 01
  MonitorElement* meCh_DataIntegrityFED02_;   //DataIntegrity for channels in FED 02
  MonitorElement* meCh_DataIntegrityFED03_;   //DataIntegrity for channels in FED 03
  MonitorElement* meCh_DataIntegrityFED04_;   //DataIntegrity for channels in FED 04
  MonitorElement* meCh_DataIntegrityFED05_;   //DataIntegrity for channels in FED 05
  MonitorElement* meCh_DataIntegrityFED06_;   //DataIntegrity for channels in FED 06
  MonitorElement* meCh_DataIntegrityFED07_;   //DataIntegrity for channels in FED 07
  MonitorElement* meCh_DataIntegrityFED08_;   //DataIntegrity for channels in FED 08
  MonitorElement* meCh_DataIntegrityFED09_;   //DataIntegrity for channels in FED 09
  MonitorElement* meCh_DataIntegrityFED10_;   //DataIntegrity for channels in FED 10
  MonitorElement* meCh_DataIntegrityFED11_;   //DataIntegrity for channels in FED 11
  MonitorElement* meCh_DataIntegrityFED12_;   //DataIntegrity for channels in FED 12
  MonitorElement* meCh_DataIntegrityFED13_;   //DataIntegrity for channels in FED 13
  MonitorElement* meCh_DataIntegrityFED14_;   //DataIntegrity for channels in FED 14
  MonitorElement* meCh_DataIntegrityFED15_;   //DataIntegrity for channels in FED 15
  MonitorElement* meCh_DataIntegrityFED16_;   //DataIntegrity for channels in FED 16
  MonitorElement* meCh_DataIntegrityFED17_;   //DataIntegrity for channels in FED 17
  MonitorElement* meCh_DataIntegrityFED18_;   //DataIntegrity for channels in FED 18
  MonitorElement* meCh_DataIntegrityFED19_;   //DataIntegrity for channels in FED 19
  MonitorElement* meCh_DataIntegrityFED20_;   //DataIntegrity for channels in FED 20
  MonitorElement* meCh_DataIntegrityFED21_;   //DataIntegrity for channels in FED 21
  MonitorElement* meCh_DataIntegrityFED22_;   //DataIntegrity for channels in FED 22
  MonitorElement* meCh_DataIntegrityFED23_;   //DataIntegrity for channels in FED 23
  MonitorElement* meCh_DataIntegrityFED24_;   //DataIntegrity for channels in FED 24
  MonitorElement* meCh_DataIntegrityFED25_;   //DataIntegrity for channels in FED 25
  MonitorElement* meCh_DataIntegrityFED26_;   //DataIntegrity for channels in FED 26
  MonitorElement* meCh_DataIntegrityFED27_;   //DataIntegrity for channels in FED 27
  MonitorElement* meCh_DataIntegrityFED28_;   //DataIntegrity for channels in FED 28
  MonitorElement* meCh_DataIntegrityFED29_;   //DataIntegrity for channels in FED 29
  MonitorElement* meCh_DataIntegrityFED30_;   //DataIntegrity for channels in FED 30
  MonitorElement* meCh_DataIntegrityFED31_;   //DataIntegrity for channels in FED 31
  // handy array of pointers to pointers...
  MonitorElement* meChann_DataIntegrityCheck_[NUMDCCS];

  uint64_t UScount[NUMDCCS][NUMSPIGS];
  float HalfHTRDataCorruptionIndicators_  [THREE_FED][THREE_SPG];  
  float LRBDataCorruptionIndicators_      [THREE_FED][THREE_SPG];  
  float ChannSumm_DataIntegrityCheck_     [TWO___FED][TWO__SPGT];
  float Chann_DataIntegrityCheck_[NUMDCCS][TWO_CHANN][TWO__SPGT];

  HcalDetId hashedHcalDetId_[NUMDCCS * NUMSPIGS * HTRCHANMAX];
  
};

#endif
