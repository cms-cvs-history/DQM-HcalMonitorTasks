#ifndef DQM_HCALMONITORTASKS_HCALDATAFORMATMONITOR_H
#define DQM_HCALMONITORTASKS_HCALDATAFORMATMONITOR_H

#include "DQM/HcalMonitorTasks/interface/HcalBaseMonitor.h"
#include "EventFilter/HcalRawToDigi/interface/HcalUnpacker.h"
#include "EventFilter/HcalRawToDigi/interface/HcalDataFrameFilter.h"
#include "EventFilter/HcalRawToDigi/interface/HcalRawToDigi.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "EventFilter/HcalRawToDigi/interface/HcalDCCHeader.h"
#include "EventFilter/HcalRawToDigi/interface/HcalHTRData.h"
#include "DataFormats/HcalDigi/interface/HcalQIESample.h"
#include "CondFormats/HcalMapping/interface/HcalMappingTextFileReader.h"

/** \class Hcaldataformatmonitor
  *  
  * $Date: 2005/11/13 17:33:07 $
  * $Revision: 1.1 $
  * \author W. Fisher - FNAL
  */
class HcalDataFormatMonitor: public HcalBaseMonitor {
public:
  HcalDataFormatMonitor(); 
  ~HcalDataFormatMonitor(); 

  void setup(const edm::ParameterSet& ps, DaqMonitorBEInterface* dbe);
  void processEvent(const FEDRawDataCollection& rawraw);
  void unpack(const FEDRawData& raw, int a, int b, int c);


private: /// Data accessors
  std::auto_ptr<HcalMapping> m_readoutMap;
  std::string m_readoutMapSource;
  std::vector<int> m_fedUnpackList;
  int m_firstFED;

private:  ///Monitoring elements
  MonitorElement* m_meDCC_ERRWD;
  MonitorElement* m_meDCC_FMT;
  
};

#endif