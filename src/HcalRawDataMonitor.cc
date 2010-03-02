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

  // Initialize an array of MonitorElements
  meChann_DataIntegrityCheck_[0] =meCh_DataIntegrityFED00_;
  meChann_DataIntegrityCheck_[1] =meCh_DataIntegrityFED01_;
  meChann_DataIntegrityCheck_[2] =meCh_DataIntegrityFED02_;
  meChann_DataIntegrityCheck_[3] =meCh_DataIntegrityFED03_;
  meChann_DataIntegrityCheck_[4] =meCh_DataIntegrityFED04_;
  meChann_DataIntegrityCheck_[5] =meCh_DataIntegrityFED05_;
  meChann_DataIntegrityCheck_[6] =meCh_DataIntegrityFED06_;
  meChann_DataIntegrityCheck_[7] =meCh_DataIntegrityFED07_;
  meChann_DataIntegrityCheck_[8] =meCh_DataIntegrityFED08_;
  meChann_DataIntegrityCheck_[9] =meCh_DataIntegrityFED09_;
  meChann_DataIntegrityCheck_[10]=meCh_DataIntegrityFED10_;
  meChann_DataIntegrityCheck_[11]=meCh_DataIntegrityFED11_;
  meChann_DataIntegrityCheck_[12]=meCh_DataIntegrityFED12_;
  meChann_DataIntegrityCheck_[13]=meCh_DataIntegrityFED13_;
  meChann_DataIntegrityCheck_[14]=meCh_DataIntegrityFED14_;
  meChann_DataIntegrityCheck_[15]=meCh_DataIntegrityFED15_;
  meChann_DataIntegrityCheck_[16]=meCh_DataIntegrityFED16_;
  meChann_DataIntegrityCheck_[17]=meCh_DataIntegrityFED17_;
  meChann_DataIntegrityCheck_[18]=meCh_DataIntegrityFED18_;
  meChann_DataIntegrityCheck_[19]=meCh_DataIntegrityFED19_;
  meChann_DataIntegrityCheck_[20]=meCh_DataIntegrityFED20_;
  meChann_DataIntegrityCheck_[21]=meCh_DataIntegrityFED21_;
  meChann_DataIntegrityCheck_[22]=meCh_DataIntegrityFED22_;
  meChann_DataIntegrityCheck_[23]=meCh_DataIntegrityFED23_;
  meChann_DataIntegrityCheck_[24]=meCh_DataIntegrityFED24_;
  meChann_DataIntegrityCheck_[25]=meCh_DataIntegrityFED25_;
  meChann_DataIntegrityCheck_[26]=meCh_DataIntegrityFED26_;
  meChann_DataIntegrityCheck_[27]=meCh_DataIntegrityFED27_;
  meChann_DataIntegrityCheck_[28]=meCh_DataIntegrityFED28_;
  meChann_DataIntegrityCheck_[29]=meCh_DataIntegrityFED29_;
  meChann_DataIntegrityCheck_[30]=meCh_DataIntegrityFED30_;
  meChann_DataIntegrityCheck_[31]=meCh_DataIntegrityFED31_;

  for (int f=0; f<NUMDCCS; f++) {
    for (int s=0; s<15; s++) {
      UScount[f][s]=0;}}

  for (int x=0; x<THREE_FED; x++)
    for (int y=0; y<THREE_SPG; y++)
      HalfHTRDataCorruptionIndicators_  [x][y]=0;

  for (int x=0; x<THREE_FED; x++)
    for (int y=0; y<THREE_SPG; y++)
      LRBDataCorruptionIndicators_  [x][y]=0;
  	 
  for (int x=0; x<TWO___FED; x++)
    for (int y=0; y<TWO__SPGT; y++)
      ChannSumm_DataIntegrityCheck_[x][y]=0;

  for (int f=0; f<NUMDCCS; f++)
    for (int x=0; x<  TWO_CHANN; x++)
      for (int y=0; y<TWO__SPGT; y++)      
	Chann_DataIntegrityCheck_  [f][x][y]=0;

  for (int i=0; i<(NUMDCCS * NUMSPIGS * HTRCHANMAX); i++) 
    hashedHcalDetId_[i]=HcalDetId::Undefined;

} // HcalRawDataMonitor::HcalRawDataMonitor()

// destructor
HcalRawDataMonitor::~HcalRawDataMonitor(){}

// BeginJob
void HcalRawDataMonitor::beginJob(){}

// BeginRun
void HcalRawDataMonitor::beginRun(const edm::Run& run, const edm::EventSetup& c){}

// Begin LumiBlock
void HcalRawDataMonitor::beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
					      const edm::EventSetup& c) {}
// Setup
void HcalRawDataMonitor::setup(void){
  // Call base class setup
  HcalBaseDQMonitor::setup();
  if (!dbe_) return;

  /******* Set up all histograms  ********/
  if (debug_>1)
    std::cout <<"<HcalRawDataMonitor::beginRun>  Setting up histograms"<<endl;
}

// Analyze
void HcalRawDataMonitor::analyze(const edm::Event& e, const edm::EventSetup& c){}

// End LumiBlock
void HcalRawDataMonitor::endLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
					    const edm::EventSetup& c){}
// EndRun -- Anything to do here?
void HcalRawDataMonitor::endRun(const edm::Run& run, const edm::EventSetup& c){}

// EndJob
void HcalRawDataMonitor::endJob(void){
  if (debug_>0) std::cout <<"HcalRawDataMonitor::endJob()"<<std::endl;
  if (enableCleanup_) cleanup(); // when do we force cleanup?
}


DEFINE_ANOTHER_FWK_MODULE(HcalRawDataMonitor);

