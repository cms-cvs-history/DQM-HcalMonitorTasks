#include "DQM/HcalMonitorTasks/interface/HcalRawDataMonitor.h"
#include "EventFilter/HcalRawToDigi/interface/HcalDCCHeader.h"

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
  
  dbe_->setCurrentFolder(subdir_);
  
  ProblemCells=dbe_->book2D(" HardwareWatchCells",
			     " Hardware Watch Cells for HCAL",
			     85,-42.5,42.5,
			     72,0.5,72.5); 
  ProblemCells->setAxisTitle("i#eta",1);
  ProblemCells->setAxisTitle("i#phi",2);
  SetEtaPhiLabels(ProblemCells);
  SetupEtaPhiHists(ProblemCellsByDepth," Hardware Watch Cells", "");
      
  //Initialize maps "problemcount" and "problemfound" before first event.
  unsigned int etabins=0;
  unsigned int phibins=0;
  for (unsigned int depth=0; depth<4; ++depth)
    {
      etabins=ProblemCellsByDepth.depth[depth]->getNbinsX();
      phibins=ProblemCellsByDepth.depth[depth]->getNbinsY();
      for (unsigned int eta=0; eta<etabins;++eta)
	{
	  for (unsigned int phi=0;phi<phibins;++phi)
	    {
	      problemcount[eta][phi][depth]=0;
	      problemfound[eta][phi][depth]=false;
	    }
	}
    }

  // * * * Old ways: * * *
  //  meEVT_ = dbe_->bookInt("Raw Data Task Event Number");
  //  meEVT_->Fill(ievt_);    
  //  meTOTALEVT_ = dbe_->bookInt("Raw Data Task Total Events Processed");
  //  meTOTALEVT_->Fill(tevt_);
  //  * * *Go to * * *
  //     dbe_->setCurrentFolder(subdir_);
  //  meIevt_ = dbe_->bookInt("EventsProcessed");
  //  if (meIevt_) meIevt_->Fill(-1);
  //  meLevt_ = dbe_->bookInt("EventsProcessed_currentLS");
  //  if (meLevt_) meLevt_->Fill(-1);
  //  meTevt_ = dbe_->bookInt("EventsProcessed_All");
  //  if (meTevt_) meTevt_->Fill(-1);
  //  meTevtHist_=dbe_->book1D("EventsProcessed_AllHists","Counter of Events Processed By This Task",1,0.5,1.5);
  //  if (meTevtHist_) meTevtHist_->Reset();
  //
  std::string type;
      
  dbe_->setCurrentFolder(subdir_ + "/Corruption"); /// Below, "Corruption" FOLDER
  type = "01 Common Data Format violations";
  meCDFErrorFound_ = dbe_->book2D(type,type,32,699.5,731.5,9,0.5,9.5);
  meCDFErrorFound_->setAxisTitle("HCAL FED ID", 1);
  meCDFErrorFound_->setBinLabel(1, "Hdr1BitUnset", 2);
  meCDFErrorFound_->setBinLabel(2, "FmtNumChange", 2);
  meCDFErrorFound_->setBinLabel(3, "BOE not '0x5'", 2);
  meCDFErrorFound_->setBinLabel(4, "Hdr2Bit Set", 2);
  meCDFErrorFound_->setBinLabel(5, "Hdr1 36-55", 2);
  meCDFErrorFound_->setBinLabel(6, "BOE not 0", 2);
  meCDFErrorFound_->setBinLabel(7, "Trlr1Bit Set", 2);
  meCDFErrorFound_->setBinLabel(8, "Size Error", 2);
  meCDFErrorFound_->setBinLabel(9, "TrailerBad", 2);
      
  type = "02 DCC Event Format violation";
  meDCCEventFormatError_ = dbe_->book2D(type,type,32,699.5,731.5,6,0.5,6.5);
  meDCCEventFormatError_->setAxisTitle("HCAL FED ID", 1);
  meDCCEventFormatError_->setBinLabel(1, "FmtVers Changed", 2);
  meDCCEventFormatError_->setBinLabel(2, "StrayBits Changed", 2);
  meDCCEventFormatError_->setBinLabel(3, "HTRStatusPad", 2);
  meDCCEventFormatError_->setBinLabel(4, "32bitPadErr", 2);
  meDCCEventFormatError_->setBinLabel(5, "Number Mismatch Bit Miscalc", 2);      
  meDCCEventFormatError_->setBinLabel(6, "Low 8 HTR Status Bits Miscopy", 2);	       
      
  type = "04 HTR BCN when OrN Diff";
  meBCNwhenOrNDiff_ = dbe_->book1D(type,type,3564,-0.5,3563.5);
  meBCNwhenOrNDiff_->setAxisTitle("BCN",1);
  meBCNwhenOrNDiff_->setAxisTitle("# of Entries",2);
      
  type = "03 OrN Difference HTR - DCC";
  meOrNCheck_ = dbe_->book1D(type,type,65,-32.5,32.5);
  meOrNCheck_->setAxisTitle("htr OrN - dcc OrN",1);
      
  type = "03 OrN Inconsistent - HTR vs DCC";
  meOrNSynch_= dbe_->book2D(type,type,32,0,32, 15,0,15);
  meOrNSynch_->setAxisTitle("FED #",1);
  meOrNSynch_->setAxisTitle("Spigot #",2);
      
  type = "05 BCN Difference HTR - DCC";
  meBCNCheck_ = dbe_->book1D(type,type,501,-250.5,250.5);
  meBCNCheck_->setAxisTitle("htr BCN - dcc BCN",1);
      
  type = "05 BCN Inconsistent - HTR vs DCC";
  meBCNSynch_= dbe_->book2D(type,type,32,0,32, 15,0,15);
  meBCNSynch_->setAxisTitle("FED #",1);
  meBCNSynch_->setAxisTitle("Slot #",2);
      
  type = "06 EvN Difference HTR - DCC";
  meEvtNCheck_ = dbe_->book1D(type,type,601,-300.5,300.5);
  meEvtNCheck_->setAxisTitle("htr Evt # - dcc Evt #",1);
      
  type = "06 EvN Inconsistent - HTR vs DCC";
  meEvtNumberSynch_= dbe_->book2D(type,type,32,0,32, 15,0,15);
  meEvtNumberSynch_->setAxisTitle("FED #",1);
  meEvtNumberSynch_->setAxisTitle("Slot #",2);
      
  //     ---------------- 
  //     | E!P | UE | TR |                                           
  // ----|  ND | OV | ID |					       
  // | T | CRC | ST | ODD| 					       
  // -------------------- 					       
  type="07 LRB Data Corruption Indicators";  
  meLRBDataCorruptionIndicators_= dbe_->book2D(type,type,
						THREE_FED,0,THREE_FED,
						THREE_SPG,0,THREE_SPG);
  label_xFEDs   (meLRBDataCorruptionIndicators_, 4); // 3 bins + 1 margin per ch.
  label_ySpigots(meLRBDataCorruptionIndicators_, 4); // 3 bins + 1 margin each spgt
      
  //     ---------------- 
  //     | CT | BE |    |
  //     | HM | 15 | WW | (Wrong Wordcount)
  //     | TM | CK | IW | (Illegal Wordcount)
  //     ---------------- 
  type="08 Half-HTR Data Corruption Indicators";
  meHalfHTRDataCorruptionIndicators_= dbe_->book2D(type,type,
						    THREE_FED,0,THREE_FED,
						    THREE_SPG,0,THREE_SPG);
  label_xFEDs   (meHalfHTRDataCorruptionIndicators_, 4); // 3 bins + 1 margin per ch.
  label_ySpigots(meHalfHTRDataCorruptionIndicators_, 4); // 3 bins + 1 margin each spgt
      
  //    ------------
  //    | !DV | Er  |
  //    | NTS | Cap |
  //    ------------
  type = "09 Channel Integrity Summarized by Spigot";
  meChannSumm_DataIntegrityCheck_= dbe_->book2D(type,type,
						 TWO___FED,0,TWO___FED,
						 TWO__SPGT,0,TWO__SPGT);
  label_xFEDs   (meChannSumm_DataIntegrityCheck_, 3); // 2 bins + 1 margin per ch.
  label_ySpigots(meChannSumm_DataIntegrityCheck_, 3); // 2 bins + 1 margin per spgt
      
  dbe_->setCurrentFolder(subdir_ + "/Corruption/Channel Data Integrity");
  char label[10];
  for (int f=0; f<NUMDCCS; f++){      
    sprintf(label, "FED %03d Channel Integrity", f+700);
    meChann_DataIntegrityCheck_[f] =  dbe_->book2D(label,label,
						    TWO_CHANN,0,TWO_CHANN,
						    TWO__SPGT,0,TWO__SPGT);
    label_xChanns (meChann_DataIntegrityCheck_[f], 3); // 2 bins + 1 margin per ch.
    label_ySpigots(meChann_DataIntegrityCheck_[f], 3); // 2 bins + 1 margin per spgt
    ;}
      
  dbe_->setCurrentFolder(subdir_ + "/Data Flow"); ////Below, "Data Flow" FOLDER
  type="DCC Event Counts";
  mefedEntries_ = dbe_->book1D(type,type,32,699.5,731.5);
      
  type = "BCN from DCCs";
  medccBCN_ = dbe_->book1D(type,type,3564,-0.5,3563.5);
  medccBCN_->setAxisTitle("BCN",1);
  medccBCN_->setAxisTitle("# of Entries",2);
      
  type = "BCN from HTRs";
  meBCN_ = dbe_->book1D(type,type,3564,-0.5,3563.5);
  meBCN_->setAxisTitle("BCN",1);
  meBCN_->setAxisTitle("# of Entries",2);
      
  type = "DCC Data Block Size Distribution";
  meFEDRawDataSizes_=dbe_->book1D(type,type,1200,-0.5,12000.5);
  meFEDRawDataSizes_->setAxisTitle("# of bytes",1);
  meFEDRawDataSizes_->setAxisTitle("# of Data Blocks",2);
      
  type = "DCC Data Block Size Profile";
  meEvFragSize_ = dbe_->bookProfile(type,type,32,699.5,731.5,100,-1000.0,12000.0,"");
  type = "DCC Data Block Size Each FED";
  meEvFragSize2_ =  dbe_->book2D(type,type,64,699.5,731.5, 240,0,12000);
      
  //     ------------
  //     | OW | OFW |    "Two Caps HTR; Three Caps FED."
  //     | BZ | BSY |
  //     | EE | RL  |
  // ----------------
  // | CE |            (corrected error, Hamming code)
  // ------
  type = "01 Data Flow Indicators";
  meDataFlowInd_= dbe_->book2D(type,type,
				TWO___FED,0,TWO___FED,
				THREE_SPG,0,THREE_SPG);
  label_xFEDs   (meDataFlowInd_, 3); // 2 bins + 1 margin per ch.
  label_ySpigots(meDataFlowInd_, 4); // 3 bins + 1 margin each spgt
      
  dbe_->setCurrentFolder(subdir_ + "/Diagnostics"); ////Below, "Diagnostics" FOLDER
      
  type = "DCC Firmware Version";
  meDCCVersion_ = dbe_->bookProfile(type,type, 32, 699.5, 731.5, 256, -0.5, 255.5);
  meDCCVersion_ ->setAxisTitle("FED ID", 1);
      
  type = "HTR Status Word HBHE";
  HTR_StatusWd_HBHE =  dbe_->book1D(type,type,16,-0.5,15.5);
  labelHTRBits(HTR_StatusWd_HBHE,1);
      
  type = "HTR Status Word HF";
  HTR_StatusWd_HF =  dbe_->book1D(type,type,16,-0.5,15.5);
  labelHTRBits(HTR_StatusWd_HF,1);
      
  type = "HTR Status Word HO";
  HTR_StatusWd_HO = dbe_->book1D(type,type,16,-0.5,15.5);
  labelHTRBits(HTR_StatusWd_HO,1);
      
  int maxbits = 16;//Look at all 16 bits of the Error Words
  type = "HTR Status Word by Crate";
  meStatusWdCrate_ = dbe_->book2D(type,type,18,-0.5,17.5,maxbits,-0.5,maxbits-0.5);
  meStatusWdCrate_ -> setAxisTitle("Crate #",1);
  labelHTRBits(meStatusWdCrate_,2);
      
  type = "Unpacking - HcalHTRData check failures";
  meInvHTRData_= dbe_->book2D(type,type,16,-0.5,15.5,32,699.5,731.5);
  meInvHTRData_->setAxisTitle("Spigot #",1);
  meInvHTRData_->setAxisTitle("DCC #",2);
      
  type = "HTR Fiber Orbit Message BCN";
  meFibBCN_ = dbe_->book1D(type,type,3564,-0.5,3563.5);
  meFibBCN_->setAxisTitle("BCN of Fib Orb Msg",1);
      
  type = "HTR Status Word - Crate 0";
  meCrate0HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate0HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate0HTRStatus_,2);
      
  type = "HTR Status Word - Crate 1";
  meCrate1HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate1HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate1HTRStatus_,2);
      
  type = "HTR Status Word - Crate 2";
  meCrate2HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate2HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate2HTRStatus_,2);
      
  type = "HTR Status Word - Crate 3";
  meCrate3HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate3HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate3HTRStatus_,2);
      
  type = "HTR Status Word - Crate 4";
  meCrate4HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate4HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate4HTRStatus_,2);

  type = "HTR Status Word - Crate 5";
  meCrate5HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate5HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate5HTRStatus_,2);

  type = "HTR Status Word - Crate 6";
  meCrate6HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate6HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate6HTRStatus_,2);

  type = "HTR Status Word - Crate 7";
  meCrate7HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate7HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate7HTRStatus_,2);

  type = "HTR Status Word - Crate 9";
  meCrate9HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate9HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate9HTRStatus_,2);

  type = "HTR Status Word - Crate 10";
  meCrate10HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate10HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate10HTRStatus_,2);

  type = "HTR Status Word - Crate 11";
  meCrate11HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate11HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate11HTRStatus_,2);

  type = "HTR Status Word - Crate 12";
  meCrate12HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate12HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate12HTRStatus_,2);

  type = "HTR Status Word - Crate 13";
  meCrate13HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate13HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate13HTRStatus_,2);

  type = "HTR Status Word - Crate 14";
  meCrate14HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate14HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate14HTRStatus_,2);

  type = "HTR Status Word - Crate 15";
  meCrate15HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate15HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate15HTRStatus_,2);

  type = "HTR Status Word - Crate 17";
  meCrate17HTRStatus_ = dbe_->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
  meCrate17HTRStatus_ ->setAxisTitle("Slot #",1);
  labelHTRBits(meCrate17HTRStatus_,2);

  type = "HTR UnSuppressed Event Fractions";
  meUSFractSpigs_ = dbe_->book1D(type,type,481,0,481);
  for(int f=0; f<NUMDCCS; f++) {
    sprintf(label, "FED 7%02d", f);
    meUSFractSpigs_->setBinLabel(1+(HcalDCCHeader::SPIGOT_COUNT*f), label);
    for(int s=1; s<HcalDCCHeader::SPIGOT_COUNT; s++) {
      sprintf(label, "sp%02d", s);
      meUSFractSpigs_->setBinLabel(1+(HcalDCCHeader::SPIGOT_COUNT*f)+s, label);}}

  // Firmware version
  type = "HTR Firmware Version";
  //  Maybe change to Profile histo eventually
  //meHTRFWVersion_ = dbe_->bookProfile(type,type,18,-0.5,17.5,245,10.0,255.0,"");
  meHTRFWVersion_ = dbe_->book2D(type,type ,18,-0.5,17.5,180,75.5,255.5);
  meHTRFWVersion_->setAxisTitle("Crate #",1);
  meHTRFWVersion_->setAxisTitle("HTR Firmware Version",2);

  type = "HTR Fiber 1 Orbit Message BCNs";
  meFib1OrbMsgBCN_= dbe_->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);
  type = "HTR Fiber 2 Orbit Message BCNs";
  meFib2OrbMsgBCN_= dbe_->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);
  type = "HTR Fiber 3 Orbit Message BCNs";
  meFib3OrbMsgBCN_= dbe_->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);
  type = "HTR Fiber 4 Orbit Message BCNs";
  meFib4OrbMsgBCN_= dbe_->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);
  type = "HTR Fiber 5 Orbit Message BCNs";
  meFib5OrbMsgBCN_= dbe_->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);
  type = "HTR Fiber 6 Orbit Message BCNs";
  meFib6OrbMsgBCN_= dbe_->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);
  type = "HTR Fiber 7 Orbit Message BCNs";
  meFib7OrbMsgBCN_= dbe_->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);
  type = "HTR Fiber 8 Orbit Message BCNs";
  meFib8OrbMsgBCN_= dbe_->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);

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

//No size checking; better have enough y-axis bins!
void HcalRawDataMonitor::label_ySpigots(MonitorElement* me_ptr, int ybins) {
  char label[10];
  for (int spig=0; spig<HcalDCCHeader::SPIGOT_COUNT; spig++) {
    sprintf(label, "Spgt %02d", spig);
    me_ptr->setBinLabel((2+(spig*ybins)), label, 2); //margin of 1 at low value
  }
}

//No size checking; better have enough x-axis bins!
void HcalRawDataMonitor::label_xChanns(MonitorElement* me_ptr, int xbins) {
  char label[10];
  for (int ch=0; ch<HcalHTRData::CHANNELS_PER_SPIGOT; ch++) {
    sprintf(label, "Ch %02d", ch+1);
    me_ptr->setBinLabel((2+(ch*xbins)), label, 1); //margin of 3 at low value
  }
}

//No size checking; better have enough x-axis bins!
void HcalRawDataMonitor::label_xFEDs(MonitorElement* me_ptr, int xbins) {
  char label[10];
  for (int thfed=0; thfed<NUMDCCS; thfed++) {
    sprintf(label, "%03d", thfed+700);
    me_ptr->setBinLabel((2+(thfed*xbins)), label, 1); //margin of 1 at low value
  }
}

void HcalRawDataMonitor::labelHTRBits(MonitorElement* mePlot,unsigned int axisType) {

  if (axisType !=1 && axisType != 2) return;

  mePlot -> setBinLabel(1,"Overflow Warn",axisType);
  mePlot -> setBinLabel(2,"Buffer Busy",axisType);
  mePlot -> setBinLabel(3,"Empty Event",axisType);
  mePlot -> setBinLabel(4,"Rejected L1A",axisType);
  mePlot -> setBinLabel(5,"Invalid Stream",axisType);
  mePlot -> setBinLabel(6,"Latency Warn",axisType);
  mePlot -> setBinLabel(7,"OptDat Err",axisType);
  mePlot -> setBinLabel(8,"Clock Err",axisType);
  mePlot -> setBinLabel(9,"Bunch Err",axisType);
  mePlot -> setBinLabel(10,"b9",axisType);
  mePlot -> setBinLabel(11,"b10",axisType);
  mePlot -> setBinLabel(12,"b11",axisType);
  mePlot -> setBinLabel(13,"Test Mode",axisType);
  mePlot -> setBinLabel(14,"Histo Mode",axisType);
  mePlot -> setBinLabel(15,"Calib Trig",axisType);
  mePlot -> setBinLabel(16,"Bit15 Err",axisType);

  return;
}

DEFINE_ANOTHER_FWK_MODULE(HcalRawDataMonitor);

