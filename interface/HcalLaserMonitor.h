#ifndef DQM_HCALMONITORTASKS_HCALLASERMONITOR_H
#define DQM_HCALMONITORTASKS_HCALLASERMONITOR_H

#include "DQM/HcalMonitorTasks/interface/HcalBaseMonitor.h"
#include "DataFormats/HcalDigi/interface/HcalUnpackerReport.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DataFormats/HcalDigi/interface/HcalLaserDigi.h"

/** \class HcalLaserMonitor
 *  
 * $Date: 2008/08/13 17:08:26 $
 * $Revision: 1.1 $
 * \author L. Lebolo - FIU
 */
static const float adc2fc_[128] = { -0.5, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5,
				    8.5,  9.5, 10.5, 11.5, 12.5, 13.5, 15., 17., 
				    19., 21., 23., 25., 27., 29.5,
				    32.5, 35.5, 38.5, 42., 46., 50., 54.5, 59.5, 
				    64.5, 59.5, 64.5, 69.5, 74.5,
				    79.5, 84.5, 89.5, 94.5, 99.5, 104.5, 109.5, 
				    114.5, 119.5, 124.5, 129.5, 137.,
				    147., 157., 167., 177., 187., 197., 209.5, 224.5, 
				    239.5, 254.5, 272., 292.,
				    312., 334.5, 359.5, 384.5, 359.5, 384.5, 409.5, 
				    434.5, 459.5, 484.5, 509.5,
				    534.5, 559.5, 584.5, 609.5, 634.5, 659.5, 684.5, 709.5, 
				    747., 797., 847.,
				    897.,  947., 997., 1047., 1109.5, 1184.5, 1259.5, 
				    1334.5, 1422., 1522., 1622.,
				    1734.5, 1859.5, 1984.5, 1859.5, 1984.5, 2109.5, 
				    2234.5, 2359.5, 2484.5,
				    2609.5, 2734.5, 2859.5, 2984.5, 3109.5, 
				    3234.5, 3359.5, 3484.5, 3609.5, 3797.,
				    4047., 4297., 4547., 4797., 5047., 5297., 
				    5609.5, 5984.5, 6359.5, 6734.5,
				    7172., 7672., 8172., 8734.5, 9359.5, 9984.5 };




class HcalLaserMonitor: public HcalBaseMonitor {

 public:
  HcalLaserMonitor(); 
  ~HcalLaserMonitor(); 

  void setup( const edm::ParameterSet&, DQMStore* );
  void processEvent( const HBHEDigiCollection&, const HODigiCollection&,
		     const HFDigiCollection&, const HcalLaserDigi&,
		     const HcalDbService& );
  void reset();
  void done();

 private:
  void perChanHists( const int, const HcalDetId, const float*, 
		     map<HcalDetId, MonitorElement*>&, 
		     map<HcalDetId, MonitorElement*>&, 
		     map<HcalDetId, MonitorElement*>&,
		     const string );

  map<HcalDetId, MonitorElement*>::iterator meIter_;

  bool doPerChannel_;
  double etaMax_, etaMin_, phiMax_, phiMin_;
  int etaBins_, phiBins_;
  
  int sigS0_, sigS1_;
  float adcThresh_;

  int ievt_;
  HcalCalibrations calibs_;

  MonitorElement* meEVT_;

  struct {
    map<HcalDetId,MonitorElement*> perChanShape_;
    map<HcalDetId,MonitorElement*> perChanTime_;
    map<HcalDetId,MonitorElement*> perChanEnergy_;

    MonitorElement* allShapePedSub_;
    //MonitorElement* allShape_;
    MonitorElement* allTime_;
    MonitorElement* allEnergy_;

    // These get filled in client...
    MonitorElement* rms_shape_;
    MonitorElement* mean_shape_;

    MonitorElement* rms_time_;
    MonitorElement* mean_time_;

    MonitorElement* rms_energy_;
    MonitorElement* mean_energy_;
  } hbHists, heHists, hfHists, hoHists;

  struct {
    MonitorElement* numChannels_;

    MonitorElement* trigger_;
    MonitorElement* rawOptosync_;
    MonitorElement* clockOptosync_;

    MonitorElement* rawOptosync_Trigger_;
  } TDCHists;

  MonitorElement* QADC_[32];

  // These get filled in client...
  MonitorElement* MEAN_MAP_TIME_L1_;
  MonitorElement* MEAN_MAP_TIME_L2_;
  MonitorElement* MEAN_MAP_TIME_L3_;
  MonitorElement* MEAN_MAP_TIME_L4_;
  MonitorElement*  RMS_MAP_TIME_L1_;
  MonitorElement*  RMS_MAP_TIME_L2_;
  MonitorElement*  RMS_MAP_TIME_L3_;
  MonitorElement*  RMS_MAP_TIME_L4_;

  MonitorElement* MEAN_MAP_SHAPE_L1_;
  MonitorElement* MEAN_MAP_SHAPE_L2_;
  MonitorElement* MEAN_MAP_SHAPE_L3_;
  MonitorElement* MEAN_MAP_SHAPE_L4_;
  MonitorElement*  RMS_MAP_SHAPE_L1_;
  MonitorElement*  RMS_MAP_SHAPE_L2_;
  MonitorElement*  RMS_MAP_SHAPE_L3_;
  MonitorElement*  RMS_MAP_SHAPE_L4_;

  MonitorElement* MEAN_MAP_ENERGY_L1_;
  MonitorElement* MEAN_MAP_ENERGY_L2_;
  MonitorElement* MEAN_MAP_ENERGY_L3_;
  MonitorElement* MEAN_MAP_ENERGY_L4_;
  MonitorElement*  RMS_MAP_ENERGY_L1_;
  MonitorElement*  RMS_MAP_ENERGY_L2_;
  MonitorElement*  RMS_MAP_ENERGY_L3_;
  MonitorElement*  RMS_MAP_ENERGY_L4_;
};

#endif