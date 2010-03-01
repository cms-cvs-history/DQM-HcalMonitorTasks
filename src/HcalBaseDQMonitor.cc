#include <DQM/HcalMonitorTasks/interface/HcalBaseDQMonitor.h>

/*
 * \file HcalBaseDQMonitor.cc
 *
 * $Date: 2010/02/28 16:12:33 $
 * $Revision: 1.1.2.5 $
 * \author J Temple
 *
 * Base class for all Hcal DQM analyzers
 *

*/

using namespace std;
using namespace edm;

// constructor

HcalBaseDQMonitor::HcalBaseDQMonitor(const ParameterSet& ps)
{
  Online_                = ps.getParameter<bool>("online");
  mergeRuns_             = ps.getParameter<bool>("mergeRuns");
  enableCleanup_         = ps.getParameter<bool>("enableCleanup");
  debug_                 = ps.getParameter<int>("debug");
  prefixME_              = ps.getParameter<string>("subSystemFolder"); // Hcal
  if (prefixME_.substr(prefixME_.size()-1,prefixME_.size())!="/")
    prefixME_.append("/");
  subdir_                = ps.getParameter<string>("TaskFolder"); // Test
  if (subdir_.size()>0 && subdir_.substr(subdir_.size()-1,subdir_.size())!="/")
    subdir_.append("/");
  subdir_=prefixME_+subdir_;
  AllowedCalibTypes_     = ps.getParameter<vector<int> > ("AllowedCalibTypes");
  skipOutOfOrderLS_      = ps.getParameter<bool>("skipOutOfOrderLS");
  NLumiBlocks_           = ps.getParameter<int>("NLumiBlocks");
  makeDiagnostics_       = ps.getUntrackedParameter<bool>("makeDiagnostics",false);
} //HcalBaseDQMonitor::HcalBaseDQMonitor(const ParameterSet& ps)

// destructor

HcalBaseDQMonitor::~HcalBaseDQMonitor()
{

}

void HcalBaseDQMonitor::beginJob(void)
{

  if (debug_>0) std::cout <<"HcalBaseDQMonitor::beginJob():  task =  '"<<subdir_<<"'"<<endl;
  dbe_ = Service<DQMStore>().operator->();

  ievt_=0;
  levt_=0;
  tevt_=0;
  currenttype_=-1;
  HBpresent_=false;
  HEpresent_=false;
  HOpresent_=false;
  HFpresent_=false;


} // beginJob()

void HcalBaseDQMonitor::endJob(void)
{
  if (enableCleanup_)
    cleanup();
} // endJob()

void HcalBaseDQMonitor::beginRun(const edm::Run& run, const edm::EventSetup& c)
{
  if (debug_>0) std::cout <<"HcalBaseDQMonitor::beginRun():  task =  '"<<subdir_<<"'"<<endl;
  if (mergeRuns_ && tevt_>0) return;
  this->setup();
  this->reset();
} // beginRun(const edm::Run& run, const edm::EventSetup& c)

void HcalBaseDQMonitor::endRun(const edm::Run& run, const edm::EventSetup& c)
{
  if (debug_>0) std::cout <<"HcalBaseDQMonitor::endRun:  task = "<<subdir_<<std::endl;
} //endRun(...)

void HcalBaseDQMonitor::reset(void)
{
  if (debug_>0) std::cout <<"HcalBaseDQMonitor::reset():  task = "<<subdir_<<std::endl;
  if (meIevt_) meIevt_->Fill(-1);
  ievt_=0;
  if (meLevt_) meLevt_->Fill(-1);
  levt_=0;
  if (meTevt_) meTevt_->Fill(-1);
  tevt_=0;
  HBpresent_=false;
  HEpresent_=false;
  HOpresent_=false;
  HFpresent_=false;
  currentLS=0;
  currenttype_=-1;
} //reset()

void HcalBaseDQMonitor::cleanup(void)
{

} //cleanup()

void HcalBaseDQMonitor::setup(void)
{
  dbe_->setCurrentFolder(subdir_);
  meIevt_ = dbe_->bookInt("EventsProcessed");
  if (meIevt_) meIevt_->Fill(-1);
  meLevt_ = dbe_->bookInt("EventsProcessed_currentLS");
  if (meLevt_) meLevt_->Fill(-1);
  meTevt_ = dbe_->bookInt("EventsProcessed_All");
  if (meTevt_) meTevt_->Fill(-1);
  
} // setup()


void HcalBaseDQMonitor::beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
			  const edm::EventSetup& c)
{
  if (this->LumiInOrder(lumiSeg.luminosityBlock())==false) return;
  currentLS=lumiSeg.luminosityBlock();
  levt_=0;
  if (meLevt_) meLevt_->Fill(-1);
}

void HcalBaseDQMonitor::endLuminosityBlock(const edm::LuminosityBlock& lumiSeg,
			const edm::EventSetup& c)
{
  if (this->LumiInOrder(lumiSeg.luminosityBlock())==false) return;
  // Inherited classes do end-of-lumi functions here
}


bool HcalBaseDQMonitor::LumiInOrder(int lumisec)
{
  if (skipOutOfOrderLS_==false) return true; // don't skip out-of-order lumi sections
  // check that latest lumi section is >= last processed
  if (Online_ && lumisec<currentLS)
    return false;
  return true;
}

bool HcalBaseDQMonitor::IsAllowedCalibType()
{
  if (debug_>9) std::cout <<"<HcalBaseDQMonitor::IsAllowedCalibType>"<<std::endl;
  if (AllowedCalibTypes_.size()==0)
    {
      if (debug_>9) std::cout <<"\tNo calib types specified by user; All events allowed"<<std::endl;
      return true;
    }
  MonitorElement* me = dbe_->get((prefixME_+"DQM Job Status/CURRENT_EVENT_TYPE").c_str());
  if (me) currenttype_=me->getIntValue();
  else 
    {
      if (debug_>9) std::cout <<"\tCalib Type cannot be determined from HcalMonitorModule"<<std::endl;
      return true; // is current type can't be determined, assume event is allowed
    }
  if (debug_>9) std::cout <<"\tHcalBaseDQMonitor::IsAllowedCalibType  checking if calibration type = "<<currenttype_<<" is allowed...";
  for (std::vector<int>::size_type i=0;i<AllowedCalibTypes_.size();++i)
    {
      if (AllowedCalibTypes_[i]==currenttype_)
	{
	  if (debug_>9) std::cout <<"\t Type allowed!"<<std::endl;
	  return true;
	}
    }
  if (debug_>9) std::cout <<"\t Type not allowed!"<<std::endl;
  return false;
} // bool HcalBaseDQMonitor::IsAllowedCalibType()

void HcalBaseDQMonitor::analyze(const edm::Event& e, const edm::EventSetup& c)
{
  if (debug_>5) std::cout <<"\t<HcalBaseDQMonitor::analyze>  event = "<<ievt_<<std::endl;
  eventAllowed_=true; // assume event is allowed

  // fill with total events seen (this differs from ievent, which is total # of good events)
  ++tevt_;
  if (meTevt_) meTevt_->Fill(tevt_);
  // skip out of order lumi events
  if (this->LumiInOrder(e.luminosityBlock())==false)
    {
      eventAllowed_=false;
      return;
    }
  // skip events of wrong calibration type
  eventAllowed_&=(this->IsAllowedCalibType());
  if (!eventAllowed_) return;

  // Event is good; count it
  ++ievt_;
  ++levt_;
  if (meIevt_) meIevt_->Fill(ievt_);
  if (meLevt_) meLevt_->Fill(levt_);


  MonitorElement* me;
  if (HBpresent_==false)
    {
      me = dbe_->get((prefixME_+"DQM Job Status/HBpresent"));
      if (me==0 || me->getIntValue()>0) HBpresent_=true;
    }
  if (HEpresent_==false)
    {
      me = dbe_->get((prefixME_+"DQM Job Status/HEpresent"));
      if (me==0 || me->getIntValue()>0) HEpresent_=true;
    }
  if (HOpresent_==false)
    {
      me = dbe_->get((prefixME_+"DQM Job Status/HOpresent"));
      if (me==0 || me->getIntValue()>0) HOpresent_=true;
    }
  if (HFpresent_==false)
    {
      me = dbe_->get((prefixME_+"DQM Job Status/HOpresent"));
      if (me ==0 || me->getIntValue()>0) HFpresent_=true;
    }


} // void HcalBaseDQMonitor::analyze(const edm::Event& e, const edm::EventSetup& c)

DEFINE_FWK_MODULE(HcalBaseDQMonitor);
