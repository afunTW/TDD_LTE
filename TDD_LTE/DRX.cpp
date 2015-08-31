#include "stdafx.h"
#include "DRX.h"

int UE_number = 0;
void DRX_initial(UE_Status * status, int UE_num)
{
	for(int index = 0; index < UE_num; index++)
	{
		status[index].next_status = IN;
		status[index].now_status = IN;
		status[index].shrt_t = Short_Cycle_Timer;
		status[index].timer = Inactivity_Timer;
	}
	UE_number = UE_num;
}
/*
 * function name: Check_Status
 * brief: To decide next DRX status and DRX parameter for UE.
 * param: ue,address of UE's deque  
 * param: UE_num,number of UE
 * return: none
 */
void Check_Status(deque<UE> &ue, UE_Status * status, Subframe * rf)
{
	//rf->UE_num.clear();
	for(int index = 0; index < UE_number; index++)
	{
		status[index].now_status = status[index].next_status;
		switch(status[index].now_status)
		{
			case IN:
				if(ue[index].pkt_buffer[D] > 0 || ue[index].pkt_buffer[U] > 0)
					status[index].timer = Inactivity_Timer;
				if(--status[index].timer <= 0)
				{
					status[index].next_status = Slp;
					status[index].shrt_t = Short_Cycle_Timer;
					status[index].timer = shrt_slp;
				}
				break;
			case ON:
				if(ue[index].pkt_buffer[D] > 0 || ue[index].pkt_buffer[U] > 0)
				{
					status[index].next_status = IN;
					status[index].timer = Inactivity_Timer;
				}
				else 
				{
					status[index].next_status = Slp;
					if(status[index].shrt_t > 0)
					{
						status[index].timer = shrt_slp;
						status[index].shrt_t--;
					}
					else
						status[index].timer = lng_slp;
				}
				break;
			case Slp:
				if(ue[index].pkt_buffer[U] > 0)
				{
					status[index].now_status = IN;
					status[index].next_status = IN;
					status[index].timer = Inactivity_Timer;
				}
				else
				{
					if(--status[index].timer <= 0)
						status[index].next_status = ON;
				}
				break;
			default:
				printf("error\n");
				while(1);
				break;
		}
		
		if(status[index].now_status != Slp)
			rf->UE_num.push_back(index);
	}
}