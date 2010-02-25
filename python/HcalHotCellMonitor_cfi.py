import FWCore.ParameterSet.Config as cms

hcalHotCellMonitor=cms.EDAnalyzer("HcalHotCellMonitor",
                                  # base class stuff
                                  debug                  = cms.int32(0),
                                  online                 = cms.bool(False),
                                  AllowedCalibTypes      = cms.vint32(),
                                  mergeRuns              = cms.bool(False),
                                  enableCleanup          = cms.bool(False),
                                  subSystemFolder        = cms.untracked.string("Hcal/"),
                                  TaskFolder             = cms.untracked.string("HotCellMonitor_Hcal/"),
                                  skipOutOfOrderLS       = cms.bool(False),
                                  NLumiBlocks            = cms.int32(4000),
                                  
                                  # Hot Cell Monitor-specific Info
                                  
                                  # Input collections
                                  hbheRechitLabel              = cms.InputTag("hbhereco"),
                                  hoRechitLabel                = cms.InputTag("horeco"),
                                  hfRechitLabel                = cms.InputTag("hfreco"),

                                  # Booleans for various tests
                                  test_energy     = cms.bool(True),
                                  test_persistent = cms.bool(True),
                                  test_neighbor   = cms.bool(False),

                                  # Threshold requirements
                                  minEvents       = cms.int32(200),
                                  minErrorFlag    = cms.double(0.05),
                                  energyThreshold = cms.double(10.),
                                  energyThreshold_HF = cms.untracked.double(20.),
                                  # other subdetector thresholds are also untracked

                                  persistentThreshold = cms.double(6.),
                                  persistentThreshold_HF = cms.untracked.double(10.),

                                  )
