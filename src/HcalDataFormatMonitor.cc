#include "DQM/HcalMonitorTasks/interface/HcalDataFormatMonitor.h"

HcalDataFormatMonitor::HcalDataFormatMonitor() {}

HcalDataFormatMonitor::~HcalDataFormatMonitor() {
}

void HcalDataFormatMonitor::clearME(){
  if(m_dbe){
    m_dbe->setCurrentFolder("HcalMonitor/DataFormatMonitor");
    m_dbe->removeContents();
  }
  return;
}

void HcalDataFormatMonitor::setup(const edm::ParameterSet& ps, DaqMonitorBEInterface* dbe){
  HcalBaseMonitor::setup(ps,dbe);
  
  ievt_=0;
  fedUnpackList_ = ps.getParameter<vector<int> >("FEDs");
  firstFED_ = ps.getParameter<int>("HcalFirstFED");
  cout << "HcalDataFormatMonitor::setup  Will unpack FEDs ";
  for (unsigned int i=0; i<fedUnpackList_.size(); i++) 
    cout << fedUnpackList_[i] << " ";
  cout << endl;

  if ( m_dbe ) {
    m_dbe->setCurrentFolder("HcalMonitor/DataFormatMonitor");    

    meEVT_ = m_dbe->bookInt("Data Format Task Event Number");    
    meEVT_->Fill(ievt_);

    char* type = "Spigot Format Errors";
    meSpigotFormatErrors_=  m_dbe->book1D(type,type,100,0,100);
    type = "Bad Quality Digis";
    meBadQualityDigis_=  m_dbe->book1D(type,type,100,0,100);
    type = "Unmapped Digis";
    meUnmappedDigis_=  m_dbe->book1D(type,type,500,0,500);
    type = "Unmapped Trigger Primitive Digis";
    meUnmappedTPDigis_=  m_dbe->book1D(type,type,500,0,500);
    type = "FED Error Map";
    meFEDerrorMap_ = m_dbe->book1D(type,type,100,0,100);


    type = "HB Data Format Error Words";
    hbHists.DCC_ERRWD =  m_dbe->book1D(type,type,12,-0.5,11.5);
    type = "HB Data Format Crate Error Map";
    hbHists.ERR_MAP = m_dbe->book2D(type,type,21,-0.5,20.5,21,-0.5,20.5);
    type = "HB Data Format Spigot Error Map";
    hbHists.SpigotMap = m_dbe->book2D(type,type,
				      HcalDCCHeader::SPIGOT_COUNT,-0.5,HcalDCCHeader::SPIGOT_COUNT-0.5,
				      fedUnpackList_.size(),0,fedUnpackList_.size());

    type = "HE Data Format Error Words";
    heHists.DCC_ERRWD =  m_dbe->book1D(type,type,12,-0.5,11.5);
    type = "HE Data Format Crate Error Map";
    heHists.ERR_MAP = m_dbe->book2D(type,type,21,-0.5,20.5,21,-0.5,20.5);
    type = "HE Data Format Spigot Error Map";
    heHists.SpigotMap = m_dbe->book2D(type,type,
				      HcalDCCHeader::SPIGOT_COUNT,-0.5,HcalDCCHeader::SPIGOT_COUNT-0.5,
				      fedUnpackList_.size(),0,fedUnpackList_.size());

    
    type = "HF Data Format Error Words";
    hfHists.DCC_ERRWD =  m_dbe->book1D(type,type,12,-0.5,11.5);
    type = "HF Data Format Crate Error Map";
    hfHists.ERR_MAP = m_dbe->book2D(type,type,21,-0.5,20.5,21,-0.5,20.5);
    type = "HF Data Format Spigot Error Map";
    hfHists.SpigotMap = m_dbe->book2D(type,type,
				      HcalDCCHeader::SPIGOT_COUNT,-0.5,HcalDCCHeader::SPIGOT_COUNT-0.5,
				      fedUnpackList_.size(),0,fedUnpackList_.size());

    type = "HO Data Format Error Words";
    hoHists.DCC_ERRWD =  m_dbe->book1D(type,type,12,-0.5,11.5);
    type = "HO Data Format Crate Error Map";
    hoHists.ERR_MAP = m_dbe->book2D(type,type,21,-0.5,20.5,21,-0.5,20.5);
    type = "HO Data Format Spigot Error Map";
    hoHists.SpigotMap = m_dbe->book2D(type,type,
				      HcalDCCHeader::SPIGOT_COUNT,0,HcalDCCHeader::SPIGOT_COUNT-1,
				      fedUnpackList_.size(),0,fedUnpackList_.size());

  }

  return;
}

void HcalDataFormatMonitor::processEvent(const FEDRawDataCollection& rawraw, const HcalUnpackerReport& report, const HcalElectronicsMap& emap){

  if(!m_dbe) { printf("HcalDataFormatMonitor::processEvent   DaqMonitorBEInterface not instantiated!!!\n");  return;}
  
  ievt_++;
  meEVT_->Fill(ievt_);

  for (vector<int>::const_iterator i=fedUnpackList_.begin(); i!=fedUnpackList_.end(); i++) {
    const FEDRawData& fed = rawraw.FEDData(*i);
    unpack(fed,emap);
  }

  meSpigotFormatErrors_->Fill(report.spigotFormatErrors());
  meBadQualityDigis_->Fill(report.badQualityDigis());
  meUnmappedDigis_->Fill(report.unmappedDigis());
  meUnmappedTPDigis_->Fill(report.unmappedTPDigis());
  
  for(unsigned int i=0; i<report.getFedsError().size(); i++){    
    const int m = report.getFedsError()[i];
    const FEDRawData& fed = rawraw.FEDData(m);
    const HcalDCCHeader* dccHeader=(const HcalDCCHeader*)(fed.data());
    if(!dccHeader) continue;
    int dccid=dccHeader->getSourceId()-firstFED_;  
    meFEDerrorMap_->Fill(dccid);
  }

  return;
}

void HcalDataFormatMonitor::unpack(const FEDRawData& raw, const HcalElectronicsMap& emap){
  
  // get the DataFormat header
  const HcalDCCHeader* dccHeader=(const HcalDCCHeader*)(raw.data());
  // check the summary status
  if(!dccHeader) return;
  int dccid=dccHeader->getSourceId()-firstFED_;  
  
  // walk through the HTR data...
  HcalHTRData htr;
  
  for (int spigot=0; spigot<HcalDCCHeader::SPIGOT_COUNT; spigot++) {
    
    if (!dccHeader->getSpigotPresent(spigot)) continue;    
    dccHeader->getSpigotData(spigot,htr);
    
    // check
    if (!htr.check() || htr.isHistogramEvent()) continue;
    
    MonitorElement* tmpErr = 0; 
    MonitorElement* tmpMapC = 0;
    MonitorElement* tmpMapS = 0;

    bool valid = false;
    for(int fchan=0; fchan<3 && !valid; fchan++){
      for(int fib=0; fib<9 && !valid; fib++){
	HcalElectronicsId eid(fchan,fib,spigot,dccid);
	eid.setHTR(htr.readoutVMECrateId(),htr.htrSlot(),htr.htrTopBottom());
	DetId did=emap.lookup(eid);
	if (!did.null()) {
	  switch (((HcalSubdetector)did.subdetId())) {
	  case (HcalBarrel): {
	    tmpErr = hbHists.DCC_ERRWD; tmpMapC = hbHists.ERR_MAP;
	    tmpMapS = hbHists.SpigotMap; 
	    valid = true;
	  } break;
	  case (HcalEndcap): {
	    tmpErr = heHists.DCC_ERRWD; tmpMapC = heHists.ERR_MAP;
	    tmpMapS = heHists.SpigotMap; 
	    valid = true;
	  } break;
	  case (HcalOuter): {
	    tmpErr = hoHists.DCC_ERRWD; tmpMapC = hoHists.ERR_MAP; 
	    tmpMapS = hoHists.SpigotMap; 
	    valid = true;
	  } break;
	  case (HcalForward): {
	    tmpErr = hfHists.DCC_ERRWD; tmpMapC = hfHists.ERR_MAP; 
	    tmpMapS = hfHists.SpigotMap; 
	    valid = true;
	  } break;
	  default: break;
	  }
	}
      }
    }
    
    int err = htr.getErrorsWord();
    if(tmpErr!=NULL){
      for(int i=0; i<12; i++)
	tmpErr->Fill(i,err&(0x01<<i));    
    }
    if(err>0 && tmpMapC!=NULL){
      tmpMapC->Fill(htr.readoutVMECrateId(),htr.htrSlot());
      tmpMapS->Fill(spigot,dccid);
    }  
  
  }
  
  return;
}
