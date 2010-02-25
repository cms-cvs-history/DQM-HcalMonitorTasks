import FWCore.ParameterSet.Config as cms

hcalBaseDQMonitor=cms.EDAnalyzer("HcalBaseDQMonitor",
                                 debug                  = cms.int32(0),
                                 online                 = cms.bool(False),
                                 AllowedCalibTypes      = cms.vint32(),
                                 mergeRuns              = cms.bool(False),
                                 enableCleanup          = cms.bool(False),
                                 subSystemFolder        = cms.untracked.string("Hcal/"),
                                 TaskFolder             = cms.untracked.string("Test/"),
                                 skipOutOfOrderLS       = cms.bool(False),
                                 NLumiBlocks            = cms.int32(4000),
                                 )