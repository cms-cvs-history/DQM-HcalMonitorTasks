import FWCore.ParameterSet.Config as cms

hcalDataIntegrityMonitor = cms.EDAnalyzer("HcalDataIntegrityTask",
                                          # base class stuff
                                          debug                  = cms.int32(0),
                                          online                 = cms.bool(False),
                                          AllowedCalibTypes      = cms.vint32(),
                                          mergeRuns              = cms.bool(False),
                                          enableCleanup          = cms.bool(False),
                                          subSystemFolder        = cms.string("Hcal/"),
                                          TaskFolder             = cms.string("HcalDataIntegrityTask/"),
                                          skipOutOfOrderLS       = cms.bool(False),
                                          NLumiBlocks            = cms.int32(4000),
                                          
                                          # task-specific stuff
                                   
                                          RawDataLabel           = cms.InputTag("source"),
                                          UnpackerReportLabel    = cms.InputTag("hcalDigis")
                                          )
