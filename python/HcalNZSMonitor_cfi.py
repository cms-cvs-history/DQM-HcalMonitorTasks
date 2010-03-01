import FWCore.ParameterSet.Config as cms

hcalNZSMonitor=cms.EDAnalyzer("HcalNZSMonitor",
                              # base class stuff
                              debug                  = cms.int32(0),
                              online                 = cms.bool(False),
                              AllowedCalibTypes      = cms.vint32(0), # don't include calibration events, since they skew NZS ratio? 
                              mergeRuns              = cms.bool(False),
                              enableCleanup          = cms.bool(False),
                              subSystemFolder        = cms.string("Hcal/"),
                              TaskFolder             = cms.string("NZSMonitor_Hcal/"),
                              skipOutOfOrderLS       = cms.bool(False),
                              NLumiBlocks            = cms.int32(4000),

                              # NZS-specific parameters
                              RawDataLabel           = cms.InputTag("source"),
                            
                              HLTResultsLabel        = cms.InputTag("TriggerResults","","HLT"),
                              nzsHLTnames            = cms.vstring('HLT_HcalPhiSym',
                                                                   'HLT_HcalNZS_8E29'),
                              NZSeventPeriod         = cms.int32(4096),
                              )
