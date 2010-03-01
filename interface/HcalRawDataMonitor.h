#ifndef DQM_HCALMONITORTASKS_HCALRAWDATAMONITOR_H
#define DQM_HCALMONITORTASKS_HCALRAWDATAMONITOR_H
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
 * $Date: 2010/03/01 14:43:00 $
 * $Revision: 1.1 $
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
  
};

#endif
