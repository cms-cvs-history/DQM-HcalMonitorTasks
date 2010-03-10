import FWCore.ParameterSet.Config as cms

from DQM.HcalMonitorTasks.HcalDigiMonitor_cfi          import *
from DQM.HcalMonitorTasks.HcalDataIntegrityTask_cfi    import *
from DQM.HcalMonitorTasks.HcalHotCellMonitor_cfi       import *
from DQM.HcalMonitorTasks.HcalDeadCellMonitor_cfi      import *
from DQM.HcalMonitorTasks.HcalRecHitMonitor_cfi        import *
from DQM.HcalMonitorTasks.HcalNZSMonitor_cfi           import *
from DQM.HcalMonitorTasks.HcalBeamMonitor_cfi          import *
from DQM.HcalMonitorTasks.HcalRawDataMonitor_cfi       import *
from DQM.HcalMonitorTasks.HcalTrigPrimMonitor_cfi      import *
from DQM.HcalMonitorTasks.HcalDetDiagLaserMonitor_cfi  import *
from DQM.HcalMonitorTasks.HcalDetDiagPedestalMonitor_cfi import*
from DQM.HcalMonitorTasks.HcalDetDiagLEDMonitor_cfi import*
from DQM.HcalMonitorTasks.HcalDetDiagNoiseMonitor_cfi import*

hcalMonitorTasksDefaultSequence=cms.Sequence(hcalDigiMonitor
                                             *hcalHotCellMonitor
                                             *hcalDeadCellMonitor
                                             *hcalRecHitMonitor
                                             *hcalBeamMonitor
                                             *hcalRawDataMonitor
                                             *hcalTrigPrimMonitor
                                             )

hcalMonitorTasksOnlineSequence = cms.Sequence(hcalDigiMonitor
                                              *hcalHotCellMonitor
                                              *hcalDeadCellMonitor
                                              *hcalRecHitMonitor
                                              *hcalBeamMonitor
                                              *hcalRawDataMonitor
                                              *hcalTrigPrimMonitor
                                              *hcalDetDiagPedestalMonitor
                                              *hcalDetDiagLaserMonitor
                                              *hcalDetDiagLEDMonitor
                                              *hcalDetDiagNoiseMonitor
                                              *hcalNZSMonitor # add DetDiag in online only?  Noise monitor in offline?  Check
                                              )


hcalMonitorTasksOfflineSequence = cms.Sequence(hcalDigiMonitor
                                               *hcalHotCellMonitor
                                               *hcalDeadCellMonitor
                                               *hcalRecHitMonitor
                                               *hcalBeamMonitor
                                               *hcalRawDataMonitor
                                               )
                                              

