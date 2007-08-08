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

void HcalDataFormatMonitor::setup(const edm::ParameterSet& ps,
				  DaqMonitorBEInterface* dbe){
  HcalBaseMonitor::setup(ps,dbe);
  
  ievt_=0;
  fedUnpackList_ = ps.getParameter<vector<int> >("FEDs");
  firstFED_ = ps.getParameter<int>("HcalFirstFED");
  cout << "HcalDataFormatMonitor::setup  Will unpack FED Crates  ";
  for (unsigned int i=0; i<fedUnpackList_.size(); i++)
    cout << fedUnpackList_[i] << " ";
  cout << endl;
  
  if ( m_dbe ) {
    m_dbe->setCurrentFolder("HcalMonitor/DataFormatMonitor");
    
    meEVT_ = m_dbe->bookInt("Data Format Task Event Number");
    meEVT_->Fill(ievt_);
    
    char* type = "Spigot Format Errors";
    meSpigotFormatErrors_=  m_dbe->book1D(type,type,500,-1,999);
    type = "Bad Quality Digis";
    meBadQualityDigis_=  m_dbe->book1D(type,type,500,-1,999);
    type = "Unmapped Digis";
    meUnmappedDigis_=  m_dbe->book1D(type,type,500,-1,999);
    type = "Unmapped Trigger Primitive Digis";
    meUnmappedTPDigis_=  m_dbe->book1D(type,type,500,-1,999);
    type = "FED Error Map";
    meFEDerrorMap_ = m_dbe->book1D(type,type,33,699.5,732.5);
    type = "Evt Number Out-of-Synch";
     meEvtNumberSynch_= m_dbe->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);
    type = "BCN Not Constant";
    meBCNSynch_= m_dbe->book2D(type,type,40,-0.25,19.75,18,-0.5,17.5);
    type = "BCN";
    meBCN_ = m_dbe->book1D(type,type,3564,-0.5,3563.5);

    // Firmware version
    type = "HTR Firmware Version";
    meFWVersion_ = m_dbe->book2D(type,type ,256,-0.5,255.5,18,-0.5,17.5);

    int maxbits = 16;//Look at all bits
    type = "HTR Error Word by Crate";
    meErrWdCrate_ = m_dbe->book2D(type,type,18,-0.5,17.5,maxbits,-0.5,maxbits-0.5);

    type = "HTR Error Word - Crate 0";
    meCrate0HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 1";
    meCrate1HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 2";
    meCrate2HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 3";
    meCrate3HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 4";
    meCrate4HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 5";
    meCrate5HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 6";
    meCrate6HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 7";
    meCrate7HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 8";
    meCrate8HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 9";
    meCrate9HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 10";
    meCrate10HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 11";
    meCrate11HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 12";
    meCrate12HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 13";
    meCrate13HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 14";
    meCrate14HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 15";
    meCrate15HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 16";
    meCrate16HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    type = "HTR Error Word - Crate 17";
    meCrate17HTRErr_ = m_dbe->book2D(type,type,40,-0.25,19.75,maxbits,-0.5,maxbits-0.5);
    
    type = "HBHE Data Format Error Word";
    hbheHists.DCC_ErrWd =  m_dbe->book1D(type,type,16,-0.5,15.5);

    type = "HF Data Format Error Word";
    hfHists.DCC_ErrWd =  m_dbe->book1D(type,type,16,-0.5,15.5);

    type = "HO Data Format Error Word";
    hoHists.DCC_ErrWd = m_dbe->book1D(type,type,16,-0.5,15.5);

   }

   return;
}

void HcalDataFormatMonitor::processEvent(const FEDRawDataCollection&
					 rawraw, const HcalUnpackerReport& report, 
					 const HcalElectronicsMap& emap){
  
  if(!m_dbe) { 
    printf("HcalDataFormatMonitor::processEvent DaqMonitorBEInterface not instantiated!!!\n");  
    return;
  }
  
  ievt_++;
  meEVT_->Fill(ievt_);
  
  lastEvtN_ = -1;
  lastBCN_ = -1;
  for (vector<int>::const_iterator i=fedUnpackList_.begin();
       i!=fedUnpackList_.end(); i++) {
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
    int dccid=dccHeader->getSourceId();
    meFEDerrorMap_->Fill(dccid);
  }
  
   return;
}

void HcalDataFormatMonitor::unpack(const FEDRawData& raw, const
				   HcalElectronicsMap& emap){
  
  // get the DataFormat header
  const HcalDCCHeader* dccHeader=(const HcalDCCHeader*)(raw.data());
  // check the summary status
  if(!dccHeader) return;
  int dccid=dccHeader->getSourceId();

  // walk through the HTR data...
  HcalHTRData htr;
  
  for (int spigot=0; spigot<HcalDCCHeader::SPIGOT_COUNT; spigot++) {
    
    if (!dccHeader->getSpigotPresent(spigot)) continue;
    dccHeader->getSpigotData(spigot,htr);

    int cratenum = htr.readoutVMECrateId();
    float slotnum = htr.htrSlot() + 0.5*htr.htrTopBottom();

    // check
    if (!htr.check() || htr.isHistogramEvent()) continue;
    unsigned int htrBCN = htr.getBunchNumber(); 
    meBCN_->Fill(htrBCN);
    unsigned int htrFWVer = htr.getFirmwareRevision() & 0xFF;
    meFWVersion_->Fill(htrFWVer,cratenum);
    ///check that all HTRs have the same L1A number
    if(lastEvtN_==-1) lastEvtN_ = htr.getL1ANumber();  ///the first one will be the reference
    else{
      if(htr.getL1ANumber()!=lastEvtN_) meEvtNumberSynch_->Fill(slotnum,cratenum);
    }
    ///check that all HTRs have the same BCN
    if(lastBCN_==-1) lastBCN_ = htr.getBunchNumber();  ///the first one will be the reference
    else{
      if(htr.getBunchNumber()!=lastBCN_) meBCNSynch_->Fill(slotnum,cratenum);
    }
 
    MonitorElement* tmpErr = 0;

    bool valid = false;
    for(int fchan=0; fchan<3 && !valid; fchan++){
      for(int fib=0; fib<9 && !valid; fib++){
	HcalElectronicsId eid(fchan,fib,spigot,dccid-firstFED_);
	eid.setHTR(htr.readoutVMECrateId(),htr.htrSlot(),htr.htrTopBottom());
	DetId did=emap.lookup(eid);
	if (!did.null()) {
	  switch (((HcalSubdetector)did.subdetId())) {
	  case (HcalBarrel): {
	    tmpErr = hbheHists.DCC_ErrWd;
	    valid = true;
	  } break;
	  case (HcalEndcap): {
	    tmpErr = hbheHists.DCC_ErrWd;
	    valid = true;
	  } break;
	  case (HcalOuter): {
	    tmpErr = hoHists.DCC_ErrWd;
	    valid = true;
	  } break;
	  case (HcalForward): {
	    tmpErr = hfHists.DCC_ErrWd; 
	    valid = true;
	  } break;
	  default: break;
	  }
	}
      }
    }
     
     int errWord = htr.getErrorsWord() & 0x1FFFF;
     if(tmpErr!=NULL){
       for(int i=0; i<16; i++){
	 int errbit = errWord&(0x01<<i);
   // Bit 15 should always be 1; consider it an error if it isn't.
	 if (i==15) errbit = errbit - 0x8000;
	 if (errbit !=0){
	   tmpErr->Fill(i);
	   meErrWdCrate_->Fill(cratenum,i);
	   if (cratenum ==0)meCrate0HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==1)meCrate1HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==2)meCrate2HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==3)meCrate3HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==4)meCrate4HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==5)meCrate5HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==6)meCrate6HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==7)meCrate7HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==8)meCrate8HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==9)meCrate9HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==10)meCrate10HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==11)meCrate11HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==12)meCrate12HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==13)meCrate13HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==14)meCrate14HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==15)meCrate15HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==16)meCrate16HTRErr_ -> Fill(slotnum,i);
	   if (cratenum ==17)meCrate17HTRErr_ -> Fill(slotnum,i);
	 } 
       }
     }


   }

   return;
}



