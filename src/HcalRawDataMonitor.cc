#include "DQM/HcalMonitorTasks/interface/HcalRawDataMonitor.h"

using namespace std;
using namespace edm;

HcalRawDataMonitor::HcalRawDataMonitor(const edm::ParameterSet& ps) {
  Online_                = ps.getParameter<bool>("online");
  mergeRuns_             = ps.getParameter<bool>("mergeRuns");
  enableCleanup_         = ps.getParameter<bool>("enableCleanup");
  debug_                 = ps.getParameter<int>("debug");
  prefixME_              = ps.getParameter<string>("subSystemFolder"); // Hcal
  if (prefixME_.substr(prefixME_.size()-1,prefixME_.size())!="/")
    prefixME_.append("/");
  subdir_                = ps.getParameter<string>("TaskFolder"); // RawDataMonitor_Hcal
  if (subdir_.size()>0 && subdir_.substr(subdir_.size()-1,subdir_.size())!="/")
    subdir_.append("/");
  subdir_=prefixME_+subdir_;
  AllowedCalibTypes_     = ps.getParameter<vector<int> > ("AllowedCalibTypes");
  skipOutOfOrderLS_      = ps.getParameter<bool>("skipOutOfOrderLS");
  NLumiBlocks_           = ps.getParameter<int>("NLumiBlocks");
  makeDiagnostics_       = ps.getUntrackedParameter<bool>("makeDiagnostics",false);
} // HcalRawDataMonitor::HcalRawDataMonitor()

// destructor
HcalRawDataMonitor::~HcalRawDataMonitor(){}

  // Analyze
void HcalRawDataMonitor::analyze(const edm::Event& e, const edm::EventSetup& c){}

// Setup
void HcalRawDataMonitor::setup(void){}


  // BeginJob
void HcalRawDataMonitor::beginJob(){}

  // BeginRun
void HcalRawDataMonitor::beginRun(const edm::Run& run, const edm::EventSetup& c){}

  // Begin LumiBlock
void HcalRawDataMonitor::beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
                            const edm::EventSetup& c) {}

  // End LumiBlock
void HcalRawDataMonitor::endLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
                          const edm::EventSetup& c){}

 // EndJob
void HcalRawDataMonitor::endJob(void){}

  // EndRun
void HcalRawDataMonitor::endRun(const edm::Run& run, const edm::EventSetup& c){}



DEFINE_ANOTHER_FWK_MODULE(HcalRawDataMonitor);

