#ifndef DQM_HCALMONITORTASKS_GUARD_HCALBASE_H
#define DQM_HCALMONITORTASKS_GUARD_HCALBASE_H

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h" // needed to grab objects

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DQM/HcalMonitorTasks/interface/HcalEtaPhiHists.h"

#include <iostream>
#include <vector>

class HcalBaseDQMonitor : public edm::EDAnalyzer
{

public:

  // Constructor
  HcalBaseDQMonitor(const edm::ParameterSet& ps);
  // Constructor with no arguments
  HcalBaseDQMonitor(){};

  // Destructor
  ~HcalBaseDQMonitor();

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

  // Reset
  void reset(void);

  // cleanup
  void cleanup(void);

  // setup
  void setup(void);
  
  // LumiOutOfOrder
  bool LumiInOrder(int lumisec);

  // IsAllowedCalibType
  bool IsAllowedCalibType();
  int currenttype_;

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
};// class HcalBaseDQMonitor : public edm::EDAnalyzer


#endif
