#include "DQM/HcalMonitorTasks/interface/HcalDeadCellMonitor.h"

using namespace std;
using namespace edm;

HcalDeadCellMonitor::HcalDeadCellMonitor(const edm::ParameterSet& ps)
{
  Online_                = ps.getUntrackedParameter<bool>("online",false);
  mergeRuns_             = ps.getUntrackedParameter<bool>("mergeRuns",false);
  enableCleanup_         = ps.getUntrackedParameter<bool>("enableCleanup",false);
  debug_                 = ps.getUntrackedParameter<int>("debug",0);
  makeDiagnostics_       = ps.getUntrackedParameter<bool>("makeDiagnostics",false);
  prefixME_              = ps.getUntrackedParameter<string>("subSystemFolder","Hcal/");
  if (prefixME_.substr(prefixME_.size()-1,prefixME_.size())!="/")
    prefixME_.append("/");
  subdir_                = ps.getUntrackedParameter<string>("TaskFolder","DeadCellMonitor_Hcal"); // DeadCellMonitor_Hcal
  if (subdir_.size()>0 && subdir_.substr(subdir_.size()-1,subdir_.size())!="/")
    subdir_.append("/");
  subdir_=prefixME_+subdir_;
  AllowedCalibTypes_     = ps.getUntrackedParameter<vector<int> > ("AllowedCalibTypes");
  skipOutOfOrderLS_      = ps.getUntrackedParameter<bool>("skipOutOfOrderLS",true);
  NLumiBlocks_           = ps.getUntrackedParameter<int>("NLumiBlocks",4000);

  // DeadCell-specific parameters

  // Collection type info
  digiLabel_             =ps.getUntrackedParameter<edm::InputTag>("digiLabel");
  hbheRechitLabel_       = ps.getUntrackedParameter<edm::InputTag>("hbheRechitLabel");
  hoRechitLabel_         = ps.getUntrackedParameter<edm::InputTag>("hoRechitLabel");
  hfRechitLabel_         = ps.getUntrackedParameter<edm::InputTag>("hfRechitLabel");

  // minimum number of events required for lumi section-based dead cell checks
  minDeadEventCount_    = ps.getUntrackedParameter<int>("minDeadEventCount",1000);

  // Set which dead cell checks will be performed
  /* Dead cells can be defined in the following ways:
     1)  never present digi -- digi is never present in run
     2)  digis -- digi is absent for one or more lumi section 
     3)  never present rechit -- rechit never present (a bit redundant, though cells excluded by Channel Status could be present in digis but not rechits)
     4)  rechits -- digi is present, but rechit energy below threshold for one or more lumi sections

     Of these tests, never-present digis are always checked.
     Occasional digis are checked only if deadmon_test_digis_ is true,
     and both rechit tests are made only if deadmon_test_rechits_ is true
  */
  
  deadmon_test_digis_              = ps.getUntrackedParameter<bool>("test_digis",true);
  deadmon_test_rechits_            = ps.getUntrackedParameter<bool>("test_rechits",false);

  // rechit energy test -- cell must be below threshold value for a number of consecutive events to be considered dead
  energyThreshold_       = ps.getUntrackedParameter<double>("MissingRechitEnergyThreshold",0);
  HBenergyThreshold_     = ps.getUntrackedParameter<double>("HB_energyThreshold",energyThreshold_);
  HEenergyThreshold_     = ps.getUntrackedParameter<double>("HE_energyThreshold",energyThreshold_);
  HOenergyThreshold_     = ps.getUntrackedParameter<double>("HO_energyThreshold",energyThreshold_);
  HFenergyThreshold_     = ps.getUntrackedParameter<double>("HF_energyThreshold",energyThreshold_);

} //constructor

HcalDeadCellMonitor::~HcalDeadCellMonitor()
{
} //destructor


/* ------------------------------------ */ 

void HcalDeadCellMonitor::setup()
{
  HcalBaseDQMonitor::setup();

  if (debug_>0)
    std::cout <<"<HcalDeadCellMonitor::setup>  Setting up histograms"<<std::endl;

  if (!dbe_) return;

  dbe_->setCurrentFolder(subdir_);
  
  Nevents = dbe_->book1D("NumberOfDeadCellEvents","# of Events Seen by DeadCellMonitor",2,0,2);
  Nevents->setBinLabel(1,"allEvents");
  Nevents->setBinLabel(2,"lumiCheck");
 // 1D plots count number of bad cells vs. luminosity block
  ProblemsVsLB=dbe_->bookProfile("TotalDeadCells_HCAL_vs_LS",
				  "Total Number of Dead Hcal Cells vs lumi section;Lumi Section;Dead Cells", 
				  NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				  100,0,10000);
  ProblemsVsLB_HB=dbe_->bookProfile("TotalDeadCells_HB_vs_LS",
				     "Total Number of Dead HB Cells vs lumi section;Lumi Section;Dead Cells",
				     NLumiBlocks_,0.5,NLumiBlocks_+0.5,
				     100,0,10000);
  ProblemsVsLB_HE=dbe_->bookProfile("TotalDeadCells_HE_vs_LS",
				     "Total Number of Dead HE Cells vs lumi section;Lumi Section;Dead Cells",
				     NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
  ProblemsVsLB_HO=dbe_->bookProfile("TotalDeadCells_HO_vs_LS",
				     "Total Number of Dead HO Cells vs lumi section;Lumi Section;Dead Cells",
				     NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
  ProblemsVsLB_HF=dbe_->bookProfile("TotalDeadCells_HF_vs_LS",
				     "Total Number of Dead HF Cells vs lumi section;Lumi Section;Dead Cells",
				     NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
  (ProblemsVsLB->getTProfile())->SetMarkerStyle(20);
  (ProblemsVsLB_HB->getTProfile())->SetMarkerStyle(20);
  (ProblemsVsLB_HE->getTProfile())->SetMarkerStyle(20);
  (ProblemsVsLB_HO->getTProfile())->SetMarkerStyle(20);
  (ProblemsVsLB_HF->getTProfile())->SetMarkerStyle(20);

  dbe_->setCurrentFolder(subdir_+"dead_cell_parameters");
  MonitorElement* me=dbe_->bookInt("Test_NeverPresent_Digis");
  me->Fill(1);
  me=dbe_->bookInt("Test_DigiMissing_Periodic_Lumi_Check");
  if (deadmon_test_digis_)
    me->Fill(1);
  else 
    me->Fill(0);
  me=dbe_->bookInt("Min_Events_Required_Periodic_Lumi_Check");
  me->Fill(minDeadEventCount_);
  me=dbe_->bookInt("Test_NeverPresent_RecHits");
  deadmon_test_rechits_>0 ? me->Fill(1) : me->Fill(0);
  me=dbe_->bookFloat("HBMinimumRecHitEnergy");
  me->Fill(HBenergyThreshold_);
  me=dbe_->bookFloat("HEMinimumRecHitEnergy");
  me->Fill(HEenergyThreshold_);
  me=dbe_->bookFloat("HOMinimumRecHitEnergy");
  me->Fill(HOenergyThreshold_);
  me=dbe_->bookFloat("HFMinimumRecHitEnergy");
  me->Fill(HFenergyThreshold_);
  me=dbe_->bookInt("Test_RecHitsMissing_Periodic_Lumi_Check");
  deadmon_test_rechits_>0 ? me->Fill(1) : me->Fill(0);

  // ProblemCells plots are in HcalDeadCellClient!
      
  // Set up plots for each failure mode of dead cells
  stringstream units; // We'll need to set the titles individually, rather than passing units to SetupEtaPhiHists (since this also would affect the name of the histograms)
  stringstream name;

  // Never-present test will always be called, by definition of dead cell

  dbe_->setCurrentFolder(subdir_+"dead_digi_never_present");
  SetupEtaPhiHists(DigiPresentByDepth,
		   "Digi Present At Least Once","");
  // 1D plots count number of bad cells
  NumberOfNeverPresentDigis=dbe_->bookProfile("Problem_NeverPresentDigis_HCAL_vs_LS",
					       "Total Number of Never-Present Hcal Cells vs LS;Lumi Section;Dead Cells",
					       NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      
  NumberOfNeverPresentDigisHB=dbe_->bookProfile("Problem_NeverPresentDigis_HB_vs_LS",
						 "Total Number of Never-Present HB Cells vs LS;Lumi Section;Dead Cells",
						 NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      
  NumberOfNeverPresentDigisHE=dbe_->bookProfile("Problem_NeverPresentDigis_HE_vs_LS",
						 "Total Number of Never-Present HE Cells vs LS;Lumi Section;Dead Cells",
						 NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      
  NumberOfNeverPresentDigisHO=dbe_->bookProfile("Problem_NeverPresentDigis_HO_vs_LS",
						 "Total Number of Never-Present HO Cells vs LS;Lumi Section;Dead Cells",
						 NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      
  NumberOfNeverPresentDigisHF=dbe_->bookProfile("Problem_NeverPresentDigis_HF_vs_LS",
						 "Total Number of Never-Present HF Cells vs LS;Lumi Section;Dead Cells",
						 NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
  (NumberOfNeverPresentDigis->getTProfile())->SetMarkerStyle(20);
  (NumberOfNeverPresentDigisHB->getTProfile())->SetMarkerStyle(20);
  (NumberOfNeverPresentDigisHE->getTProfile())->SetMarkerStyle(20);
  (NumberOfNeverPresentDigisHO->getTProfile())->SetMarkerStyle(20);
  (NumberOfNeverPresentDigisHF->getTProfile())->SetMarkerStyle(20);


  for (unsigned int depth=0;depth<DigiPresentByDepth.depth.size();++depth)
    DigiPresentByDepth.depth[depth]->Reset();
      
  FillUnphysicalHEHFBins(DigiPresentByDepth);

  if (deadmon_test_digis_)
    {
      dbe_->setCurrentFolder(subdir_+"dead_digi_often_missing");
      //units<<"("<<deadmon_checkNevents_<<" consec. events)";
      name<<"Dead Cells with No Digis";
      SetupEtaPhiHists(RecentMissingDigisByDepth,
		       name.str(),
		       "");
      name.str("");
      name<<"HB HE HF Depth 1 Dead Cells with No Digis for at least 1 Full Luminosity Block"; 
      RecentMissingDigisByDepth.depth[0]->setTitle(name.str().c_str());

      name.str("");
      name<<"HB HE HF Depth 2 Dead Cells with No Digis for at least 1 Full Luminosity Block";
      RecentMissingDigisByDepth.depth[1]->setTitle(name.str().c_str());

      name.str("");
      name<<"HE Depth 3 Dead Cells with No Digis for at least 1 Full Luminosity Block";
      RecentMissingDigisByDepth.depth[2]->setTitle(name.str().c_str());

      name.str("");
      name<<"HO Depth 4 Dead Cells with No Digis for at least 1 Full Luminosity Block";
      RecentMissingDigisByDepth.depth[3]->setTitle(name.str().c_str());
      name.str("");

      // 1D plots count number of bad cells
      name<<"Total Number of Hcal Digis Unoccupied for at least 1 Full Luminosity Block"; 
      NumberOfRecentMissingDigis=dbe_->bookProfile("Problem_RecentMissingDigis_HCAL_vs_LS",
						    name.str(),
						    NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HB Digis Unoccupied for at least 1 Full LS vs LS;Lumi Section; Dead Cells";
      NumberOfRecentMissingDigisHB=dbe_->bookProfile("Problem_RecentMissingDigis_HB_vs_LS",
						      name.str(),
						      NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HE Digis Unoccupied for at least 1 Full LS vs LS;Lumi Section; Dead Cells";
      NumberOfRecentMissingDigisHE=dbe_->bookProfile("Problem_RecentMissingDigis_HE_vs_LS",
						      name.str(),
						      NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HO Digis Unoccupied for at least 1 Full LS vs LS;Lumi Section; Dead Cells";
      NumberOfRecentMissingDigisHO=dbe_->bookProfile("Problem_RecentMissingDigis_HO_vs_LS",
						      name.str(),
						      NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HF Digis Unoccupied for at least 1 Full LS vs LS;Lumi Section; Dead Cells";
      NumberOfRecentMissingDigisHF=dbe_->bookProfile("Problem_RecentMissingDigis_HF_vs_LS",
						      name.str(),
						      NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      (NumberOfRecentMissingDigis->getTProfile())->SetMarkerStyle(20);
      (NumberOfRecentMissingDigisHB->getTProfile())->SetMarkerStyle(20);
      (NumberOfRecentMissingDigisHE->getTProfile())->SetMarkerStyle(20);
      (NumberOfRecentMissingDigisHO->getTProfile())->SetMarkerStyle(20);
      (NumberOfRecentMissingDigisHF->getTProfile())->SetMarkerStyle(20);

    }
      
  if (deadmon_test_rechits_)
    {
      // test 1:  energy never above threshold
      dbe_->setCurrentFolder(subdir_+"dead_rechit_neverpresent");
      SetupEtaPhiHists(RecHitPresentByDepth,"RecHit Above Threshold At Least Once","");
      // set more descriptive titles for threshold plots
      units.str("");
      units<<"Cells Above Energy Threshold At Least Once: Depth 1 -- HB >="<<HBenergyThreshold_<<" GeV, HE >= "<<HEenergyThreshold_<<", HF >="<<HFenergyThreshold_<<" GeV";
      RecHitPresentByDepth.depth[0]->setTitle(units.str().c_str());
      units.str("");
      units<<"Cells Above Energy Threshold At Least Once: Depth 2 -- HB >="<<HBenergyThreshold_<<" GeV, HE >= "<<HEenergyThreshold_<<", HF >="<<HFenergyThreshold_<<" GeV";
      RecHitPresentByDepth.depth[1]->setTitle(units.str().c_str());
      units.str("");
      units<<"Cells Above Energy Threshold At Least Once: Depth 3 -- HE >="<<HEenergyThreshold_<<" GeV";
      RecHitPresentByDepth.depth[2]->setTitle(units.str().c_str());
      units.str("");
      units<<"Cells Above Energy Threshold At Least Once: Depth 4 -- HO >="<<HOenergyThreshold_<<" GeV";
      RecHitPresentByDepth.depth[3]->setTitle(units.str().c_str());
      units.str("");

      // 1D plots count number of bad cells
      NumberOfNeverPresentRecHits=dbe_->bookProfile("Problem_RecHitsNeverPresent_HCAL_vs_LS",
						     "Total Number of Hcal Rechits with Low Energy;Lumi Section;Dead Cells",
						     NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HB RecHits with Energy Never >= "<<HBenergyThreshold_<<" GeV;Lumi Section;Dead Cells";
      NumberOfNeverPresentRecHitsHB=dbe_->bookProfile("Problem_RecHitsNeverPresent_HB_vs_LS",
						       name.str(),
						       NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HE RecHits with Energy Never >= "<<HEenergyThreshold_<<" GeV;Lumi Section;Dead Cells";
      NumberOfNeverPresentRecHitsHE=dbe_->bookProfile("Problem_RecHitsNeverPresent_HE_vs_LS",
						       name.str(),
						       NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HO RecHits with Energy Never >= "<<HOenergyThreshold_<<" GeV;Lumi Section;Dead Cells";
      NumberOfNeverPresentRecHitsHO=dbe_->bookProfile("Problem_RecHitsNeverPresent_HO_vs_LS",
						       name.str(),
						       NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HF RecHits with Energy Never >= "<<HFenergyThreshold_<<" GeV;Lumi Section;Dead Cells";
      NumberOfNeverPresentRecHitsHF=dbe_->bookProfile("Problem_RecHitsNeverPresent_HF_vs_LS",
						       name.str(),
						       NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      (NumberOfNeverPresentRecHits->getTProfile())->SetMarkerStyle(20);
      (NumberOfNeverPresentRecHitsHB->getTProfile())->SetMarkerStyle(20);
      (NumberOfNeverPresentRecHitsHE->getTProfile())->SetMarkerStyle(20);
      (NumberOfNeverPresentRecHitsHO->getTProfile())->SetMarkerStyle(20);
      (NumberOfNeverPresentRecHitsHF->getTProfile())->SetMarkerStyle(20);
 
      dbe_->setCurrentFolder(subdir_+"dead_rechit_often_missing");
      SetupEtaPhiHists(RecentMissingRecHitsByDepth,"RecHits Failing Energy Threshold Test","");
      // set more descriptive titles for threshold plots
      units.str("");
      units<<"RecHits with Consistent Low Energy Depth 1 -- HB <"<<HBenergyThreshold_<<" GeV, HE < "<<HEenergyThreshold_<<", HF <"<<HFenergyThreshold_<<" GeV";
      RecentMissingRecHitsByDepth.depth[0]->setTitle(units.str().c_str());
      units.str("");
      units<<"RecHits with Consistent Low Energy Depth 2 -- HB <"<<HBenergyThreshold_<<" GeV, HE < "<<HEenergyThreshold_<<", HF <"<<HFenergyThreshold_<<" GeV";
      RecentMissingRecHitsByDepth.depth[1]->setTitle(units.str().c_str());
      units.str("");
      units<<"RecHits with Consistent Low Energy Depth 3 -- HE <"<<HEenergyThreshold_<<" GeV";
      RecentMissingRecHitsByDepth.depth[2]->setTitle(units.str().c_str());
      units.str("");
      units<<"RecHits with Consistent Low Energy Depth 4 -- HO <"<<HOenergyThreshold_<<" GeV";
      RecentMissingRecHitsByDepth.depth[3]->setTitle(units.str().c_str());
      units.str("");


      // 1D plots count number of bad cells
      name<<"Total Number of Hcal RecHits with Consistent Low Energy;Lumi Section;Dead Cells";
      NumberOfRecentMissingRecHits=dbe_->bookProfile("Problem_BelowEnergyRecHits_HCAL_vs_LS",
						      name.str(),
						      NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HB RecHits with Consistent Low Energy < "<<HBenergyThreshold_<<" GeV;Lumi Section;Dead Cells";
      NumberOfRecentMissingRecHitsHB=dbe_->bookProfile("Problem_BelowEnergyRecHits_HB_vs_LS",
							name.str(),
							NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HE RecHits with Consistent Low Energy < "<<HEenergyThreshold_<<" GeV;Lumi Section;Dead Cells";
      NumberOfRecentMissingRecHitsHE=dbe_->bookProfile("Problem_BelowEnergyRecHits_HE_vs_LS",
							name.str(),
							NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HO RecHits with Consistent Low Energy < "<<HOenergyThreshold_<<" GeV;Lumi Section;Dead Cells";
      NumberOfRecentMissingRecHitsHO=dbe_->bookProfile("Problem_BelowEnergyRecHits_HO_vs_LS",
							name.str(),
							NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      name.str("");
      name<<"Total Number of HF RecHits with Consistent Low Energy < "<<HFenergyThreshold_<<" GeV;Lumi Section;Dead Cells";
      NumberOfRecentMissingRecHitsHF=dbe_->bookProfile("Problem_BelowEnergyRecHits_HF_vs_LS",
							name.str(),
							NLumiBlocks_,0.5,NLumiBlocks_+0.5,100,0,10000);
      (NumberOfRecentMissingRecHits->getTProfile())->SetMarkerStyle(20);
      (NumberOfRecentMissingRecHitsHB->getTProfile())->SetMarkerStyle(20);
      (NumberOfRecentMissingRecHitsHE->getTProfile())->SetMarkerStyle(20);
      (NumberOfRecentMissingRecHitsHO->getTProfile())->SetMarkerStyle(20);
      (NumberOfRecentMissingRecHitsHF->getTProfile())->SetMarkerStyle(20);

    } // if (deadmon_test_rechits)


  if (makeDiagnostics_)
    {
      dbe_->setCurrentFolder(subdir_+"DiagnosticPlots");
      HBDeadVsEvent=dbe_->book1D("HBDeadVsEvent","HB Total Dead Cells Vs Event", NLumiBlocks_,-0.5,NLumiBlocks_-0.5);
      HEDeadVsEvent=dbe_->book1D("HEDeadVsEvent","HE Total Dead Cells Vs Event", NLumiBlocks_,-0.5,NLumiBlocks_-0.5);
      HODeadVsEvent=dbe_->book1D("HODeadVsEvent","HO Total Dead Cells Vs Event", NLumiBlocks_,-0.5,NLumiBlocks_-0.5);
      HFDeadVsEvent=dbe_->book1D("HFDeadVsEvent","HF Total Dead Cells Vs Event", NLumiBlocks_,-0.5,NLumiBlocks_-0.5);
    }

  this->reset();

  return;

} // void HcalDeadCellMonitor::setup(...)

void HcalDeadCellMonitor::beginRun(const edm::Run& run, const edm::EventSetup& c)
{
  if (debug_>1) std::cout <<"HcalDeadCellMonitor::beginRun"<<std::endl;
  HcalBaseDQMonitor::beginRun(run,c);

  if (tevt_==0) this->setup(); // set up histograms if they have not been created before
  if (mergeRuns_==false)
    this->reset();

  return;
} //void HcalDeadCellMonitor::beginRun(...)

void HcalDeadCellMonitor::reset()
{
  if (debug_>1) std::cout <<"HcalDeadCellMonitor::reset()"<<std::endl;
  HcalBaseDQMonitor::reset();
  zeroCounters();
  deadevt_=0;
  ProblemsVsLB->Reset(); ProblemsVsLB_HB->Reset(); ProblemsVsLB_HE->Reset(); ProblemsVsLB_HO->Reset(); ProblemsVsLB_HF->Reset();
  NumberOfNeverPresentDigis->Reset(); NumberOfNeverPresentDigisHB->Reset(); NumberOfNeverPresentDigisHE->Reset(); NumberOfNeverPresentDigisHO->Reset(); NumberOfNeverPresentDigisHF->Reset();

  if (deadmon_test_digis_)
    {
      NumberOfRecentMissingDigis->Reset(); NumberOfRecentMissingDigisHB->Reset(); NumberOfRecentMissingDigisHE->Reset(); NumberOfRecentMissingDigisHO->Reset(); NumberOfRecentMissingDigisHF->Reset();
      RecentMissingDigisByDepth.Reset();
    }
  if (deadmon_test_rechits_)
    {
      NumberOfRecentMissingRecHits->Reset();
      NumberOfRecentMissingRecHitsHB->Reset();
      NumberOfRecentMissingRecHitsHE->Reset();
      NumberOfRecentMissingRecHitsHO->Reset(); 
      NumberOfRecentMissingRecHitsHF->Reset();
      NumberOfNeverPresentRecHits->Reset(); NumberOfNeverPresentRecHitsHB->Reset(); NumberOfNeverPresentRecHitsHE->Reset(); NumberOfNeverPresentRecHitsHO->Reset(); NumberOfNeverPresentRecHitsHF->Reset();
      RecentMissingRecHitsByDepth.Reset();
      RecHitPresentByDepth.Reset();
    }

  Nevents->Reset();
  DigiPresentByDepth.Reset();
}  // reset function is empty for now

/* ------------------------------------------------------------------------- */


void HcalDeadCellMonitor::cleanup()
{
  if (!enableCleanup_) return;
  if (dbe_)
    {
      dbe_->setCurrentFolder(subdir_);
      dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"dead_digi_never_present");
      dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"dead_digi_often_missing");
      dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"dead_rechit_neverpresent");
      dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"dead_rechit_often_missing");
      dbe_->removeContents();
      dbe_->setCurrentFolder(subdir_+"dead_cell_parameters");
      dbe_->removeContents();
    }
  return;
} // void HcalDeadCellMonitor::cleanup()

/* ------------------------------------------------------------------------- */

void HcalDeadCellMonitor::endLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
					     const edm::EventSetup& c)
{
  // skip old lumi sections
  if (this->LumiInOrder(lumiSeg.luminosityBlock())==false) return;

  // Here is where we determine whether or not to process an event
  // Not enough events
  if (deadevt_<minDeadEventCount_)
      return;

  // fillNevents_problemCells checks for never-present cells
  fillNevents_problemCells();
  fillNevents_recentdigis();
  fillNevents_recentrechits();

  zeroCounters();
  deadevt_=0;
  return;
} //endLuminosityBlock()

void HcalDeadCellMonitor::endRun(const edm::Run& run, const edm::EventSetup& c)
{
  // Always carry out overall occupancy test at endRun, regardless minimum number of events?  
  // Or should we require an absolute lower bound?
  fillNevents_problemCells(); // always check for never-present cells
  return;
}

void HcalDeadCellMonitor::endJob()
{
  if (debug_>0) std::cout <<"HcalDeadCellMonitor::endJob()"<<std::endl;
  if (enableCleanup_) cleanup(); // when do we force cleanup?
}

void HcalDeadCellMonitor::analyze(edm::Event const&e, edm::EventSetup const&s)
{

  if (!IsAllowedCalibType()) return;
  if (LumiInOrder(e.luminosityBlock())==false) return;
  // try to get rechits and digis
  edm::Handle<HBHEDigiCollection> hbhe_digi;
  edm::Handle<HODigiCollection> ho_digi;
  edm::Handle<HFDigiCollection> hf_digi;

  edm::Handle<HBHERecHitCollection> hbhe_rechit;
  edm::Handle<HORecHitCollection> ho_rechit;
  edm::Handle<HFRecHitCollection> hf_rechit;

  if (!(e.getByLabel(digiLabel_,hbhe_digi)))
    {
      LogWarning("HcalDeadCellMonitor")<< digiLabel_<<" hbhe_digi not available";
      return;
    }
  if (!(e.getByLabel(digiLabel_,ho_digi)))
    {
      LogWarning("HcalDeadCellMonitor")<< digiLabel_<<" ho_digi not available";
      return;
    }
  if (!(e.getByLabel(digiLabel_,hf_digi)))
    {
      LogWarning("HcalDeadCellMonitor")<< digiLabel_<<" hf_digi not available";
      return;
    }

  if (!(e.getByLabel(hbheRechitLabel_,hbhe_rechit)))
    {
      LogWarning("HcalDeadCellMonitor")<< hbheRechitLabel_<<" hbhe_rechit not available";
      return;
    }

  if (!(e.getByLabel(hfRechitLabel_,hf_rechit)))
    {
      LogWarning("HcalDeadCellMonitor")<< hfRechitLabel_<<" hf_rechit not available";
      return;
    }
  if (!(e.getByLabel(hoRechitLabel_,ho_rechit)))
    {
      LogWarning("HcalDeadCellMonitor")<< hoRechitLabel_<<" ho_rechit not available";
      return;
    }
  if (debug_>1) std::cout <<"\t<HcalDeadCellMonitor::analyze>  Processing good event! event # = "<<ievt_<<endl;
  // Good event found; increment counter (via base class analyze method)
  // This also runs the allowed calibration /lumi in order tests again;  remove?
  HcalBaseDQMonitor::analyze(e,s);
  
  ++deadevt_; //increment local counter

  processEvent(*hbhe_rechit, *ho_rechit, *hf_rechit, *hbhe_digi, *ho_digi, *hf_digi);

} // void HcalDeadCellMonitor::analyze(...)

/* --------------------------------------- */


void HcalDeadCellMonitor::processEvent(const HBHERecHitCollection& hbHits,
				       const HORecHitCollection&   hoHits,
				       const HFRecHitCollection&   hfHits,
				       const HBHEDigiCollection&   hbhedigi,
				       const HODigiCollection&     hodigi,
				       const HFDigiCollection&     hfdigi)
{
  if (debug_>1) std::cout <<"<HcalDeadCellMonitor::processEvent> Processing event..."<<std::endl;

  // Do Digi-Based dead cell searches 

  // Dummy fills needed for client normalization of problems
  // (though not necessarily here; we could do this in endluminosityblock)
  for (unsigned int i=0;i<DigiPresentByDepth.depth.size();++i)
    DigiPresentByDepth.depth[i]->setBinContent(0,0,ievt_); 
    
  NumberOfNeverPresentDigis->update();;
  NumberOfNeverPresentDigisHB->update();
  NumberOfNeverPresentDigisHE->update();
  NumberOfNeverPresentDigisHO->update();
  NumberOfNeverPresentDigisHF->update();

  if (deadmon_test_digis_)
    {
      
      for (unsigned int i=0;i<RecentMissingDigisByDepth.depth.size();++i)
	RecentMissingDigisByDepth.depth[i]->setBinContent(0,0,ievt_);
      
      NumberOfRecentMissingDigis->update();
      NumberOfRecentMissingDigisHB->update();
      NumberOfRecentMissingDigisHE->update();
      NumberOfRecentMissingDigisHO->update();
      NumberOfRecentMissingDigisHF->update();
    }
  
  for (HBHEDigiCollection::const_iterator j=hbhedigi.begin();
       j!=hbhedigi.end(); ++j)
    {
      const HBHEDataFrame digi = (const HBHEDataFrame)(*j);
      processEvent_HBHEdigi(digi);
    }
    
  for (HODigiCollection::const_iterator j=hodigi.begin();
       j!=hodigi.end(); ++j)
    {
      const HODataFrame digi = (const HODataFrame)(*j);
      process_Digi(digi);
    }
  for (HFDigiCollection::const_iterator j=hfdigi.begin();
       j!=hfdigi.end(); ++j)
    {
      const HFDataFrame digi = (const HFDataFrame)(*j);	 
      process_Digi(digi);
    }
  FillUnphysicalHEHFBins(DigiPresentByDepth);
  
  // Search for "dead" cells below a certain energy
  if (deadmon_test_rechits_) 
    {
      // Normalization Fill
      for (unsigned int i=0;i<RecentMissingRecHitsByDepth.depth.size();++i)
	RecentMissingRecHitsByDepth.depth[i]->setBinContent(0,0,ievt_);

      NumberOfRecentMissingRecHits->update();
      NumberOfRecentMissingRecHitsHB->update();
      NumberOfRecentMissingRecHitsHE->update();
      NumberOfRecentMissingRecHitsHO->update();
      NumberOfRecentMissingRecHitsHF->update();

      for (HBHERecHitCollection::const_iterator j=hbHits.begin();
	   j!=hbHits.end(); ++j)
	process_RecHit(j);
      
      for (HORecHitCollection::const_iterator k=hoHits.begin();
	   k!=hoHits.end(); ++k)
	process_RecHit(k);
      
      for (HFRecHitCollection::const_iterator j=hfHits.begin();
	   j!=hfHits.end(); ++j)
	process_RecHit(j);
	
    } // if (deadmon_test_rechits)

  Nevents->Fill(0,1);

  if (!makeDiagnostics_) return;
  if (tevt_>=NLumiBlocks_) return;
  // Diagnostic plots -- add number of missing channels vs event number
  int hbpresent=0;
  int hepresent=0;
  int hopresent=0;
  int hfpresent=0;
  int ieta=0;
  for (int d=0;d<4;++d)
    {
      for (int phi=0;phi<72;++phi)
	{
	  for (int eta=0;eta<85;++eta)
	    {
	      if (!present_digi[eta][phi][d]) continue;
	      if (d==3) ++hopresent;
	      else if (d==2) ++hepresent;
	      else if (d==1)
		{
		  ieta=binmapd2[eta];//JEFF
		  //if (abs(ieta)>29) continue;
		  if (abs(ieta)>29) ++hfpresent;
		  else if (abs(ieta)<17) ++hbpresent; //depths 15&16
		  else ++hepresent;
		}
	      else if (d==0)
		{
		  ieta=eta-42;
		  if (abs(ieta)>29) ++hfpresent;
		  else if (abs(ieta)<17) ++hbpresent;
		  else ++hepresent;
		}
	    }
	}
    } // for (int d=0;d<4;++d)
  HBDeadVsEvent->Fill(tevt_,2592-hbpresent);
  HEDeadVsEvent->Fill(tevt_,2592-hepresent);
  HODeadVsEvent->Fill(tevt_,2160-hopresent);
  HFDeadVsEvent->Fill(tevt_,1728-hfpresent);
  return;
} // void HcalDeadCellMonitor::processEvent(...)

/************************************/


// Digi-based dead cell checks

void HcalDeadCellMonitor::processEvent_HBHEdigi(const HBHEDataFrame digi)
{
  // Simply check whether a digi is present.  If so, increment occupancy counter.
  process_Digi(digi);
  return;
} //void HcalDeadCellMonitor::processEvent_HBHEdigi(HBHEDigiCollection::const_iterator j)

template<class DIGI> 
void HcalDeadCellMonitor::process_Digi(DIGI& digi)
{
  // Remove the validate check as when we figure out how to access bad digis in digi monitor
  //if (!digi.validate()) return; // digi must be good to be counted
  int ieta=digi.id().ieta();
  int iphi=digi.id().iphi();
  int depth=digi.id().depth();

  // Fill occupancy counter
  ++recentoccupancy_digi[CalcEtaBin(digi.id().subdet(),ieta,depth)][iphi-1][depth-1];

  // If previously-missing digi found, change boolean status and fill histogram
  if (present_digi[CalcEtaBin(digi.id().subdet(),ieta,depth)][iphi-1][depth-1]==false)
    {
      if (DigiPresentByDepth.depth[depth-1])
	{
	  DigiPresentByDepth.depth[depth-1]->setBinContent(CalcEtaBin(digi.id().subdet(),ieta,depth)+1,iphi,1);
	}
      present_digi[CalcEtaBin(digi.id().subdet(),ieta,depth)][iphi-1][depth-1]=true;
    }
  return;
}

//RecHit-based dead cell checks

template<class RECHIT>
void HcalDeadCellMonitor::process_RecHit(RECHIT& rechit)
{
  float en = rechit->energy();
  HcalDetId id(rechit->detid().rawId());
  int ieta = id.ieta();
  int iphi = id.iphi();
  int depth = id.depth();
  
  if (id.subdet()==HcalBarrel)
    {
      if (en>=HBenergyThreshold_)
	{
	  ++recentoccupancy_rechit[CalcEtaBin(id.subdet(),ieta,depth)][iphi-1][depth-1];
	  present_rechit[CalcEtaBin(id.subdet(),ieta,depth)][iphi-1][depth-1]=true;
	  if (RecHitPresentByDepth.depth[depth-1])
	    RecHitPresentByDepth.depth[depth-1]->setBinContent(CalcEtaBin(id.subdet(),ieta,depth)+1,iphi,1);
	}
    }
  else if (id.subdet()==HcalEndcap)
    {
      if (en>=HEenergyThreshold_)
	{
	++recentoccupancy_rechit[CalcEtaBin(id.subdet(),ieta,depth)][iphi-1][depth-1];
	present_rechit[CalcEtaBin(id.subdet(),ieta,depth)][iphi-1][depth-1]=true;
	if (RecHitPresentByDepth.depth[depth-1])
	  RecHitPresentByDepth.depth[depth-1]->setBinContent(CalcEtaBin(id.subdet(),ieta,depth)+1,iphi,1);
	}
    }
  else if (id.subdet()==HcalForward)
    {
      if (en>=HFenergyThreshold_)
	{
	  ++recentoccupancy_rechit[CalcEtaBin(id.subdet(),ieta,depth)][iphi-1][depth-1];
	
	  present_rechit[CalcEtaBin(id.subdet(),ieta,depth)][iphi-1][depth-1]=true;
	  if (RecHitPresentByDepth.depth[depth-1])
	    RecHitPresentByDepth.depth[depth-1]->setBinContent(CalcEtaBin(id.subdet(),ieta,depth)+1,iphi,1);
	}
    }
  else if (id.subdet()==HcalOuter)
    {
      if (en>=HOenergyThreshold_)
	{
	  ++recentoccupancy_rechit[CalcEtaBin(id.subdet(),ieta,depth)][iphi-1][depth-1];
	  present_rechit[CalcEtaBin(id.subdet(),ieta,depth)][iphi-1][depth-1]=true;
	  if (RecHitPresentByDepth.depth[depth-1])
	    RecHitPresentByDepth.depth[depth-1]->setBinContent(CalcEtaBin(id.subdet(),ieta,depth)+1,iphi,1); 
	}
    }
}

void HcalDeadCellMonitor::fillNevents_recentdigis()
{
  // Fill Histograms showing digi cells with no occupancy for the past few lumiblocks
  if (!deadmon_test_digis_) return; // extra protection here against calling histograms than don't exist

  if (deadevt_ < minDeadEventCount_) return; // not enough entries to make a determination for this LS

  if (debug_>0)
    std::cout <<"<HcalDeadCellMonitor::fillNevents_recentdigis> CHECKING FOR RECENT MISSING DIGIS   evtcount = "<<deadevt_<<std::endl;

  int ieta=0;
  int iphi=0;

  int etabins=0;
  int phibins=0;
  for (unsigned int depth=0;depth<RecentMissingDigisByDepth.depth.size();++depth)
    { 
      etabins=RecentMissingDigisByDepth.depth[depth]->getNbinsX();
      phibins=RecentMissingDigisByDepth.depth[depth]->getNbinsY();
      for (int eta=0;eta<etabins;++eta)
	{
	  for (int subdet=1;subdet<=4;++subdet)
	    {
	      ieta=CalcIeta((HcalSubdetector)subdet,eta,depth+1);
	      if (ieta==-9999) continue;
	      for (int phi=0;phi<phibins;++phi)
		{
		  iphi=phi+1;
		  
		  if (!validDetId((HcalSubdetector)subdet, ieta, iphi, depth+1))
		    continue;
		  
		  // Ignore subdetectors that weren't in run?
		  /*
		  if ((subdet==HcalBarrel && !HBpresent_) || 
		      (subdet==HcalEndcap &&!HEpresent_)  ||
		      (subdet==HcalOuter &&!HOpresent_)  || 
		      (subdet==HcalForward &&!HFpresent_))   continue;
		  */
		  int zside=0;
		  if (subdet==HcalForward) // shift HcalForward ieta
		    ieta<0 ? zside=-1 : zside=+1;
		  
		  if (recentoccupancy_digi[eta][phi][depth]==0)
		    {
		      if (debug_>0)
			{
			  std::cout <<"DEAD CELL; NO RECENT OCCUPANCY: subdet = "<<subdet<<", ieta = "<<ieta<<", iphi = "<<iphi<<" depth = "<<depth+1<<std::endl;
			  std::cout <<"\t RAW COORDINATES:  eta = "<<eta<< " phi = "<<phi<<" depth = "<<depth<<std::endl;
 			  std::cout <<"\t Present? "<<present_digi[eta][phi][depth]<<std::endl;
			}
			     // no digi was found for the N events; Fill cell as bad for all N events (N = checkN);
		      if (RecentMissingDigisByDepth.depth[depth]) RecentMissingDigisByDepth.depth[depth]->Fill(ieta+zside,iphi,deadevt_);
		    }
		} // for (int subdet=1;subdet<=4;++subdet)
	    } // for (int phi=0;...)
	} // for (int eta=0;...)
    } //for (int depth=1;...)
  FillUnphysicalHEHFBins(RecentMissingDigisByDepth);

  return;

} // void HcalDeadCellMonitor::fillNevents_recentdigis()



/* ----------------------------------- */

void HcalDeadCellMonitor::fillNevents_recentrechits()
{
  // Fill Histograms showing unoccupied rechits, or rec hits with low energy

  // This test is a bit pointless, unless the energy threshold is greater than the ZS threshold.
  // If we require that cells are always < thresh to be flagged by this test, and if 
  // thresh < ZS, then we will never catch any cells, since they'll show up as dead in the
  // neverpresent/occupancy test plots first.
  // Only exception is if something strange is going on between ZS ADC value an RecHit energy?

  if (!deadmon_test_rechits_) return;
  if (deadevt_ < minDeadEventCount_) return; // not enough entries to make a determination for this LS

  if (debug_>0)
    std::cout <<"<HcalDeadCellMonitor::fillNevents_energy> BELOW-ENERGY-THRESHOLD PLOTS"<<std::endl;

  int ieta=0;
  int iphi=0;

  int etabins=0;
  int phibins=0;
  for (unsigned int depth=0;depth<RecentMissingRecHitsByDepth.depth.size();++depth)
    { 
      etabins=RecentMissingRecHitsByDepth.depth[depth]->getNbinsX();
      phibins=RecentMissingRecHitsByDepth.depth[depth]->getNbinsY();
      for (int eta=0;eta<etabins;++eta)
	{
	  for (int subdet=1;subdet<=4;++subdet)
	    {
	      ieta=CalcIeta((HcalSubdetector)subdet,eta,depth+1);
	      if (ieta==-9999) continue;
	      for (int phi=0;phi<phibins;++phi)
		{
		  iphi=phi+1;
		  if (!validDetId((HcalSubdetector)subdet, ieta, iphi, depth+1))
		    continue;
		  if (recentoccupancy_rechit[eta][phi][depth]>0) continue; // cell exceeded energy at least once, so it's not dead

		  // Ignore subdetectors that weren't in run?
		  /*
                  if ((subdet==HcalBarrel && !HBpresent_) || 
		      (subdet==HcalEndcap &&!HEpresent_)  ||
		      (subdet==HcalOuter &&!HOpresent_)  ||
		      (subdet==HcalForward &&!HFpresent_))   continue;
		  */

		  int zside=0;
		  if (subdet==HcalForward) // shift HcalForward ieta
		    {
		      ieta<0 ? zside=-1 : zside=+1;
		    }
		  
		  if (debug_>2) 
		    std::cout <<"DEAD CELL; BELOW ENERGY THRESHOLD; subdet = "<<subdet<<" ieta = "<<ieta<<", phi = "<<iphi<<" depth = "<<depth+1<<std::endl;
			  
		  if (RecentMissingRecHitsByDepth.depth[depth]) RecentMissingRecHitsByDepth.depth[depth]->Fill(ieta+zside,iphi,deadevt_);
		} // loop on phi bins
	    } // for (unsigned int depth=1;depth<=4;++depth)
	} // // loop on subdetectors
    } // for (int eta=0;...)

  FillUnphysicalHEHFBins(RecHitPresentByDepth);
  FillUnphysicalHEHFBins(RecentMissingRecHitsByDepth);

  return;
} // void HcalDeadCellMonitor::fillNevents_recentrechits()



void HcalDeadCellMonitor::fillNevents_problemCells()
{
  //fillNevents_problemCells now only performs checks of never-present cells

  if (debug_>0)
    std::cout <<"<HcalDeadCellMonitor::fillNevents_problemCells> FILLING PROBLEM CELL PLOTS"<<std::endl;

  int ieta=0;
  int iphi=0;

  // Count problem cells in each subdetector

  NumBadHB=0;
  NumBadHE=0;
  NumBadHO=0;
  NumBadHF=0;
  
  unsigned int neverpresentHB=0;
  unsigned int neverpresentHE=0;
  unsigned int neverpresentHO=0;
  unsigned int neverpresentHF=0;
  
  unsigned int unoccupiedHB=0;
  unsigned int unoccupiedHE=0;
  unsigned int unoccupiedHO=0;
  unsigned int unoccupiedHF=0;
    
  unsigned int belowenergyHB=0;
  unsigned int belowenergyHE=0;
  unsigned int belowenergyHO=0;
  unsigned int belowenergyHF=0;
  
  unsigned int energyneverpresentHB=0;
  unsigned int energyneverpresentHE=0;
  unsigned int energyneverpresentHO=0;
  unsigned int energyneverpresentHF=0;

  if (deadevt_>=minDeadEventCount_)
    Nevents->Fill(1,deadevt_);

  int etabins=0;
  int phibins=0;
  for (unsigned int depth=0;depth<DigiPresentByDepth.depth.size();++depth)
    {

      etabins=DigiPresentByDepth.depth[depth]->getNbinsX();
      phibins=DigiPresentByDepth.depth[depth]->getNbinsY();
      for (int eta=0;eta<etabins;++eta)
	{
	  for (int phi=0;phi<phibins;++phi)
	    {
	      iphi=phi+1;
	      for (int subdet=1;subdet<=4;++subdet)
		{
		  ieta=CalcIeta((HcalSubdetector)subdet,eta,depth+1);
		  if (ieta==-9999) continue;
		  if (!validDetId((HcalSubdetector)subdet, ieta, iphi, depth+1))
		    continue;
		  // Ignore subdetectors that weren't in run?
		  /*
                  if ((subdet==HcalBarrel && !HBpresent_) || 
		      (subdet==HcalEndcap &&!HEpresent_)  ||
		      (subdet==HcalOuter &&!HOpresent_)  ||
		      (subdet==HcalForward &&!HFpresent_))   continue;
		  */
		  
		  /*
		  if ((!checkHB_ && subdet==HcalBarrel) ||
		      (!checkHE_ && subdet==HcalEndcap) ||
		      (!checkHO_ && subdet==HcalOuter) ||
		      (!checkHF_ && subdet==HcalForward))  continue;
		  */

		  // now check which dead cell tests failed; increment counter if any failed
		  if ((present_digi[eta][phi][depth]==0) ||
		      (deadmon_test_digis_ && recentoccupancy_digi[eta][phi][depth]==0 && (deadevt_>=minDeadEventCount_)) ||
		      (deadmon_test_rechits_ && recentoccupancy_rechit[eta][phi][depth]==0  && (deadevt_>=minDeadEventCount_))
		      )
		    {
		      if (subdet==HcalBarrel)       ++NumBadHB;
		      else if (subdet==HcalEndcap)  ++NumBadHE;
		      else if (subdet==HcalOuter)   ++NumBadHO;
		      else if (subdet==HcalForward) ++NumBadHF;
		    }
		  if (present_digi[eta][phi][depth]==0)
		    {
		      if (subdet==HcalBarrel) ++neverpresentHB;
		      else if (subdet==HcalEndcap) ++neverpresentHE;
		      else if (subdet==HcalOuter) ++neverpresentHO;
		      else if (subdet==HcalForward) ++neverpresentHF;
		    }
		  // Count recent unoccupied digis if the total events in this lumi section is > minEvents_
		  if (deadmon_test_digis_ && recentoccupancy_digi[eta][phi][depth]==0 && (deadevt_>=minDeadEventCount_))
		    {
		      if (subdet==HcalBarrel) ++unoccupiedHB;
		      else if (subdet==HcalEndcap) ++unoccupiedHE;
		      else if (subdet==HcalOuter) ++unoccupiedHO;
		      else if (subdet==HcalForward) ++unoccupiedHF;
		    }
		  // Look at rechit checks
		  if (deadmon_test_rechits_)
		    {
		      if (present_rechit[eta][phi][depth]==0)
			{
			  if (subdet==HcalBarrel) ++energyneverpresentHB;
			  else if (subdet==HcalEndcap) ++energyneverpresentHE;
			  else if (subdet==HcalOuter) ++energyneverpresentHO;
			  else if (subdet==HcalForward) ++energyneverpresentHF;
			}
		      if (recentoccupancy_rechit[eta][phi][depth]==0 && (deadevt_>=minDeadEventCount_))
			{
			  if (subdet==HcalBarrel) ++belowenergyHB;
			  else if (subdet==HcalEndcap) ++belowenergyHE;
			  else if (subdet==HcalOuter) ++belowenergyHO;
			  else if (subdet==HcalForward) ++belowenergyHF;
			}
		    }
		} // subdet loop
	    } // phi loop
	} //eta loop
    } // depth loop

  // Fill with number of problem cells found on this pass


  NumberOfNeverPresentDigisHB->Fill(currentLS,neverpresentHB);
  NumberOfNeverPresentDigisHE->Fill(currentLS,neverpresentHE);
  NumberOfNeverPresentDigisHO->Fill(currentLS,neverpresentHO);
  NumberOfNeverPresentDigisHF->Fill(currentLS,neverpresentHF);
  NumberOfNeverPresentDigis->Fill(currentLS,neverpresentHB+neverpresentHE+neverpresentHO+neverpresentHF);
  
  ProblemsVsLB_HB->Fill(currentLS,NumBadHB);
  ProblemsVsLB_HE->Fill(currentLS,NumBadHE);
  ProblemsVsLB_HO->Fill(currentLS,NumBadHO);
  ProblemsVsLB_HF->Fill(currentLS,NumBadHF);
  ProblemsVsLB->Fill(currentLS,NumBadHB+NumBadHE+NumBadHO+NumBadHF);
  
  if (deadevt_<minDeadEventCount_)
    return;
  
  if (deadmon_test_digis_)
    {
      NumberOfRecentMissingDigisHE->Fill(currentLS,unoccupiedHB);
      NumberOfRecentMissingDigisHE->Fill(currentLS,unoccupiedHE);
      NumberOfRecentMissingDigisHO->Fill(currentLS,unoccupiedHO);
      NumberOfRecentMissingDigisHF->Fill(currentLS,unoccupiedHF);
      NumberOfRecentMissingDigis->Fill(currentLS,unoccupiedHB+unoccupiedHE+unoccupiedHO+unoccupiedHF);
    }
  
  if (deadmon_test_rechits_)
    {
      NumberOfNeverPresentRecHitsHB->Fill(currentLS,energyneverpresentHB);
      NumberOfNeverPresentRecHitsHE->Fill(currentLS,energyneverpresentHE);
      NumberOfNeverPresentRecHitsHO->Fill(currentLS,energyneverpresentHO);
      NumberOfNeverPresentRecHitsHF->Fill(currentLS,energyneverpresentHF);
      NumberOfNeverPresentRecHits->Fill(currentLS,energyneverpresentHB+energyneverpresentHE+energyneverpresentHO+energyneverpresentHF);
      
      NumberOfRecentMissingRecHitsHB->Fill(currentLS,belowenergyHB);
      NumberOfRecentMissingRecHitsHE->Fill(currentLS,belowenergyHE);
      NumberOfRecentMissingRecHitsHO->Fill(currentLS,belowenergyHO);
      NumberOfRecentMissingRecHitsHF->Fill(currentLS,belowenergyHF);
      NumberOfRecentMissingRecHits->Fill(currentLS,belowenergyHB+belowenergyHE+belowenergyHO+belowenergyHF);
    }

  return;
} // void HcalDeadCellMonitor::fillNevents_problemCells(void)


void HcalDeadCellMonitor::zeroCounters(bool resetpresent)
{

  // zero all counters

  // 2D histogram counters
  for (unsigned int i=0;i<85;++i)
    {
      for (unsigned int j=0;j<72;++j)
	{
	  for (unsigned int k=0;k<4;++k)
	    {
	      if (resetpresent) present_digi[i][j][k]=false; // keeps track of whether digi was ever present
	      if (resetpresent) present_rechit[i][j][k]=false;
	      recentoccupancy_digi[i][j][k]=0; // counts occupancy in last (checkNevents) events
	      recentoccupancy_rechit[i][j][k]=0; // counts instances of cell above threshold energy in last (checkNevents)
	    }
	}
    }

  return;
} // void HcalDeadCellMonitor::zeroCounters(bool resetpresent)

DEFINE_ANOTHER_FWK_MODULE(HcalDeadCellMonitor);
