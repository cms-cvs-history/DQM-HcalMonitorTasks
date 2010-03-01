import FWCore.ParameterSet.Config as cms

hcalHotCellMonitor=cms.EDAnalyzer("HcalHotCellMonitor",
                                  # base class stuff
                                  debug                  = cms.untracked.int32(0),
                                  online                 = cms.untracked.bool(False),
                                  AllowedCalibTypes      = cms.untracked.vint32(0), # by default, don't include calibration events
                                  mergeRuns              = cms.untracked.bool(False),
                                  enableCleanup          = cms.untracked.bool(False),
                                  subSystemFolder        = cms.untracked.string("Hcal/"),
                                  TaskFolder             = cms.untracked.string("HotCellMonitor_Hcal/"),
                                  skipOutOfOrderLS       = cms.untracked.bool(True),
                                  NLumiBlocks            = cms.untracked.int32(4000),
                                  
                                  # Hot Cell Monitor-specific Info
                                  
                                  # Input collections
                                  hbheRechitLabel              = cms.untracked.InputTag("hbhereco"),
                                  hoRechitLabel                = cms.untracked.InputTag("horeco"),
                                  hfRechitLabel                = cms.untracked.InputTag("hfreco"),

                                  # Booleans for various tests
                                  test_energy     = cms.untracked.bool(True),
                                  test_persistent = cms.untracked.bool(True),
                                  test_neighbor   = cms.untracked.bool(False),

                                  # Threshold requirements
                                  minEvents       = cms.untracked.int32(200),
                                  minErrorFlag    = cms.untracked.double(1.), # fraction of a lumi section for which a channel must be above threshold to be considered a problem in LS plots
                                  energyThreshold = cms.untracked.double(10.),
                                  energyThreshold_HF = cms.untracked.double(20.),
                                  # other subdetector thresholds are also untracked

                                  persistentThreshold = cms.untracked.double(6.),
                                  persistentThreshold_HF = cms.untracked.double(10.),

                                  )
