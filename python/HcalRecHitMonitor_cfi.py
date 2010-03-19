import FWCore.ParameterSet.Config as cms

hcalRecHitMonitor=cms.EDAnalyzer("HcalRecHitMonitor",
                                 # base class stuff
                                 debug                  = cms.untracked.int32(0),
                                 online                 = cms.untracked.bool(False),
                                 AllowedCalibTypes      = cms.untracked.vint32(0),  # by default, don't include calibration events
                                 mergeRuns              = cms.untracked.bool(False),
                                 enableCleanup          = cms.untracked.bool(False),
                                 subSystemFolder        = cms.untracked.string("Hcal/"),
                                 TaskFolder             = cms.untracked.string("RecHitMonitor_Hcal/"),
                                 skipOutOfOrderLS       = cms.untracked.bool(False),
                                 NLumiBlocks            = cms.untracked.int32(4000),
                                 makeDiagnostics        = cms.untracked.bool(False),
                                 
                                 # variables specific to HcalRecHitMonitor
                                 
                                 # Input collections
                                 hbheRechitLabel              = cms.untracked.InputTag("hbhereco"),
                                 hoRechitLabel                = cms.untracked.InputTag("horeco"),
                                 hfRechitLabel                = cms.untracked.InputTag("hfreco"),
                                 
                                 L1GTLabel                    = cms.untracked.InputTag("l1GtUnpack"),

                                 L1TriggerBits                = cms.untracked.vint32(9,10), # passes L1 if any of the bits are true; negative values will force the check to be always true
                                 BPTXBits                     = cms.untracked.vint32(0), # passes BPTX if any of the bits are true; negative values will force the check to be always true 

                                 # Energy thresholds for some BPTX plots
                                 energyThreshold              = cms.untracked.double(2.),
                                 ETThreshold                  = cms.untracked.double(0.),
                                 HF_energyThreshold           = cms.untracked.double(3.),
                                 HF_ETThreshold               = cms.untracked.double(0.),
                                 HO_energyThreshold           = cms.untracked.double(5.),
                                 collisiontimediffThresh      = cms.untracked.double(10.) # max time diff between HF+, HF- weighted times for some plot filling
                                 )
