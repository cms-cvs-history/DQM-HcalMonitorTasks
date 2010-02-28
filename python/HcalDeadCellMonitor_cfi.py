import FWCore.ParameterSet.Config as cms

hcalDeadCellMonitor=cms.EDAnalyzer("HcalDeadCellMonitor",
                                   # base class stuff
                                   debug                  = cms.int32(0),
                                   online                 = cms.bool(False),
                                   AllowedCalibTypes      = cms.vint32(),
                                   mergeRuns              = cms.bool(False),
                                   enableCleanup          = cms.bool(False),
                                   subSystemFolder        = cms.string("Hcal/"),
                                   TaskFolder             = cms.string("DeadCellMonitor_Hcal/"),
                                   skipOutOfOrderLS       = cms.bool(True),
                                   NLumiBlocks            = cms.int32(4000),
                                   
                                   # Dead Cell Monitor-specific Info
                                   
                                   # Input collections
                                   hbheRechitLabel        = cms.InputTag("hbhereco"),
                                   hoRechitLabel          = cms.InputTag("horeco"),
                                   hfRechitLabel          = cms.InputTag("hfreco"),
                                   digiLabel              = cms.InputTag("hcalDigis"),
                                   # minimum number of events necessary for lumi-block-based checking to commence
                                   minDeadEventCount      = cms.int32(2000),

                                   #booleans for dead cell tests
                                   test_digis             = cms.bool(True), # test for recent missing digis
                                   test_rechits           = cms.bool(False), # test for missing rechits
                                   MissingRechitEnergyThreshold = cms.double(-1.)

                                   )
