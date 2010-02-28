import FWCore.ParameterSet.Config as cms

from DQM.HcalMonitorTasks.HcalDigiMonitor_cfi          import *
from DQM.HcalMonitorTasks.HcalDataIntegrityTask_cfi    import *
from DQM.HcalMonitorTasks.HcalHotCellMonitor_cfi       import *
from DQM.HcalMonitorTasks.HcalDeadCellMonitor_cfi      import *
from DQM.HcalMonitorTasks.HcalRecHitMonitor_cfi        import *


hcalMonitorTasksDefaultSequence=cms.Sequence(hcalDigiMonitor*
                                             hcalHotCellMonitor*
                                             hcalDeadCellMonitor*
                                             hcalRecHitMonitor)
