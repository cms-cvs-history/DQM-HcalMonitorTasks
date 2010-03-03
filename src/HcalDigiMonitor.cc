#include "DQM/HcalMonitorTasks/interface/HcalDigiMonitor.h"
#include "EventFilter/HcalRawToDigi/interface/HcalDCCHeader.h"
#include <cmath>

using namespace std;
using namespace edm;

// constructor
HcalDigiMonitor::HcalDigiMonitor(const ParameterSet& ps) 
{
  Online_                = ps.getUntrackedParameter<bool>("online",false);
  mergeRuns_             = ps.getUntrackedParameter<bool>("mergeRuns",false);
  enableCleanup_         = ps.getUntrackedParameter<bool>("enableCleanup",false);
  debug_                 = ps.getUntrackedParameter<int>("debug",0);
  prefixME_              = ps.getUntrackedParameter<string>("subSystemFolder","Hcal/");
  if (prefixME_.substr(prefixME_.size()-1,prefixME_.size())!="/")
    prefixME_.append("/");
  subdir_                = ps.getUntrackedParameter<string>("TaskFolder","DigiMonitor_Hcal"); 
  if (subdir_.size()>0 && subdir_.substr(subdir_.size()-1,subdir_.size())!="/")
    subdir_.append("/");
  subdir_=prefixME_+subdir_;
  AllowedCalibTypes_     = ps.getUntrackedParameter<vector<int> > ("AllowedCalibTypes");
  skipOutOfOrderLS_      = ps.getUntrackedParameter<bool>("skipOutOfOrderLS",true);
  NLumiBlocks_           = ps.getUntrackedParameter<int>("NLumiBlocks",4000);
  makeDiagnostics_       = ps.getUntrackedParameter<bool>("makeDiagnostics",false);

  digiLabel_     = ps.getUntrackedParameter<edm::InputTag>("digiLabel");
  shapeThresh_   = ps.getUntrackedParameter<int>("shapeThresh",50);
  //shapeThresh_ is used for plotting pulse shapes for all digis with ADC sum > shapeThresh_;
  shapeThreshHB_ = ps.getUntrackedParameter<int>("shapeThreshHB",shapeThresh_);
  shapeThreshHE_ = ps.getUntrackedParameter<int>("shapeThreshHE",shapeThresh_);
  shapeThreshHF_ = ps.getUntrackedParameter<int>("shapeThreshHF",shapeThresh_);
  shapeThreshHO_ = ps.getUntrackedParameter<int>("shapeThreshHO",shapeThresh_);

  if (debug_>0)
    std::cout <<"<HcalDigiMonitor> Digi shape ADC threshold set to: >" << shapeThresh_ << std::endl;
  
  // Specify which tests to run when looking for problem digis
  digi_checkoccupancy_ = ps.getUntrackedParameter<bool>("checkForMissingDigis",false); // off by default -- checked by dead cell monitor
  digi_checkcapid_     = ps.getUntrackedParameter<bool>("checkCapID",true);
  digi_checkdigisize_  = ps.getUntrackedParameter<bool>("checkDigiSize",true);
  digi_checkadcsum_    = ps.getUntrackedParameter<bool>("checkADCsum",true);
  digi_checkdverr_     = ps.getUntrackedParameter<bool>("checkDVerr",true);
  mindigisize_ = ps.getUntrackedParameter<int>("minDigiSize",10);
  maxdigisize_ = ps.getUntrackedParameter<int>("maxDigiSize",10);

  if (debug_>1)
    {
      std::cout <<"<HcalDigiMonitor> Checking for the following problems:"<<std::endl; 
      if (digi_checkcapid_) std::cout <<"\tChecking that cap ID rotation is correct;"<<std::endl;
      if (digi_checkdigisize_) std::cout <<"\tChecking that digi size is between ["<<mindigisize_<<" - "<<maxdigisize_<<"];"<<std::endl;
      if (digi_checkadcsum_) std::cout <<"\tChecking that ADC sum of digi is greater than 0;"<<std::endl; 
      if (digi_checkdverr_) std::cout <<"\tChecking that data valid bit is true and digi error bit is false;\n"<<std::endl;
    }
  
  shutOffOrbitTest_ = ps.getUntrackedParameter<bool>("shutOffOrbitTest",false);
  DigiMonitor_ExpectedOrbitMessageTime_=ps.getUntrackedParameter<int>("ExpectedOrbitMessageTime",3559); // -1 means that orbit mismatches won't be checked
}

// destructor
HcalDigiMonitor::~HcalDigiMonitor() {}

// Checks capid rotation; returns false if no problems with rotation
static bool bitUpset(int last, int now){
  if(last ==-1) return false;
  int v = last+1; 
  if(v==4) v=0;
  if(v==now) return false;
  return true;
} // static bool bitUpset(...)

void HcalDigiMonitor::cleanup()
{
  // Need to add code to clear out subfolders as well?
  if (debug_>0) std::cout <<"HcalDigiMonitor::cleanup()"<<std::endl;
  if (!enableCleanup_) return;
  if (dbe_)
    {
      // removeContents doesn't remove subdirectories
      dbe_->setCurrentFolder(subdir_);
      dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"digi_parameters");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"bad_digis/bad_digi_occupancy");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"bad_digis/1D_digi_plots");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"bad_digis/badcapID");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"bad_digis/data_invalid_error");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"bad_digis/bad_reportUnpackerErrors");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"bad_digis/baddigisize");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"digi_info");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"bad_digis/badfibBCNoff");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"good_digis/1D_digi_plots");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"good_digis/digi_occupancy");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"bad_digis/bad_digi_occupancy");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"bad_digis");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"good_digis/");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"digi_info/HB");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"digi_info/HE");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"digi_info/HO");  dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"digi_info/HF");  dbe_->removeContents();
    } // if(dbe_)

} // void HcalDigiMonitor::cleanup();


void HcalDigiMonitor::endRun(const edm::Run& run, const edm::EventSetup& c)
{
  // Anything to do here?
}

void HcalDigiMonitor::endJob()
{
  if (debug_>0) std::cout <<"HcalDigiMonitor::endJob()"<<std::endl;
  if (enableCleanup_) cleanup(); // when do we force cleanup?
}


void HcalDigiMonitor::setup()
{
  // Call base class setup
  HcalBaseDQMonitor::setup();
  if (!dbe_) return;

  /******* Set up all histograms  ********/
  if (debug_>1)
    std::cout <<"<HcalDigiMonitor::beginRun>  Setting up histograms"<<endl;

  ostringstream name;
  dbe_->setCurrentFolder(subdir_);
  
  dbe_->setCurrentFolder(subdir_+"digi_parameters");
  MonitorElement* ExpectedOrbit = dbe_->bookInt("ExpectedOrbitMessageTime");
  ExpectedOrbit->Fill(DigiMonitor_ExpectedOrbitMessageTime_);

  MonitorElement* shapeT = dbe_->bookInt("DigiShapeThresh");
  shapeT->Fill(shapeThresh_);
  MonitorElement* shapeTHB = dbe_->bookInt("DigiShapeThreshHB");
  shapeTHB->Fill(shapeThreshHB_);
  MonitorElement* shapeTHE = dbe_->bookInt("DigiShapeThreshHE");
  shapeTHE->Fill(shapeThreshHE_);
  MonitorElement* shapeTHO = dbe_->bookInt("DigiShapeThreshHO");
  shapeTHO->Fill(shapeThreshHO_);
  MonitorElement* shapeTHF = dbe_->bookInt("DigiShapeThreshHF");
  shapeTHF->Fill(shapeThreshHF_);
  
  dbe_->setCurrentFolder(subdir_+"bad_digis/bad_digi_occupancy");
  SetupEtaPhiHists(DigiErrorsByDepth,"Bad Digi Map","");
  dbe_->setCurrentFolder(subdir_+"bad_digis/1D_digi_plots");
  ProblemsVsLB=dbe_->bookProfile("BadDigisVsLB","# Bad Digis vs Luminosity block;Lumi block;# of Bad digis",
				  NLumiBlocks_,0.5,NLumiBlocks_+0.5,0,10000);
  ProblemsVsLB_HB=dbe_->bookProfile("HB Bad Quality Digis vs LB","HB Bad Quality Digis vs Luminosity Block",
				     NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				     0,10000);   
  ProblemsVsLB_HE=dbe_->bookProfile("HE Bad Quality Digis vs LB","HE Bad Quality Digis vs Luminosity Block",
				     NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				     0,10000);
  ProblemsVsLB_HO=dbe_->bookProfile("HO Bad Quality Digis vs LB","HO Bad Quality Digis vs Luminosity Block",
				     NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				     0,10000);
  ProblemsVsLB_HF=dbe_->bookProfile("HF Bad Quality Digis vs LB","HF Bad Quality Digis vs Luminosity Block",
				     NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				     0,10000);
  
  if (makeDiagnostics_) // not yet used
    {
      dbe_->setCurrentFolder(subdir_+"bad_digis/badcapID");
      SetupEtaPhiHists(DigiErrorsBadCapID," Digis with Bad Cap ID Rotation", "");
      dbe_->setCurrentFolder(subdir_+"bad_digis/data_invalid_error");
      SetupEtaPhiHists(DigiErrorsDVErr," Digis with Data Invalid or Error Bit Set", "");
    }
  
  dbe_->setCurrentFolder(subdir_+"bad_digis/bad_reportUnpackerErrors");
  SetupEtaPhiHists(DigiErrorsUnpacker," Bad Unpacker Digis", "");
  
  dbe_->setCurrentFolder(subdir_+"bad_digis/baddigisize");
  SetupEtaPhiHists(DigiErrorsBadDigiSize," Digis with Bad Size", "");
  
  dbe_->setCurrentFolder(subdir_+"digi_info");
  
  h_valid_digis=dbe_->book1D("ValidEvents","Events with minimum number of valid digis",2,-0.5,1.5);
  h_valid_digis->setBinLabel(1,"Valid");
  h_valid_digis->setBinLabel(2,"Invalid");
  
  h_invalid_orbitnumMod103=dbe_->book1D("InvalidDigiEvents_ORN","Orbit Number (mod 103) for Events with Many Unpacker Errors",103,-0.5,102.5);
  h_invalid_bcn=dbe_->book1D("InvalidDigiEvents_BCN","Bunch Crossing Number fo Events with Many Unpacker Errors",3464,-0.5,3563.5);

  DigiSize = dbe_->book2D("Digi Size", "Digi Size",4,0,4,20,-0.5,19.5);
  DigiSize->setBinLabel(1,"HB",1);
  DigiSize->setBinLabel(2,"HE",1);
  DigiSize->setBinLabel(3,"HO",1);
  DigiSize->setBinLabel(4,"HF",1);
  DigiSize->setAxisTitle("Subdetector",1);
  DigiSize->setAxisTitle("Digi Size",2);
  
  dbe_->setCurrentFolder(subdir_+"bad_digis/badfibBCNoff");
  SetupEtaPhiHists(DigiErrorsBadFibBCNOff," Digis with non-zero Fiber Orbit Msg Idle BCN Offsets", "");
  
  dbe_->setCurrentFolder(subdir_+"good_digis/1D_digi_plots");
  HBocc_vs_LB=dbe_->bookProfile("HBoccVsLB","HB digi occupancy vs Luminosity Block;Lumi block;# of Good digis",
				 NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				 0,2600);
  HEocc_vs_LB=dbe_->bookProfile("HEoccVsLB","HE digi occupancy vs Luminosity Block;Lumi block;# of Good digis",
				 NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				 0,2600);
  HOocc_vs_LB=dbe_->bookProfile("HOoccVsLB","HO digi occupancy vs Luminosity Block;Lumi block;# of Good digis",
				 NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				 0,2200);
  HFocc_vs_LB=dbe_->bookProfile("HFoccVsLB","HF digi occupancy vs Luminosity Block;Lumi block;# of Good digis",
				 NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				 0,1800);
  
  dbe_->setCurrentFolder(subdir_+"good_digis/digi_occupancy");
  SetupEtaPhiHists(DigiOccupancyByDepth," Digi Eta-Phi Occupancy Map","");
  DigiOccupancyPhi= dbe_->book1D("Digi Phi Occupancy Map",
				  "Digi Phi Occupancy Map;i#phi;# of Events",
				  72,0.5,72.5);
  DigiOccupancyEta= dbe_->book1D("Digi Eta Occupancy Map",
				  "Digi Eta Occupancy Map;i#eta;# of Events",
				  83,-41.5,41.5);
  DigiOccupancyVME = dbe_->book2D("Digi VME Occupancy Map",
				   "Digi VME Occupancy Map;HTR Slot;VME Crate Id",
				   40,-0.25,19.75,18,-0.5,17.5);
  
  DigiOccupancySpigot = dbe_->book2D("Digi Spigot Occupancy Map",
				      "Digi Spigot Occupancy Map;Spigot;DCC Id",
				      HcalDCCHeader::SPIGOT_COUNT,-0.5,HcalDCCHeader::SPIGOT_COUNT-0.5,
				      36,-0.5,35.5);
  
  dbe_->setCurrentFolder(subdir_+"bad_digis/bad_digi_occupancy");
  DigiErrorVME = dbe_->book2D("Digi VME Error Map",
			       "Digi VME Error Map;HTR Slot;VME Crate Id",
			       40,-0.25,19.75,18,-0.5,17.5);
  
  DigiErrorSpigot = dbe_->book2D("Digi Spigot Error Map",
				  "Digi Spigot Error Map;Spigot;DCC Id",
				  HcalDCCHeader::SPIGOT_COUNT,-0.5,HcalDCCHeader::SPIGOT_COUNT-0.5,
				  36,-0.5,35.5);
  
  dbe_->setCurrentFolder(subdir_+"bad_digis");
  int nbins = sizeof(bins_cellcount_new)/sizeof(float)-1;

  DigiBQ = dbe_->book1D("NumBadQualDigis","# Bad Qual Digis within Digi Collection",nbins, bins_cellcount_new);

  nbins=sizeof(bins_fraccount_new)/sizeof(float)-1;

  DigiBQFrac =  dbe_->book1D("Bad Digi Fraction","Bad Digi Fraction;Bad Quality Digi Fraction for digis in collection; # of Events",
			     nbins, bins_fraccount_new);
  
  nbins = sizeof(bins_cellcount_new)/sizeof(float)-1;
  DigiUnpackerErrorCount = dbe_->book1D("Unpacker Error Count", "Number of Bad Digis from Unpacker; Bad Unpacker Digis; # of Events",nbins, bins_cellcount_new);
  
  nbins=sizeof(bins_fraccount_new)/sizeof(float)-1;
  DigiUnpackerErrorFrac = dbe_->book1D("Unpacker Bad Digi Fraction", 
				       "Bad Digis From Unpacker/ (Bad Digis From Unpacker + Good Digis); Bad Unpacker Fraction; # of Events",
				       nbins,bins_fraccount_new);
  
  dbe_->setCurrentFolder(subdir_+"good_digis/");
  DigiNum = dbe_->book1D("NumGoodDigis","# of Digis;# of Good Digis;# of Events",DIGI_NUM+1,-0.5,DIGI_NUM+1-0.5);
    
  setupSubdetHists(hbHists,"HB");
  setupSubdetHists(heHists,"HE");
  setupSubdetHists(hoHists,"HO");
  setupSubdetHists(hfHists,"HF");

  this->reset();
  return;
} // void HcalDigiMonitor::setup()

void HcalDigiMonitor::beginRun(const edm::Run& run, const edm::EventSetup& c)
{
  HcalBaseDQMonitor::beginRun(run,c);
  if (mergeRuns_ && tevt_>0) return; // don't reset counters if merging runs

  if (debug_>1) std::cout <<"\t<HcalDigiMonitor::setup> Getting conditions from DB!"<<std::endl;
  c.get<HcalDbRecord>().get(conditions_);

  if (tevt_==0) this->setup(); // create all histograms; not necessary if merging runs together
  if (mergeRuns_==false) this->reset(); // call reset at start of all runs
} // void HcalDigiMonitor::beginRun()


void HcalDigiMonitor::setupSubdetHists(DigiHists& hist, std::string subdet)
{
  if (!dbe_) return;
  stringstream name;
  int nChan=0;
  if (subdet=="HB" || subdet=="HE") nChan=2592;
  else if (subdet == "HO") nChan=2160;
  else if (subdet == "HF") nChan=1728;

  dbe_->setCurrentFolder(subdir_+"digi_info/"+subdet);
  hist.shape = dbe_->book1D(subdet+" Digi Shape",subdet+" Digi Shape;Time Slice",10,-0.5,9.5);
  hist.shapeThresh = dbe_->book1D(subdet+" Digi Shape - over thresh",
				   subdet+" Digi Shape - over thresh;Time slice",
				   10,-0.5,9.5);
  // Create plots of sums of adjacent time slices
  for (int ts=0;ts<9;++ts)
    {
      name<<subdet<<" Plus Time Slices "<<ts<<" and "<<ts+1;
      hist.TS_sum_plus.push_back(dbe_->book1D(name.str().c_str(),name.str().c_str(),50,-5.5,44.5));
      name.str("");
      name<<subdet<<" Minus Time Slices "<<ts<<" and "<<ts+1;
      hist.TS_sum_minus.push_back(dbe_->book1D(name.str().c_str(),name.str().c_str(),50,-5.5,44.5));
      name.str("");
    }
  hist.presample= dbe_->book1D(subdet+" Digi Presamples",subdet+" Digi Presamples",50,-0.5,49.5);
  hist.BQ = dbe_->book1D(subdet+" Bad Quality Digis",subdet+" Bad Quality Digis",nChan+1,-0.5,nChan+0.5);
  //(hist.BQ->getTH1F())->LabelsOption("v");
  hist.BQFrac = dbe_->book1D(subdet+" Bad Quality Digi Fraction",subdet+" Bad Quality Digi Fraction",DIGI_BQ_FRAC_NBINS,(0-0.5/(DIGI_BQ_FRAC_NBINS-1)),1+0.5/(DIGI_BQ_FRAC_NBINS-1));
  hist.DigiFirstCapID = dbe_->book1D(subdet+" Capid 1st Time Slice",subdet+" Capid for 1st Time Slice;CapId (T0)- 1st CapId (T0);# of Events",7,-3.5,3.5);

  hist.DVerr = dbe_->book1D(subdet+" Data Valid Err Bits",subdet+" QIE Data Valid Err Bits",4,-0.5,3.5);
  hist.DVerr ->setBinLabel(1,"Err=0, DV=0",1);
  hist.DVerr ->setBinLabel(2,"Err=0, DV=1",1);
  hist.DVerr ->setBinLabel(3,"Err=1, DV=0",1);
  hist.DVerr ->setBinLabel(4,"Err=1, DV=1",1);
  hist.CapID = dbe_->book1D(subdet+" CapID",subdet+" CapID",4,-0.5,3.5);
  hist.ADC = dbe_->book1D(subdet+" ADC count per time slice",subdet+" ADC count per time slice",200,-0.5,199.5);
  hist.ADCsum = dbe_->book1D(subdet+" ADC sum", subdet+" ADC sum",200,-0.5,199.5);
  hist.fibBCNOff = dbe_->book1D(subdet+" Fiber Orbit Message Idle BCN Offset", subdet+" Fiber Orbit Message Idle BCN Offset;Offset from Expected",
				 15, -7.5, 7.5);
}

void HcalDigiMonitor::analyze(edm::Event const&e, edm::EventSetup const&s)
{
  if (!IsAllowedCalibType()) return;
  if (LumiInOrder(e.luminosityBlock())==false) return;
  
  // Now get collections we need

  // try to get digis
  edm::Handle<HBHEDigiCollection> hbhe_digi;
  edm::Handle<HODigiCollection> ho_digi;
  edm::Handle<HFDigiCollection> hf_digi;

  if (!(e.getByLabel(digiLabel_,hbhe_digi)))
    {
      LogWarning("HcalDigiMonitor")<< digiLabel_<<" hbhe_digi not available";
      return;
    }
  
  if (!(e.getByLabel(digiLabel_,hf_digi)))
    {
      LogWarning("HcalDigiMonitor")<< digiLabel_<<" hf_digi not available";
      return;
    }
  if (!(e.getByLabel(digiLabel_,ho_digi)))
    {
      LogWarning("HcalDigiMonitor")<< digiLabel_<<" ho_digi not available";
      return;
    }
  edm::Handle<HcalUnpackerReport> report;  
  if (!(e.getByLabel(digiLabel_,report)))
    {
      LogWarning("HcalDigiMonitor")<< digiLabel_<<" unpacker report not available";
      return;
    }

  // all objects grabbed; event is good
  if (debug_>1) std::cout <<"\t<HcalDigiMonitor::analyze>  Processing good event! event # = "<<ievt_<<endl;

  HcalBaseDQMonitor::analyze(e,s); // base class increments ievt_, etc. counters

  // Digi collection was grabbed successfully; process the Event
  processEvent(*hbhe_digi, *ho_digi, *hf_digi, *conditions_,
	       *report, e.orbitNumber(),e.bunchCrossing());

} //void HcalDigiMonitor::analyze(...)

void HcalDigiMonitor::processEvent(const HBHEDigiCollection& hbhe,
				   const HODigiCollection& ho,
				   const HFDigiCollection& hf,
				   const HcalDbService& cond,
				   const HcalUnpackerReport& report,
				   int orN, int bcN)
{ 
  if(!dbe_) 
    { 
      if(debug_) 
	std::cout <<"HcalDigiMonitor::processEvent   DQMStore not instantiated!!!"<<std::endl; 
      return; 
    }

  // Skip events in which minimal good digis found -- still getting some strange (calib?) events through DQM

  DigiUnpackerErrorCount->Fill(report.badQualityDigis());
  
  unsigned int allgooddigis= hbhe.size()+ho.size()+hf.size();
  // bad threshold:  ignore events in which bad outnumber good by more than 100:1
  // (one RBX in HBHE seems to send valid data occasionally even on QIE resets, which is why we can't just require allgooddigis==0 when looking for events to skip)
  if ((allgooddigis==0) ||
      (1.*report.badQualityDigis()>100*allgooddigis))
    {
      h_valid_digis->Fill(1);
      if (bcN>-1)
	h_invalid_bcn->Fill(bcN);
      if (orN>-1)
	h_invalid_orbitnumMod103->Fill(orN%103);
      return;
    }
  
  h_valid_digis->Fill(0);

  hbHists.count_bad=0;
  hbHists.count_good=0;
  heHists.count_bad=0;
  heHists.count_good=0;
  hoHists.count_bad=0;
  hoHists.count_good=0;
  hfHists.count_bad=0;
  hfHists.count_good=0;

  // Check unpacker report for bad digis

  typedef std::vector<DetId> DetIdVector;

  DetIdVector::const_iterator dummy = report.bad_quality_begin();
  DetIdVector::const_iterator dumm2 = report.bad_quality_end();

  for ( DetIdVector::const_iterator baddigi_iter=report.bad_quality_begin(); 
	baddigi_iter != report.bad_quality_end();
	++baddigi_iter)
    {
      HcalDetId id(baddigi_iter->rawId());
      int rDepth = id.depth();
      int rPhi   = id.iphi();
      int rEta   = id.ieta();
      rEta = CalcEtaBin(id.subdet(), rEta, rDepth);
      if (id.subdet()==HcalBarrel) ++hbHists.count_bad;
      else if (id.subdet()==HcalEndcap) ++heHists.count_bad;
      else if (id.subdet()==HcalForward) ++hfHists.count_bad;
      else if (id.subdet()==HcalOuter) ++hoHists.count_bad;
      else 
	continue; // skip anything that isn't HB, HE, HO, HF
      // extra protection against nonsensical values -- prevents occasional crashes
      if (rEta < 85 && rEta >= 0 
	  && (rPhi-1) >= 0 && (rPhi-1)<72 
	  && (rDepth-1) >= 0 && (rDepth-1)<4)
	{
	  ++badunpackerreport[rEta][rPhi-1][rDepth-1];
	  ++baddigis[rEta][rPhi-1][rDepth-1];  
	}

    }

 ///////////////////////////////////////// Loop over HBHE

  int firsthbcap=-1;
  int firsthecap=-1;
  int firsthocap=-1;
  int firsthfcap=-1;

  for (HBHEDigiCollection::const_iterator j=hbhe.begin(); j!=hbhe.end(); ++j)
    {
	const HBHEDataFrame digi = (const HBHEDataFrame)(*j);
	  
	if (digi.id().subdet()==HcalBarrel)
	  {
	    if (!HBpresent_) continue;
	    process_Digi(digi, hbHists, firsthbcap);
	  }
	else if (digi.id().subdet()==HcalEndcap)
	  {
	    if (!HEpresent_) continue;
	    process_Digi(digi, heHists,firsthecap);
	  }
	
    }

  // Fill good digis vs lumi block; also fill bad errors?
  HBocc_vs_LB->Fill(currentLS,hbHists.count_good);
  HEocc_vs_LB->Fill(currentLS,heHists.count_good);

  // Calculate number of bad quality cells and bad quality fraction
  if (HBpresent_ && (hbHists.count_good>0 || hbHists.count_bad>0))
    {
      int counter=hbHists.count_bad;
      if (counter<DIGI_SUBDET_NUM)
	++hbHists.count_BQ[counter];
      float counter2 = (1.*hbHists.count_bad)/(hbHists.count_bad+hbHists.count_good)*(DIGI_BQ_FRAC_NBINS-1);
      if (counter2<DIGI_SUBDET_NUM) ++hbHists.count_BQFrac[(int)counter2];
    }
  if (HEpresent_ && (heHists.count_good>0 || heHists.count_bad>0))
    {
      int counter=heHists.count_bad;
      if (counter<DIGI_SUBDET_NUM)
	++heHists.count_BQ[counter];
      float counter2 = (1.*heHists.count_bad)/(heHists.count_bad+heHists.count_good)*(DIGI_BQ_FRAC_NBINS-1);
      if (counter2<DIGI_SUBDET_NUM) ++heHists.count_BQFrac[int(counter2)];
    }

  //////////////////////////////////// Loop over HO collection
  if (HOpresent_)
    {
      for (HODigiCollection::const_iterator j=ho.begin(); j!=ho.end(); ++j)
	{
	  const HODataFrame digi = (const HODataFrame)(*j);
	  process_Digi(digi, hoHists, firsthocap);
	} // for (HODigiCollection)
   
      if (hoHists.count_bad>0 || hoHists.count_good>0)
	{
	  int counter=hoHists.count_bad;
	  if (counter<DIGI_SUBDET_NUM)
	    ++hoHists.count_BQ[counter];

	  float counter2 = (1.*hoHists.count_bad)/(hoHists.count_bad+hoHists.count_good)*(DIGI_BQ_FRAC_NBINS-1);
	  if (counter2<DIGI_SUBDET_NUM) ++hoHists.count_BQFrac[int(counter2)];
	}
      HOocc_vs_LB->Fill(currentLS,hoHists.count_good);
    } // if (HOpresent_)
  
  /////////////////////////////////////// Loop over HF collection
  if (HFpresent_)
    {
      for (HFDigiCollection::const_iterator j=hf.begin(); j!=hf.end(); ++j)
	{
	  const HFDataFrame digi = (const HFDataFrame)(*j);
	  process_Digi(digi, hfHists, firsthfcap);
	} // for (HFDigiCollection)

      if (hfHists.count_bad>0 || hfHists.count_good>0)
	{
	  int counter=hfHists.count_bad;
	  if (counter<DIGI_SUBDET_NUM)
	    ++hfHists.count_BQ[counter];
	  float counter2 = (1.*hfHists.count_bad)/(hfHists.count_bad+hfHists.count_good)*(DIGI_BQ_FRAC_NBINS-1);
	  if (counter2<DIGI_SUBDET_NUM) ++hfHists.count_BQFrac[int(counter2)];
	}
      HFocc_vs_LB->Fill(currentLS,hfHists.count_good);
    } // if (HFpresent_)
  

  // This only counts digis that are present but bad somehow; it does not count digis that are missing
  int count_good=hbHists.count_good+heHists.count_good+hoHists.count_good+hfHists.count_good;
  int count_bad=hbHists.count_bad+heHists.count_bad+hoHists.count_bad+hfHists.count_bad;

  if (count_good<DIGI_NUM)
    ++diginum[count_good];

  // Fill bad quality histograms
  DigiUnpackerErrorFrac->Fill(1.*report.badQualityDigis()/(report.badQualityDigis()+count_good));
  DigiBQ->Fill(count_bad);
  if (count_bad>0 || count_good>0)
    DigiBQFrac->Fill(1.*count_bad/(count_bad+count_good));

  // Call 'update' on all histograms so that they update in online DQM  
  UpdateHists(hbHists);
  UpdateHists(heHists);
  UpdateHists(hoHists);
  UpdateHists(hfHists);

  // Now update global (non-subdetector-specific) histograms
  DigiNum->update();
  DigiErrorVME->update();
  DigiErrorSpigot->update();
  DigiBQ->update();
  DigiBQFrac->update();
  DigiUnpackerErrorCount->update();
  DigiUnpackerErrorFrac->update();

  // Update eta-phi hists
  for (unsigned int zz=0;zz<DigiErrorOccupancyByDepth.depth.size();++zz)
      DigiErrorOccupancyByDepth.depth[zz]->update();
  for (unsigned int zz=0;zz<DigiErrorsByDepth.depth.size();++zz)
      DigiErrorsByDepth.depth[zz]->update();
  for (unsigned int zz=0;zz<DigiErrorsBadCapID.depth.size();++zz)
      DigiErrorsBadCapID.depth[zz]->update();
  for (unsigned int zz=0;zz<DigiErrorsDVErr.depth.size();++zz)
      DigiErrorsDVErr.depth[zz]->update();
  for (unsigned int zz=0;zz<DigiErrorsBadDigiSize.depth.size();++zz)
      DigiErrorsBadDigiSize.depth[zz]->update();
  for (unsigned int zz=0;zz<DigiErrorsBadADCSum.depth.size();++zz)
      DigiErrorsBadADCSum.depth[zz]->update();
  for (unsigned int zz=0;zz<DigiErrorsUnpacker.depth.size();++zz)
      DigiErrorsUnpacker.depth[zz]->update();
  for (unsigned int zz=0;zz<DigiErrorsBadFibBCNOff.depth.size();++zz)
    DigiErrorsBadFibBCNOff.depth[zz]->update();

  DigiOccupancyEta->update();
  DigiOccupancyPhi->update();
  DigiOccupancyVME->update();
  DigiOccupancySpigot->update();
  DigiSize->update();

  // Fill problems vs. lumi block plots
  ProblemsVsLB->Fill(currentLS,count_bad);
  ProblemsVsLB_HB->Fill(currentLS,hbHists.count_bad);
  ProblemsVsLB_HE->Fill(currentLS,heHists.count_bad);
  ProblemsVsLB_HO->Fill(currentLS,hoHists.count_bad);
  ProblemsVsLB_HF->Fill(currentLS,hfHists.count_bad);

  // Call fill method every checkNevents
  //fill_Nevents();
  
  return;
} // void HcalDigiMonitor::processEvent(...)



template <class DIGI>
int HcalDigiMonitor::process_Digi(DIGI& digi, DigiHists& h, int& firstcap)
{
  int err=0x0;
  bool bitUp = false;
  int ADCcount=0;
  
  int shapeThresh=0;
  if (digi.id().subdet()==HcalBarrel)
      shapeThresh=shapeThreshHB_;
  else if (digi.id().subdet()==HcalEndcap)
      shapeThresh=shapeThreshHE_;
  else if (digi.id().subdet()==HcalOuter)
      shapeThresh=shapeThreshHO_;
  else if (digi.id().subdet()==HcalForward)
      shapeThresh=shapeThreshHF_;
  
  int iEta = digi.id().ieta();
  int iPhi = digi.id().iphi();
  int iDepth = digi.id().depth();
  int calcEta = CalcEtaBin(digi.id().subdet(),iEta,iDepth);
	  
  // Check that digi size is correct
  if (digi.size()<mindigisize_ || digi.size()>maxdigisize_)
    {
      if (digi_checkdigisize_) err|=0x1;
      ++baddigisize[calcEta][iPhi-1][iDepth-1];
    }
  // Check digi size; if > 20, increment highest bin of digisize array
  if (digi.size()<20)
    ++digisize[static_cast<int>(digi.size())][digi.id().subdet()-1];
  else
    ++digisize[19][digi.id().subdet()-1];

  // loop over time slices of digi to check capID and errors
  ++h.count_presample[digi.presamples()];

  // Check CapID rotation
  if (firstcap==-1) firstcap = digi.sample(0).capid();
  int capdif = digi.sample(0).capid() - firstcap;
  //capdif = capdif%3 - capdif/3; // unnecessary?
  // capdif should run from -3 to +3
  if (capdif >-4 && capdif<4)
    ++h.capIDdiff[capdif+3];
  else
      ++h.capIDdiff[7];

  int last=-1;
  
  int offset = digi.fiberIdleOffset();

  // Only count BCN offset errors if ExpectedOrbitMessage Time is >-1
  // For offline (and thus cfg default), this won't be checked, since
  // we can't keep up to date with changes.
  if (offset != -1000 && DigiMonitor_ExpectedOrbitMessageTime_>-1) 
    {
      // increment counters only for non-zero offsets?
      ++h.fibbcnoff[offset + 7];
      if (offset != 0)
	{
	  ++badFibBCNOff[calcEta][iPhi-1][iDepth-1];
	  if (shutOffOrbitTest_ == false) err |= 0xF; // not an error if test turned off
	}
    }
  
  int tssum=0;

  bool digi_error=false;

  for (int i=0;i<digi.size();++i)
    {
      int thisCapid = digi.sample(i).capid();
      if (thisCapid<4) ++h.capid[thisCapid];

      if (makeDiagnostics_)
	{
	  if(bitUpset(last,thisCapid)) bitUp=true; // checking capID rotation
	  last = thisCapid;
	  // Check for digi error bits
	  if (digi_checkdverr_)
	    {
	      if(digi.sample(i).er()) err=(err|0x2);
	      if(!digi.sample(i).dv()) err=(err|0x2);
	    }
	  if ((digi_error==false) && (digi.sample(i).er() || !digi.sample(i).dv()))
	    {
	      ++digierrorsdverr[calcEta][iPhi-1][iDepth-1];
	      digi_error=true; // only count 1 error per digi in this plot
	    }
	  ++h.dverr[static_cast<int>(2*digi.sample(i).er()+digi.sample(i).dv())];
	}
      ADCcount+=digi.sample(i).adc();
      if (digi.sample(i).adc()<200) ++h.adc[digi.sample(i).adc()];
      h.count_shape[i]+=digi.sample(i).adc();
      
      // Calculate ADC sum of adjacent samples -- still necessary?
      if (i==digi.size()-1) continue;
      tssum= digi.sample(i).adc()+digi.sample(i+1).adc();
      if (tssum<45 && tssum>=-5)
	{
	  if (iEta>0)
	    ++h.tssumplus[tssum+5][i];
	  else
	    ++h.tssumminus[tssum+5][i];
	}
    } // for (int i=0;i<digi.size();++i)
  
  // capid error found
  if(bitUp) 
    {
      if (digi_checkcapid_) err=(err|0x4);
      ++badcapID[calcEta][iPhi-1][iDepth-1];
    }

  // These plots generally don't get filled, unless we turn off the suppression of bad digis
  if (err>0)
    {
      ++h.count_bad;
      ++baddigis[calcEta][iPhi-1][iDepth-1];
      ++errorVME[static_cast<int>(2*(digi.elecId().htrSlot()+0.5*digi.elecId().htrTopBottom()))][static_cast<int>(digi.elecId().readoutVMECrateId())];
      ++errorSpigot[static_cast<int>(digi.elecId().spigot())][static_cast<int>(digi.elecId().dccid())];
      return err;
    }

  // require minimum ADC count for occupancy; this doesn't do anything?
  if (ADCcount<199)
    ++h.adcsum[ADCcount];
  else
    ++h.adcsum[199]; // effective overflow bin

  // require larger threshold to look at pulse shapes

  if (ADCcount>shapeThresh)
    {
      if (digi.id().subdet()!=HcalOuter || isSiPM(iEta,iPhi, iDepth)==false)
	for (int i=0;i<digi.size();++i)
	  h.count_shapeThresh[i]+=digi.sample(i).adc();
    }

  // occupancy plots are only filled for good histograms
  ++h.count_good;
  ++occupancyEtaPhi[calcEta][iPhi-1][iDepth-1];
  ++occupancyEta[iEta+41];
  ++occupancyPhi[iPhi-1];
  // htr Slots run from 0-20, incremented by 0.5 for top/bottom
  ++occupancyVME[static_cast<int>(2*(digi.elecId().htrSlot()+0.5*digi.elecId().htrTopBottom()))][static_cast<int>(digi.elecId().readoutVMECrateId())];
  ++occupancySpigot[static_cast<int>(digi.elecId().spigot())][static_cast<int>(digi.elecId().dccid())];

  return err;
} // template <class DIGI> int HcalDigiMonitor::process_Digi

void HcalDigiMonitor::beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
					     const edm::EventSetup& c) 
{
  HcalBaseDQMonitor::beginLuminosityBlock(lumiSeg,c);
}

void HcalDigiMonitor::endLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
					   const edm::EventSetup& c)
{
  if (LumiInOrder(lumiSeg.luminosityBlock())==false) return;
  fill_Nevents();
  return;
}

void HcalDigiMonitor::fill_Nevents()
{
  if (debug_>0)
    std::cout <<"<HcalDigiMonitor> Calling fill_Nevents for event  "<<tevt_<< " (processed events = "<<ievt_<<")"<<std::endl;
  int iPhi, iEta, iDepth;
  bool valid=false;

  // Fill plots of sums of adjacent digi samples
  for (int i=0;i<10;++i)
    {
      for (int j=0;j<50;++j)
	{
	  if (hbHists.tssumplus[j][i]>0) hbHists.TS_sum_plus[i]->Fill(j, hbHists.tssumplus[j][i]);
	  if (hbHists.tssumminus[j][i]>0) hbHists.TS_sum_minus[i]->Fill(j, hbHists.tssumminus[j][i]);	  
	  if (heHists.tssumplus[j][i]>0) heHists.TS_sum_plus[i]->Fill(j, heHists.tssumplus[j][i]); 
	  if (heHists.tssumminus[j][i]>0) heHists.TS_sum_minus[i]->Fill(j, heHists.tssumminus[j][i]); 
	  if (hoHists.tssumplus[j][i]>0) hoHists.TS_sum_plus[i]->Fill(j, hoHists.tssumplus[j][i]); 
	  if (hoHists.tssumminus[j][i]>0) hoHists.TS_sum_minus[i]->Fill(j, hoHists.tssumminus[j][i]); 
	  if (hfHists.tssumplus[j][i]>0) hfHists.TS_sum_plus[i]->Fill(j, hfHists.tssumplus[j][i]); 
	  if (hfHists.tssumminus[j][i]>0) hfHists.TS_sum_minus[i]->Fill(j, hfHists.tssumminus[j][i]); 
	}
    } // for (int i=0;i<10;++i)

  for (int i=0;i<DIGI_NUM;++i)
    {
      if (diginum[i]>0) DigiNum->Fill(i, diginum[i]);
      if (i>=DIGI_SUBDET_NUM) continue;

      if (hbHists.count_BQ[i]>0) hbHists.BQ->Fill(i, hbHists.count_BQ[i]);
      if (heHists.count_BQ[i]>0) heHists.BQ->Fill(i, heHists.count_BQ[i]);
      if (hoHists.count_BQ[i]>0) hoHists.BQ->Fill(i, hoHists.count_BQ[i]);
      if (hfHists.count_BQ[i]>0) hfHists.BQ->Fill(i, hfHists.count_BQ[i]);
    }//for int i=0;i<DIGI_NUM;++i)


  // Fill data-valid/error plots and capid plots
  for (int i=0;i<4;++i)
    {
      if (hbHists.dverr[i]>0) hbHists.DVerr->Fill(i, hbHists.dverr[i]);
      if (heHists.dverr[i]>0) heHists.DVerr->Fill(i, heHists.dverr[i]);
      if (hoHists.dverr[i]>0) hoHists.DVerr->Fill(i, hoHists.dverr[i]);
      if (hfHists.dverr[i]>0) hfHists.DVerr->Fill(i, hfHists.dverr[i]);

      if (hbHists.capid[i]>0) hbHists.CapID->Fill(i, hbHists.capid[i]);
      if (heHists.capid[i]>0) heHists.CapID->Fill(i, heHists.capid[i]);
      if (hoHists.capid[i]>0) hoHists.CapID->Fill(i, hoHists.capid[i]);
      if (hfHists.capid[i]>0) hfHists.CapID->Fill(i, hfHists.capid[i]);
    }

  for (int i=0;i<200;++i)
    {
      if (hbHists.adc[i]>0) hbHists.ADC->Fill(i, hbHists.adc[i]);
      if (heHists.adc[i]>0) heHists.ADC->Fill(i, heHists.adc[i]);
      if (hoHists.adc[i]>0) hoHists.ADC->Fill(i, hoHists.adc[i]);
      if (hfHists.adc[i]>0) hfHists.ADC->Fill(i, hfHists.adc[i]);

      if (hbHists.adcsum[i]>0) hbHists.ADCsum->Fill(i, hbHists.adcsum[i]);
      if (heHists.adcsum[i]>0) heHists.ADCsum->Fill(i, heHists.adcsum[i]);
      if (hoHists.adcsum[i]>0) hoHists.ADCsum->Fill(i, hoHists.adcsum[i]);
      if (hfHists.adcsum[i]>0) hfHists.ADCsum->Fill(i, hfHists.adcsum[i]);
    }

  for (int i = 0; i < 15; ++i)
    {
      if (hbHists.fibbcnoff[i]>0) hbHists.fibBCNOff->Fill(i-7, 
							  hbHists.fibbcnoff[i]);
      if (heHists.fibbcnoff[i]>0) heHists.fibBCNOff->Fill(i-7, 
							  heHists.fibbcnoff[i]);
      if (hfHists.fibbcnoff[i]>0) hfHists.fibBCNOff->Fill(i-7, 
							  hfHists.fibbcnoff[i]);
      if (hoHists.fibbcnoff[i]>0) hoHists.fibBCNOff->Fill(i-7,
							  hoHists.fibbcnoff[i]);
    }

  // Fill plots of bad fraction of digis found
  for (int i=0;i<DIGI_BQ_FRAC_NBINS;++i)
    {
      if (DIGI_BQ_FRAC_NBINS==1) break;
      if (hbHists.count_BQFrac[i]>0) hbHists.BQFrac->Fill(1.*i/(DIGI_BQ_FRAC_NBINS-1), hbHists.count_BQFrac[i]);
      if (heHists.count_BQFrac[i]>0) heHists.BQFrac->Fill(1.*i/(DIGI_BQ_FRAC_NBINS-1), heHists.count_BQFrac[i]);
      if (hoHists.count_BQFrac[i]>0) 
	{
	  hoHists.BQFrac->Fill(1.*i/(DIGI_BQ_FRAC_NBINS), hoHists.count_BQFrac[i]);
	}
      if (hfHists.count_BQFrac[i]>0) hfHists.BQFrac->Fill(1.*i/(DIGI_BQ_FRAC_NBINS-1), hfHists.count_BQFrac[i]);
    }//for (int i=0;i<DIGI_BQ_FRAC_NBINS;++i)

  // Fill presample plots
  for (int i=0;i<50;++i)
    {
      if (hbHists.count_presample[i]>0) hbHists.presample->Fill(i, hbHists.count_presample[i]);
      if (heHists.count_presample[i]>0) heHists.presample->Fill(i, heHists.count_presample[i]);
      if (hoHists.count_presample[i]>0) hoHists.presample->Fill(i, hoHists.count_presample[i]);
      if (hfHists.count_presample[i]>0) hfHists.presample->Fill(i, hfHists.count_presample[i]);
    } //for (int i=0;i<50;++i)

  // Fill shape plots
  for (int i=0;i<10;++i)
    {
      if (hbHists.count_shape[i]>0) hbHists.shape->Fill(i, hbHists.count_shape[i]);
      if (hbHists.count_shapeThresh[i]>0) hbHists.shapeThresh->Fill(i, hbHists.count_shapeThresh[i]);
      if (heHists.count_shape[i]>0) heHists.shape->Fill(i, heHists.count_shape[i]);
      if (heHists.count_shapeThresh[i]>0) heHists.shapeThresh->Fill(i, heHists.count_shapeThresh[i]);
      if (hoHists.count_shape[i]>0) hoHists.shape->Fill(i, hoHists.count_shape[i]);
      if (hoHists.count_shapeThresh[i]>0) hoHists.shapeThresh->Fill(i, hoHists.count_shapeThresh[i]);
      if (hfHists.count_shape[i]>0) hfHists.shape->Fill(i, hfHists.count_shape[i]);
      if (hfHists.count_shapeThresh[i]>0) hfHists.shapeThresh->Fill(i, hfHists.count_shapeThresh[i]);
    }//  for (int i=0;i<10;++i)

  // Fill capID difference plots
  for (int i=0;i<8;++i)
    {
      if (hbHists.capIDdiff[i]>0) hbHists.DigiFirstCapID->Fill(i, hbHists.capIDdiff[i]);
      if (heHists.capIDdiff[i]>0) heHists.DigiFirstCapID->Fill(i, heHists.capIDdiff[i]);
      if (hoHists.capIDdiff[i]>0) hoHists.DigiFirstCapID->Fill(i, hoHists.capIDdiff[i]);
      if (hfHists.capIDdiff[i]>0) hfHists.DigiFirstCapID->Fill(i, hfHists.capIDdiff[i]);
    }

  // Fill VME plots
  for (int i=0;i<40;++i)
    {
      for (int j=0;j<18;++j)
	{
	  if (errorVME[i][j]>0) DigiErrorVME->Fill(i, j,errorVME[i][j]);
	  if (occupancyVME[i][j]>0) DigiOccupancyVME->Fill(i, j,occupancyVME[i][j]);
	}
    } //for (int i=0;i<40;++i)

  // Fill SPIGOT plots
  for (int i=0;i<HcalDCCHeader::SPIGOT_COUNT;++i)
    {
      for (int j=0;j<36;++j)
	{
	  if (errorSpigot[i][j]>0) DigiErrorSpigot->Fill(i, j,errorSpigot[i][j]);
	  if (occupancySpigot[i][j]>0) DigiOccupancySpigot->Fill(i, j,occupancySpigot[i][j]);
	}
    } //for (int i=0;i<HcalDCCHeader::SPIGOT_COUNT;++i)

  // Loop over subdetectors
  for (int sub=0;sub<4;++sub)
    {
      for (int dsize=0;dsize<20;++dsize)
	{
	  if (digisize[dsize][sub]>0)
	    DigiSize->Fill(sub,dsize,digisize[dsize][sub]);
	}
    } // for (int sub=0;sub<4;++sub)

  // Loop over eta, phi, depth
  for (int d=0;d<4;++d)
    {
      iDepth=d+1;
      DigiErrorsByDepth.depth[d]->setBinContent(0,0,ievt_); // underflow bin contains event counter
      DigiOccupancyByDepth.depth[d]->setBinContent(0,0,ievt_);
      DigiErrorsBadDigiSize.depth[d]->setBinContent(0,0,ievt_);
      DigiErrorsUnpacker.depth[d]->setBinContent(0,0,ievt_);
      DigiErrorsBadFibBCNOff.depth[d]->setBinContent(0,0,ievt_);

      for (int phi=0;phi<72;++phi)
	{
	  iPhi=phi+1;
	  DigiOccupancyPhi->Fill(iPhi,occupancyPhi[phi]);
	  for (int eta=0;eta<83;++eta)
	    {
	      // DigiOccupanyEta uses 'true' ieta (included the overlap at +/- 29)
	      iEta=eta-41;
	      if (phi==0)
		DigiOccupancyEta->Fill(iEta,occupancyEta[eta]);
	      valid=false;
	
	      // HB
	      if (validDetId(HcalBarrel, iEta, iPhi, iDepth))
		{
		  valid=true;
		  if (HBpresent_)
		    {
                      int calcEta = CalcEtaBin(HcalBarrel,iEta,iDepth);

		      DigiOccupancyByDepth.depth[d]->Fill(iEta, iPhi,
						    occupancyEtaPhi[calcEta][phi][d]);
		      
		      if (makeDiagnostics_)
			{
			  DigiErrorsBadCapID.depth[d]->Fill(iEta, iPhi,
							    badcapID[calcEta][phi][d]);
			  DigiErrorsDVErr.depth[d]->Fill(iEta, iPhi,
							 digierrorsdverr[calcEta][phi][d]);
			}
		      DigiErrorsBadDigiSize.depth[d]->Fill(iEta, iPhi,
							   baddigisize[calcEta][phi][d]);
		      DigiErrorsBadFibBCNOff.depth[d]->Fill(iEta, iPhi,
							    badFibBCNOff[calcEta][phi][d]);
		      DigiErrorsUnpacker.depth[d]->Fill(iEta, iPhi,
							badunpackerreport[calcEta][phi][d]);
		      DigiErrorsByDepth.depth[d]->Fill(iEta, iPhi,
						       baddigis[calcEta][phi][d]);
		      // Use this for testing purposes only
		      //DigiErrorsByDepth[d]->Fill(iEta, iPhi, ievt_);
		    } // if (HBpresent_)
		} // validDetId(HB)
	      // HE
	      if (validDetId(HcalEndcap, iEta, iPhi, iDepth))
		{
		  valid=true;
		  if (HEpresent_)
		    {
                      int calcEta = CalcEtaBin(HcalEndcap,iEta,iDepth);

		      DigiOccupancyByDepth.depth[d]->Fill(iEta, iPhi,
						    occupancyEtaPhi[calcEta][phi][d]);
		      
		      if (makeDiagnostics_)
			{
			  DigiErrorsBadCapID.depth[d]->Fill(iEta, iPhi,
							    badcapID[calcEta][phi][d]);
			  DigiErrorsDVErr.depth[d]->Fill(iEta, iPhi,
							 digierrorsdverr[calcEta][phi][d]);
			}
		      DigiErrorsBadDigiSize.depth[d]->Fill(iEta, iPhi,
							   baddigisize[calcEta][phi][d]);
		      DigiErrorsBadFibBCNOff.depth[d]->Fill(iEta, iPhi,
							    badFibBCNOff[calcEta][phi][d]);
		      DigiErrorsUnpacker.depth[d]->Fill(iEta, iPhi,
							badunpackerreport[calcEta][phi][d]);
		      DigiErrorsByDepth.depth[d]->Fill(iEta, iPhi,
						       baddigis[calcEta][phi][d]);
		    } // if (HEpresent_)
		} // valid HE found
	      // HO
	      if (validDetId(HcalOuter,iEta,iPhi,iDepth))
		{
		  valid=true;
		  if (HOpresent_)
		    {
                      int calcEta = CalcEtaBin(HcalOuter,iEta,iDepth);
		      DigiOccupancyByDepth.depth[d]->Fill(iEta, iPhi,
							  occupancyEtaPhi[calcEta][phi][d]);
		      if (makeDiagnostics_)
			{
			  DigiErrorsBadCapID.depth[d]->Fill(iEta, iPhi,
							    badcapID[calcEta][phi][d]);
			  DigiErrorsDVErr.depth[d]->Fill(iEta, iPhi,
							 digierrorsdverr[calcEta][phi][d]);
			}
		      DigiErrorsBadDigiSize.depth[d]->Fill(iEta, iPhi,
							   baddigisize[calcEta][phi][d]);
		      DigiErrorsBadFibBCNOff.depth[d]->Fill(iEta, iPhi,
							    badFibBCNOff[calcEta][phi][d]);
		      DigiErrorsUnpacker.depth[d]->Fill(iEta, iPhi,
							badunpackerreport[calcEta][phi][d]);
		      DigiErrorsByDepth.depth[d]->Fill(iEta, iPhi,
						       baddigis[calcEta][phi][d]);
		    } // if (HOpresent_)
		}//validDetId(HO)
	      // HF
	      if (validDetId(HcalForward,iEta,iPhi,iDepth))
		{
		  valid=true;
		  if (HFpresent_)
		    {
                      int calcEta = CalcEtaBin(HcalForward,iEta,iDepth);
                      int zside = iEta/abs(iEta);
		      DigiOccupancyByDepth.depth[d]->Fill(iEta+zside, iPhi,
						    occupancyEtaPhi[calcEta][phi][d]);
		      
		      if (makeDiagnostics_)
			{
			  DigiErrorsBadCapID.depth[d]->Fill(iEta+zside, iPhi,
							    badcapID[calcEta][phi][d]);
			  DigiErrorsDVErr.depth[d]->Fill(iEta+zside, iPhi,
							 digierrorsdverr[calcEta][phi][d]);
			}
		      DigiErrorsBadDigiSize.depth[d]->Fill(iEta+zside, iPhi,
						     baddigisize[calcEta][phi][d]);
		      DigiErrorsBadFibBCNOff.depth[d]->Fill(iEta+zside, iPhi,
							 badFibBCNOff[calcEta][phi][d]);
		      DigiErrorsUnpacker.depth[d]->Fill(iEta+zside, iPhi,
							badunpackerreport[calcEta][phi][d]);
		      DigiErrorsByDepth.depth[d]->Fill(iEta+zside, iPhi,
						       baddigis[calcEta][phi][d]);
		    } // if (HFpresent_)
		}
	    } // for (int eta=0;...)
	} // for (int phi=0;...)
    } // for (int d=0;...)

  // Now fill all the unphysical cell values
  FillUnphysicalHEHFBins(DigiErrorsByDepth);
  if (makeDiagnostics_)
    {
      FillUnphysicalHEHFBins(DigiErrorsBadCapID);
      FillUnphysicalHEHFBins(DigiErrorsDVErr);
    }
  FillUnphysicalHEHFBins(DigiErrorsBadDigiSize);
  FillUnphysicalHEHFBins(DigiOccupancyByDepth);
  FillUnphysicalHEHFBins(DigiErrorsBadFibBCNOff);
  FillUnphysicalHEHFBins(DigiErrorsUnpacker);

  zeroCounters(); // reset counters of good/bad digis
 
  return;
} // void HcalDigiMonitor::fill_Nevents()


void HcalDigiMonitor::zeroCounters()
{
  // Set all histogram counters back to 0
  // Call this after all every N evnets

  /******** Zero all counters *******/
  
  for (int i=0;i<85;++i)
    {
      occupancyEta[i]=0;
      if (i<72)
        occupancyPhi[i]=0;
      for (int j=0;j<72;++j)
        {
          for (int k=0;k<4;++k)
            {
              baddigis[i][j][k]=0;
              badcapID[i][j][k]=0;
              baddigisize[i][j][k]=0;
              occupancyEtaPhi[i][j][k]=0;
              digierrorsdverr[i][j][k]=0;
	      badFibBCNOff[i][j][k]=0;
	      badunpackerreport[i][j][k]=0;
            }
        } // for (int j=0;j<72;++i)
    } // for (int i=0;i<85;++i)

  for (int i=0;i<40;++i)
    {
      for (int j=0;j<18;++j)
	{
	  occupancyVME[i][j]=0;
	  errorVME[i][j]=0;
	}
    }

  for (int i=0;i<HcalDCCHeader::SPIGOT_COUNT;++i)
    {
      for (int j=0;j<36;++j)
	{
	  occupancySpigot[i][j]=0;
	  errorSpigot[i][j]=0;
	}
    }


  for (int i=0;i<20;++i)
    {
      for (int j=0;j<4;++j)
	digisize[i][j]=0;
    }

  for (int i=0;i<DIGI_NUM;++i)
    {
      diginum[i]=0;
      
      // set all DigiHists counters to 0
      if (i<4)
	{
	  hbHists.dverr[i]=0;
	  heHists.dverr[i]=0;
	  hoHists.dverr[i]=0;
	  hfHists.dverr[i]=0;
	  hbHists.capid[i]=0;
	  heHists.capid[i]=0;
	  hoHists.capid[i]=0;
	  hfHists.capid[i]=0;
	}
      if (i<8)
	{
	  hbHists.capIDdiff[i]=0;
	  heHists.capIDdiff[i]=0;
	  hoHists.capIDdiff[i]=0;
	  hfHists.capIDdiff[i]=0;
	}

      if (i<10)
	{
	  hbHists.count_shape[i]=0;
	  heHists.count_shape[i]=0;
	  hoHists.count_shape[i]=0;
	  hfHists.count_shape[i]=0;
	 
	  hbHists.count_shapeThresh[i]=0;
	  heHists.count_shapeThresh[i]=0;
	  hoHists.count_shapeThresh[i]=0;
	  hfHists.count_shapeThresh[i]=0;
	}
      if (i<50)
	{
	  hbHists.count_presample[i]=0;
	  heHists.count_presample[i]=0;
	  hoHists.count_presample[i]=0;
	  hfHists.count_presample[i]=0;
	  for (int j=0;j<10;++j)
	    {
	      hbHists.tssumplus[i][j]=0;
	      heHists.tssumplus[i][j]=0;
	      hoHists.tssumplus[i][j]=0;
	      hfHists.tssumplus[i][j]=0;
	      hbHists.tssumminus[i][j]=0;
	      heHists.tssumminus[i][j]=0;
	      hoHists.tssumminus[i][j]=0;
	      hfHists.tssumminus[i][j]=0;
	    }
	}
      if (i<200)
	{
	  hbHists.adc[i]=0;
	  heHists.adc[i]=0;
	  hoHists.adc[i]=0;
	  hfHists.adc[i]=0;
	  hbHists.adcsum[i]=0;
	  heHists.adcsum[i]=0;
	  hoHists.adcsum[i]=0;
	  hfHists.adcsum[i]=0;
	}
      if (i<DIGI_SUBDET_NUM)
	{
	  hbHists.count_BQ[i]=0;
	  heHists.count_BQ[i]=0;
	  hoHists.count_BQ[i]=0;
	  hfHists.count_BQ[i]=0;
	}
      if (i<DIGI_BQ_FRAC_NBINS)
	{
	  hbHists.count_BQFrac[i]=0;
	  heHists.count_BQFrac[i]=0;
	  hoHists.count_BQFrac[i]=0;
	  hfHists.count_BQFrac[i]=0;
	}
    } // for (int i=0;i<DIGI_NUM;++i)


  return;
}

void HcalDigiMonitor::UpdateHists(DigiHists& h)
{
  // call update command for all histograms (should make them update when running in online DQM?
  h.shape->update();
  h.shapeThresh->update();
  h.presample->update();
  h.BQ->update();
  h.BQFrac->update();
  h.DigiFirstCapID->update();
  h.DVerr->update();
  h.CapID->update();
  h.ADC->update();
  h.ADCsum->update();
  h.fibBCNOff->update();

  for (unsigned int i=0;i<h.TS_sum_plus.size();++i)
    h.TS_sum_plus[i]->update();
  for (unsigned int i=0;i<h.TS_sum_minus.size();++i)
    h.TS_sum_minus[i]->update();
} //void HcalDigiMonitor::UpdateHists(DigiHists& h)


void HcalDigiMonitor::reset()
{
  // reset the temporary histograms
  zeroCounters();
  
  // then reset the MonitorElements

  DigiErrorsByDepth.Reset();
  DigiErrorsBadCapID.Reset();
  DigiErrorsDVErr.Reset();
  DigiErrorsBadDigiSize.Reset();
  DigiErrorsBadADCSum.Reset();
  DigiErrorsUnpacker.Reset();
  DigiErrorsBadFibBCNOff.Reset();
  DigiOccupancyByDepth.Reset();
  DigiErrorOccupancyByDepth.Reset();

  DigiOccupancyEta->Reset();
  DigiOccupancyPhi->Reset();
  DigiOccupancyVME->Reset();
  DigiOccupancySpigot->Reset();
  DigiErrorVME->Reset();
  DigiErrorSpigot->Reset();
  
  DigiBQ->Reset();
  DigiBQFrac->Reset();
  DigiUnpackerErrorCount->Reset();
  DigiUnpackerErrorFrac->Reset();

  DigiNum->Reset();

  hbHists.shape->Reset();
  hbHists.shapeThresh->Reset();
  hbHists.presample->Reset();
  hbHists.BQ->Reset();
  hbHists.BQFrac->Reset();
  hbHists.DigiFirstCapID->Reset();
  hbHists.DVerr->Reset();
  hbHists.CapID->Reset();
  hbHists.ADC->Reset();
  hbHists.ADCsum->Reset();
  hbHists.fibBCNOff->Reset();
  for (unsigned int i=0;i<hbHists.TS_sum_plus.size();++i)
    hbHists.TS_sum_plus[i]->Reset();
  for (unsigned int i=0;i<hbHists.TS_sum_minus.size();++i)
    hbHists.TS_sum_minus[i]->Reset();

  heHists.shape->Reset();
  heHists.shapeThresh->Reset();
  heHists.presample->Reset();
  heHists.BQ->Reset();
  heHists.BQFrac->Reset();
  heHists.DigiFirstCapID->Reset();
  heHists.DVerr->Reset();
  heHists.CapID->Reset();
  heHists.ADC->Reset();
  heHists.ADCsum->Reset();
  heHists.fibBCNOff->Reset();
  for (unsigned int i=0;i<heHists.TS_sum_plus.size();++i)
    heHists.TS_sum_plus[i]->Reset();
  for (unsigned int i=0;i<heHists.TS_sum_minus.size();++i)
    heHists.TS_sum_minus[i]->Reset();

  hoHists.shape->Reset();
  hoHists.shapeThresh->Reset();
  hoHists.presample->Reset();
  hoHists.BQ->Reset();
  hoHists.BQFrac->Reset();
  hoHists.DigiFirstCapID->Reset();
  hoHists.DVerr->Reset();
  hoHists.CapID->Reset();
  hoHists.ADC->Reset();
  hoHists.ADCsum->Reset();
  hoHists.fibBCNOff->Reset();
  for (unsigned int i=0;i<hoHists.TS_sum_plus.size();++i)
    hoHists.TS_sum_plus[i]->Reset();
  for (unsigned int i=0;i<hoHists.TS_sum_minus.size();++i)
    hoHists.TS_sum_minus[i]->Reset();

  hfHists.shape->Reset();
  hfHists.shapeThresh->Reset();
  hfHists.presample->Reset();
  hfHists.BQ->Reset();
  hfHists.BQFrac->Reset();
  hfHists.DigiFirstCapID->Reset();
  hfHists.DVerr->Reset();
  hfHists.CapID->Reset();
  hfHists.ADC->Reset();
  hfHists.ADCsum->Reset();
  hfHists.fibBCNOff->Reset();
  for (unsigned int i=0;i<hfHists.TS_sum_plus.size();++i)
    hfHists.TS_sum_plus[i]->Reset();
  for (unsigned int i=0;i<hfHists.TS_sum_minus.size();++i)
    hfHists.TS_sum_minus[i]->Reset();

  return;
}
DEFINE_ANOTHER_FWK_MODULE(HcalDigiMonitor);
               
