#ifndef DQM_HCALMONITORTASKS_HCALPEDESTALMONITOR_H
#define DQM_HCALMONITORTASKS_HCALPEDESTALMONITOR_H

#include "DQM/HcalMonitorTasks/interface/HcalBaseMonitor.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CondFormats/HcalObjects/interface/HcalQIECoder.h"

/** \class HcalPedestalMonitor
  *  
  * $Date: 2006/04/10 16:31:26 $
  * $Revision: 1.2 $
  * \author W. Fisher - FNAL
  */
class HcalPedestalMonitor: public HcalBaseMonitor {
public:
  HcalPedestalMonitor(); 
  ~HcalPedestalMonitor(); 

  void setup(const edm::ParameterSet& ps, DaqMonitorBEInterface* dbe);
  void processEvent(const HBHEDigiCollection& hbhe,
		    const HODigiCollection& ho,
		    const HFDigiCollection& hf,
		    const HcalDbService& cond);
  void done();

private: 
  void perChanHists(int id, const HcalDetId detid, const HcalQIESample& qie, map<HcalDetId, map<int, MonitorElement*> > &tool);

  bool m_doPerChannel;
  map<HcalDetId, map<int,MonitorElement*> >::iterator _meo;

  string m_outputFile;
  const HcalQIEShape* m_shape;
  const HcalQIECoder* m_coder;

  MonitorElement* meEVT_;
  int ievt_;

  struct{
    map<HcalDetId,map<int, MonitorElement*> > PEDVALS;
    MonitorElement* ALLPEDS;
    MonitorElement* PEDRMS;
    MonitorElement* PEDMEAN;    

    MonitorElement* CAPIDRMS;
    MonitorElement* CAPIDMEAN;    

    MonitorElement* QIERMS;
    MonitorElement* QIEMEAN;    

    MonitorElement* ERRGEO;
    MonitorElement* ERRELEC;    
  } hbHists, hfHists, hoHists;

};

#endif
