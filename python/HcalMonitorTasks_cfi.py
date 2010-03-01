import FWCore.ParameterSet.Config as cms

from DQM.HcalMonitorTasks.HcalDigiMonitor_cfi          import *
from DQM.HcalMonitorTasks.HcalDataIntegrityTask_cfi    import *
from DQM.HcalMonitorTasks.HcalHotCellMonitor_cfi       import *
from DQM.HcalMonitorTasks.HcalDeadCellMonitor_cfi      import *
from DQM.HcalMonitorTasks.HcalRecHitMonitor_cfi        import *
from DQM.HcalMonitorTasks.HcalNZSMonitor_cfi           import *
from DQM.HcalMonitorTasks.HcalBeamMonitor_cfi           import *



hcalMonitorTasksDefaultSequence=cms.Sequence(hcalDigiMonitor
                                             *hcalHotCellMonitor
                                             *hcalDeadCellMonitor
                                             *hcalRecHitMonitor
                                             *hcalBeamMonitor
                                             )

hcalMonitorTasksOnlineSequence = cms.Sequence(hcalDigiMonitor
                                              *hcalHotCellMonitor
                                              *hcalDeadCellMonitor
                                              *hcalRecHitMonitor
                                              *hcalBeamMonitor
                                              *hcalNZSMonitor # add DetDiag in online only?  Noise monitor in offline?  Check
                                              )


hcalMonitorTasksOfflineSequence = cms.Sequence(hcalDigiMonitor
                                               *hcalHotCellMonitor
                                               *hcalDeadCellMonitor
                                               *hcalRecHitMonitor
                                               *hcalBeamMonitor
                                               )
                                              

