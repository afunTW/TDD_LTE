#ifndef _DRX_H
#define _DRX_H
#include "LTE_TypeDefine.h"


// for DRX parameter
#define On_Duration        1       // On duration is 1 ms.
#define Inactivity_Timer   10      // Inactivity timer is 10 ms.
#define Short_Cycle_Timer  2       // # of short DRX cycle
#define Short_DRX_Cycle    40      // Short DRX cycle is 40 ms.
#define Long_DRX_Cycle     160     // Long DRX cycle is 160 ms.
#define shrt_slp           Short_DRX_Cycle - On_Duration  // sleep time in Short_DRX_Cycle
#define lng_slp            Long_DRX_Cycle - On_Duration   // sleep time in Long_DRX_Cycle



void DRX_initial(UE_Status * status, int UE_num);
void Check_Status(deque<UE> &ue, UE_Status * status, Subframe * rf);
#endif