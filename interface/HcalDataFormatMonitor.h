#ifndef DQM_HCALMONITORTASKS_HCALDATAFORMATMONITOR_H
#define DQM_HCALMONITORTASKS_HCALDATAFORMATMONITOR_H

#define  IETAMIN -43
#define  IETAMAX 43
#define  IPHIMIN 0
#define  IPHIMAX 71
#define  HBHE_LO_DCC 700
#define  HBHE_HI_DCC 717
#define  HF_LO_DCC   718
#define  HF_HI_DCC   724
#define  HO_LO_DCC   725
#define  HO_HI_DCC   731

#include "DQM/HcalMonitorTasks/interface/HcalBaseMonitor.h"
#include "EventFilter/HcalRawToDigi/interface/HcalUnpacker.h"
#include "EventFilter/HcalRawToDigi/interface/HcalHTRData.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

/** \class Hcaldataformatmonitor
 *
 * $Date: 2008/08/28 17:03:51 $
 * $Revision: 1.30 $
 * \author W. Fisher - FNAL
 */
class HcalDataFormatMonitor: public HcalBaseMonitor {
 public:
  HcalDataFormatMonitor();
  ~HcalDataFormatMonitor();
  
  void setup(const edm::ParameterSet& ps, DQMStore* dbe);
  //  void setup(const edm::ParameterSet& ps, DQMStore* dbe,const HcalElectronicsMap& emap );
  void processEvent(const FEDRawDataCollection& rawraw, const
		    HcalUnpackerReport& report, const HcalElectronicsMap& emap);
  void unpack(const FEDRawData& raw, const HcalElectronicsMap& emap);
  void clearME();
  void reset();

  void HTRPrint(const HcalHTRData& htr,int prtlvl);
  void labelHTRBits(MonitorElement* mePlot,unsigned int axisType);
  void labelthezoo (MonitorElement* zoo);

 public: //Electronics map -> geographic channel map
  void smuggleMaps(std::map<uint32_t, std::vector<HcalDetId> >& givenDCCtoCell,
		  std::map<pair <int,int> , std::vector<HcalDetId> >& givenHTRtoCell);
  std::map<uint32_t, std::vector<HcalDetId> > DCCtoCell;
  std::map<uint32_t, std::vector<HcalDetId> > ::iterator thisDCC;
  std::map<pair <int,int> , std::vector<HcalDetId> > HTRtoCell;
  std::map<pair <int,int> , std::vector<HcalDetId> > ::iterator thisHTR;

  private: 
  //backstage accounting mechanisms for the ProblemMap
  static size_t iphirange; // = IPHIMAX - IPHIMIN;
  static size_t ietarange; // = IETAMAX - IETAMIN;
  std::vector<std::vector<bool> > problemhere;  // Whole HCAL
  std::vector<std::vector<bool> > problemHB;    //  
  std::vector<std::vector<bool> > problemHE;    //  
  std::vector<std::vector<bool> > problemHF;    // Includes ZDC?
  std::vector<std::vector<bool> > problemHO;    //  
  void mapHTRproblem (int dcc, int spigot) ;
  void mapDCCproblem(int dcc) ;
  void fillzoos(int bin, int dccid);
  std::vector<std::vector<uint64_t> > phatmap;  // iphi/ieta projection of all hcal cells
  std::vector<std::vector<uint64_t> > HBmap;  // iphi/ieta projection of hb
  std::vector<std::vector<uint64_t> > HEmap;  // iphi/ieta projection of he
  std::vector<std::vector<uint64_t> > HFmap;  // iphi/ieta projection of hf
  std::vector<std::vector<uint64_t> > HOmap;  // iphi/ieta projection of ho
  void UpdateMap();

  // Data accessors
  vector<int> fedUnpackList_;
  vector<int> dccCrate_;
  vector<HcalSubdetector> dccSubdet_;
  int firstFED_;
  int ievt_;
  int lastEvtN_;
  int lastBCN_;
  //   int dccnum_;
  //int cratenum_;
  
  int prtlvl_;
  
 private:  //Monitoring elements
   
  MonitorElement* meEVT_;
  MonitorElement* DATAFORMAT_PROBLEM_MAP;
  MonitorElement* DATAFORMAT_PROBLEM_ZOO;
  MonitorElement* HB_DATAFORMAT_PROBLEM_MAP;
  MonitorElement* HBHE_DATAFORMAT_PROBLEM_ZOO;
  MonitorElement* HE_DATAFORMAT_PROBLEM_MAP;
  MonitorElement* HF_DATAFORMAT_PROBLEM_MAP;
  MonitorElement* HF_DATAFORMAT_PROBLEM_ZOO;
  MonitorElement* HO_DATAFORMAT_PROBLEM_MAP;
  MonitorElement* HO_DATAFORMAT_PROBLEM_ZOO;
   
   //MEs for hcalunpacker report info
   MonitorElement* meSpigotFormatErrors_;
   MonitorElement* meBadQualityDigis_;
   MonitorElement* meUnmappedDigis_;
   MonitorElement* meUnmappedTPDigis_;
   MonitorElement* meFEDerrorMap_;

   MonitorElement* meFEDRawDataSizes_;
   //Check that evt numbers are synchronized across all HTRs
   MonitorElement* meEvtNumberSynch_;
   MonitorElement* meBCNSynch_;
   MonitorElement* meBCN_;
   MonitorElement* medccBCN_;

   MonitorElement* meInvHTRData_;
   MonitorElement* meBCNCheck_; // htr BCN compared to dcc BCN
   MonitorElement* meEvtNCheck_; // htr Evt # compared to dcc Evt #
   MonitorElement* meFibBCN_;

   MonitorElement* meFWVersion_;
   MonitorElement* meEvFragSize_;
   MonitorElement* meEvFragSize2_;

   MonitorElement* meErrWdCrate_;  //HTR error bits by crate

   // The following MEs map specific conditons from the EventFragment headers as specified in
   //   http://cmsdoc.cern.ch/cms/HCAL/document/CountingHouse/DCC/DCC_1Jul06.pdf

   MonitorElement* meFEDId_;               //All of HCAL, as a stupidcheck.
   MonitorElement* meCDFErrorFound_;       //Summary histo of Common Data Format violations by FED ID
   MonitorElement* meDCCEventFormatError_; //Summary histo of DCC Event Format violations by FED ID 
   //Summary histo for HTR Status bits, DCC Error&Warn Counters Flagged Nonzero
   MonitorElement* meDCCErrorAndWarnConditions_;  
   MonitorElement* meDCCStatusFlags_;
   MonitorElement* meDCCSummariesOfHTRs_;  //Summary histo of HTR Summaries from DCC

   // The following MEs map specific conditons from the HTR/DCC headers as specified in
   //   http://cmsdoc.cern.ch/cms/HCAL/document/CountingHouse/HTR/design/Rev4MainFPGA.pdf

     MonitorElement* meCrate0HTRErr_;   //Map of HTR errors into Crate 0
     MonitorElement* meCrate1HTRErr_;   //Map of HTR errors into Crate 1
     MonitorElement* meCrate2HTRErr_;   //Map of HTR errors into Crate 2
     MonitorElement* meCrate3HTRErr_;   //Map of HTR errors into Crate 3
     MonitorElement* meCrate4HTRErr_;   //Map of HTR errors into Crate 4
     MonitorElement* meCrate5HTRErr_;   //Map of HTR errors into Crate 5
     MonitorElement* meCrate6HTRErr_;   //Map of HTR errors into Crate 6
     MonitorElement* meCrate7HTRErr_;   //Map of HTR errors into Crate 7
     MonitorElement* meCrate8HTRErr_;   //Map of HTR errors into Crate 8
     MonitorElement* meCrate9HTRErr_;   //Map of HTR errors into Crate 9
     MonitorElement* meCrate10HTRErr_;   //Map of HTR errors into Crate 10
     MonitorElement* meCrate11HTRErr_;   //Map of HTR errors into Crate 11
     MonitorElement* meCrate12HTRErr_;   //Map of HTR errors into Crate 12
     MonitorElement* meCrate13HTRErr_;   //Map of HTR errors into Crate 13
     MonitorElement* meCrate14HTRErr_;   //Map of HTR errors into Crate 14
     MonitorElement* meCrate15HTRErr_;   //Map of HTR errors into Crate 15
     MonitorElement* meCrate16HTRErr_;   //Map of HTR errors into Crate 16
     MonitorElement* meCrate17HTRErr_;   //Map of HTR errors into Crate 17

     MonitorElement* meFib1OrbMsgBCN_;  //BCN of Fiber 1 Orb Msg
     MonitorElement* meFib2OrbMsgBCN_;  //BCN of Fiber 2 Orb Msg
     MonitorElement* meFib3OrbMsgBCN_;  //BCN of Fiber 3 Orb Msg
     MonitorElement* meFib4OrbMsgBCN_;  //BCN of Fiber 4 Orb Msg
     MonitorElement* meFib5OrbMsgBCN_;  //BCN of Fiber 5 Orb Msg
     MonitorElement* meFib6OrbMsgBCN_;  //BCN of Fiber 6 Orb Msg
     MonitorElement* meFib7OrbMsgBCN_;  //BCN of Fiber 7 Orb Msg
     MonitorElement* meFib8OrbMsgBCN_;  //BCN of Fiber 8 Orb Msg

     MonitorElement* DCC_ErrWd_HBHE;
     MonitorElement* DCC_ErrWd_HF;
     MonitorElement* DCC_ErrWd_HO;

     //Member variables for reference values to be used in consistency checks.
     std::map<int, short> CDFversionNumber_list;
     std::map<int, short>::iterator CDFvers_it;
     std::map<int, short> CDFEventType_list;
     std::map<int, short>::iterator CDFEvT_it;
     std::map<int, short> CDFReservedBits_list;
     std::map<int, short>::iterator CDFReservedBits_it;
     std::map<int, short> DCCEvtFormat_list;
     std::map<int, short>::iterator DCCEvtFormat_it;
     std::map<int, short> DCCRsvdBits_list;
     std::map<int, short>::iterator DCCRsvdBits_it;
};

#endif
