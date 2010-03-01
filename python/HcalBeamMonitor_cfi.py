import FWCore.ParameterSet.Config as cms

hcalBeamMonitor=cms.EDAnalyzer("HcalBeamMonitor",
                               # base class stuff
                               debug                  = cms.int32(0),
                               online                 = cms.bool(False),
                               AllowedCalibTypes      = cms.vint32(),
                               mergeRuns              = cms.bool(False),
                               enableCleanup          = cms.bool(False),
                               subSystemFolder        = cms.string("Hcal/"),
                               TaskFolder             = cms.string("BeamMonitor_Hcal/"),
                               skipOutOfOrderLS       = cms.bool(True),
                               NLumiBlocks            = cms.int32(4000),
                               
                               # BeamMonitor-specific Info
                               
                               # Input collections
                               hbheRechitLabel        = cms.InputTag("hbhereco"),
                               hoRechitLabel          = cms.InputTag("horeco"),
                               hfRechitLabel          = cms.InputTag("hfreco"),
                               digiLabel              = cms.InputTag("hcalDigis"),

                               minEvents              = cms.int32(500),
                               lumiqualitydir         = cms.string("")
                               )
