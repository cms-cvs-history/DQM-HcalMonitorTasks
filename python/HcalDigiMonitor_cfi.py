import FWCore.ParameterSet.Config as cms

hcalDigiMonitor=cms.EDAnalyzer("HcalDigiMonitor",
                                 # base class stuff
                                 debug                  = cms.int32(0),
                                 online                 = cms.bool(False),
                                 AllowedCalibTypes      = cms.vint32(0), # by default, don't include calibratin events
                                 mergeRuns              = cms.bool(False),
                                 enableCleanup          = cms.bool(False),
                                 subSystemFolder        = cms.string("Hcal/"),
                                 TaskFolder             = cms.string("DigiMonitor_Hcal/"),
                                 skipOutOfOrderLS       = cms.bool(False),
                                 NLumiBlocks            = cms.int32(4000),
                                 
                                 # Digi Monitor Info
                                 digiLabel              = cms.InputTag("hcalDigis"),
                                 shapeThresh            = cms.int32(50),
                                 shapeThreshHB          = cms.int32(50),
                                 shapeThreshHE          = cms.int32(50),
                                 shapeThreshHO          = cms.int32(50),
                                 shapeThreshHF          = cms.int32(50),
                                 
                                 # problem checks
                                 checkForMissingDigis   = cms.bool(False),
                                 checkCapID             = cms.bool(True),
                                 checkDigiSize          = cms.bool(True),
                                 checkADCsum            = cms.bool(True),
                                 checkDVerr             = cms.bool(True),
                                 minDigiSize            = cms.int32(10),
                                 maxDigiSize            = cms.int32(10),
                                 
                                 # block orbit test
                                 shutOffOrbitTest       = cms.bool(False),
                                 ExpectedOrbitMessageTime = cms.int32(3559)
                                 )
