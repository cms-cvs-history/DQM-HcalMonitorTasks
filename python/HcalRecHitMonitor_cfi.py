import FWCore.ParameterSet.Config as cms

hcalRecHitMonitor=cms.EDAnalyzer("HcalRecHitMonitor",
                                 # base class stuff
                                 debug                  = cms.int32(0),
                                 online                 = cms.bool(False),
                                 AllowedCalibTypes      = cms.vint32(0),  # by default, don't include calibration events
                                 mergeRuns              = cms.bool(False),
                                 enableCleanup          = cms.bool(False),
                                 subSystemFolder        = cms.string("Hcal/"),
                                 TaskFolder             = cms.string("RecHitMonitor_Hcal/"),
                                 skipOutOfOrderLS       = cms.bool(False),
                                 NLumiBlocks            = cms.int32(4000),

                                 # variables specific to HcalRecHitMonitor
                                 
                                 # Input collections
                                 hbheRechitLabel              = cms.InputTag("hbhereco"),
                                 hoRechitLabel                = cms.InputTag("horeco"),
                                 hfRechitLabel                = cms.InputTag("hfreco"),
                                 
                                 L1GTLabel                    = cms.InputTag("l1GtUnpack"),

                                 L1TriggerBits                = cms.vint32(8,9,10), # passes L1 if any of the bits are true
                                 BPTXBits                     = cms.vint32(4), # passes BPTX if any of the bits are true

                                 energyThreshold              = cms.double(2.),
                                 ETThreshold                  = cms.double(0.),
                                 collisionHFthresh            = cms.double(3.),
                                 collisionHFETthresh          = cms.double(3.),
                                 collisionHEthresh            = cms.double(3.),
                                 collisionHEETthresh          = cms.double(3.),
                                 collisiontimediffThresh     = cms.double(10.),
                                 )
