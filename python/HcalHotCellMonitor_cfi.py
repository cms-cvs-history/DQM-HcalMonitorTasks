import FWCore.ParameterSet.Config as cms

hcalHotCellMonitor=cms.EDAnalyzer("HcalHotCellMonitor",
                                  # base class stuff
                                  debug                  = cms.int32(0),
                                  online                 = cms.bool(False),
                                  AllowedCalibTypes      = cms.vint32(0), # by default, don't include calibration events
                                  mergeRuns              = cms.bool(False),
                                  enableCleanup          = cms.bool(False),
                                  subSystemFolder        = cms.string("Hcal/"),
                                  TaskFolder             = cms.string("HotCellMonitor_Hcal/"),
                                  skipOutOfOrderLS       = cms.bool(True),
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
                                  minErrorFlag    = cms.untracked.double(1.), # fraction of a lumi section for which a channel must be above threshold to be considered a problem in LS plots
                                  energyThreshold = cms.double(10.),
                                  energyThreshold_HF = cms.untracked.double(20.),
                                  # other subdetector thresholds are also untracked

                                  persistentThreshold = cms.double(6.),
                                  persistentThreshold_HF = cms.untracked.double(10.),

                                  )
