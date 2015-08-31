// TDD_LTE.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LBPS.h"
#include "TDD_Map.h"
#include "DRX.h"
#include <algorithm>
#include <time.h>
#include <fstream>
#include <iostream>
/*
 * Global value:
 * ue : total UE
 * group : total group for aggr, split or merge
 * radioframe[10] : a radioframe which are 10 subframe and there are some group in each subframe for virtual mapping to real
 * virtualframe[2] : number of virtual subframe of for aggr, split or merge is k, and each one has a group.
 *                   virtualframe[0] and virtualframe[1] are in DL and UL,respectively.
 * is_load[2] : There are load or not for DL and UL.
 * system_capacity[2] : system capacity for DL and UL in a realsubframe
 * virtual_capacity[2] : system capacity for DL and UL in a virtualsubframe
 * tmp_sc : rest of capacity which is used by packets
 * u_len : number of UE awake in a realsubframe
 * p_len : number of packet in a UE
 * ue_num : UE's number in a realsubframe
 * R_UE_num[2] : number of UE which is real traffic for DL or UL
 * TD_offset : mapping offset with different configurations and mapping methods
 * system_load[2] : system load for DL and UL
 * tmp_pkt : temp packet
 * drt : direction for a realsubframe configured by TD-configuration
 * tmpRF[2] : Stroed realsubframes which didn't complete for DL and UL
 * tmpRF_len[2] : number of tmpRF for DL and UL
 * CompletedRFSize[2] : number of tmpRF which is completed in a subframe
 */

deque<UE> ue;
deque<Group> group;
Subframe radioframe[10];
deque<Group> virtualframe[2];
bool is_load[2] = {false,false};
int system_capacity[2] = {0,0}, virtual_capacity[2] = {0,0},tmp_sc, u_len, p_len, ue_num;
short R_UE_num[2] = {0,0};
short TD_offset = 0;
double system_load[2] = {0,0};
Packet tmp_pkt;
SF_Config drt;
deque <Subframe> tmpRF[2];
int tmpRF_len[2] = {0,0};
int CompletedRFSize[2] = {0,0};

/*
 * function name: compare_pkt
 * brief: UE's deque is sorted by this function with pkt_num.
 * param: ue1
 * param: ue2
 * return: sort result
 */
bool compare_pkt(Sort_UE ue1, Sort_UE ue2)
{
	return ue1.pkt_num < ue2.pkt_num;
}

/*
 * function name: RT_compare_pkt
 * brief: UE's deque is sorted by this function with delay budget. This is not used.
 * param: ue1
 * param: ue2
 * return: sort result
 */
bool RT_compare_pkt(Sort_UE ue1, Sort_UE ue2)
{
		return ue1.delay_budget < ue2.delay_budget;
}

/*
 * function name: sort_ue
 * brief: Sorting UEs awaked in the realsubframe.
 * param: ue_len,number of UE awake in a realsubframe
 * param: drt,direction for a realsubframe configured by TD-configuration
 * param: rf,realsubframe
 * return: none
 */
void sort_ue(int * ue_len, SF_Config * drt, Subframe * rf)
{
	*ue_len = rf->UE_num.size();
	*drt  = rf->drt;
	Sort_UE tmp_sort_ue;
	deque<Sort_UE> sort_ue;
	for(int i = 0; i < *ue_len; i++)
	{
		tmp_sort_ue.ue_num = rf->UE_num[i];
		tmp_sort_ue.pkt_num = ue[tmp_sort_ue.ue_num].pkt_num[*drt];
		/*** for RT and NRT -> This is not used. ***/
		//if(ue[tmp_sort_ue.ue_num].delay_budget[*drt] > -1)
		//	tmp_sort_ue.delay_budget = ue[tmp_sort_ue.ue_num].delay_budget[*drt];
		//else
		//	tmp_sort_ue.delay_budget = SHRT_MAX;
		/*********************/
		sort_ue.push_back(tmp_sort_ue);
	}
	sort(sort_ue.begin(),sort_ue.end(),compare_pkt/*RT_compare_pkt*/);

	rf->UE_num.clear();
	for(int i = 0; i < *ue_len; i++)
		rf->UE_num.push_back(sort_ue[i].ue_num);
}

/*
 * function name: sort_ue2
 * brief: Sorting UEs awaked in the realsubframe.(For FDD project, but this is not used now.)
 * param: ue_len,number of UE awake in a realsubframe
 * param: drt,direction for a realsubframe configured by TD-configuration
 * param: rf,realsubframe
 * param: tmp_ue,address of tmp_ue which didn't completed
 * return: none
 */
void sort_ue2(int * ue_len, SF_Config * drt, Subframe * rf, deque<int> &tmp_ue)
{
	*ue_len = rf->UE_num.size();
	*drt  = rf->drt;
	Sort_UE tmp_sort_ue;
	deque<Sort_UE> sort_ue;
	for(int i = 0; i < *ue_len; i++)
	{
		tmp_sort_ue.ue_num = rf->UE_num[i];
		tmp_sort_ue.pkt_num = ue[tmp_sort_ue.ue_num].pkt_num[*drt];
		sort_ue.push_back(tmp_sort_ue);
	}
	sort(sort_ue.begin(),sort_ue.end(),compare_pkt);

	rf->UE_num.clear();
	int tmp_ue_len = tmp_ue.size();
	for(int i = 0; i < tmp_ue_len; i++)
		rf->UE_num.push_back(tmp_ue[i]);
	bool is_duplicate = false;
	for(int i = 0; i < *ue_len; i++)
	{
		is_duplicate = false;
		for(int j = 0; j < tmp_ue_len; j++)
			if(sort_ue[i].ue_num == tmp_ue[j])
			{
				is_duplicate  = true;
				break;
			}
		if(!is_duplicate)
			rf->UE_num.push_back(sort_ue[i].ue_num);
	}
	*ue_len = rf->UE_num.size();
	tmp_ue.clear();
}

/*
 * function name: system_initial
 * brief: To initialize some parameters for different simulations.
 * param: none
 * return: none
 */
void system_initial(void)
{
	ue.clear();
	group.clear();
	for(int i = 0; i < 2; i++)
	{
		virtualframe[i].clear();
		is_load[i] = false;
		system_capacity[i] = 0;
		virtual_capacity[i] = 0;
		system_load[i] = 0.0;
		tmpRF[i].clear();
	}
	
}

/*
 * function name: TDD_simulation
 * brief: LBPS simulation for TDD environment.
 * param: none
 * return: none
 */
void TDD_simulation(void)
{
	srand((unsigned)time(NULL));
#ifdef RandomWayPoint
	Read_RWP_CQI();
#endif
	for(int ntime = 0; ntime < 11; ntime++)
	{
		int runtime = 0;
		double tmp_delay = 0.0;
		system_initial();
		Get_offset(&TD_offset);
		Configure_Load(is_load,R_UE_num);
		UE_initial(ue,system_load);
		while(runtime < RunTime)
		{printf("*");
			ue_10ms_cqi(ue);
			system_10ms_capacity(ue,system_capacity,system_load);
			virtual_10ms_capacity(system_capacity,virtual_capacity);
			while(virtualframe[DL].size() < 10 && is_load[DL])
			{
				if(R_UE_num[DL] > 0)
				    Split_R(ue,virtualframe,virtual_capacity,DL,&TD_offset,R_UE_num[DL],group);
				else
					Aggr(ue,virtualframe,virtual_capacity,DL,group);
			}
			while(virtualframe[UL].size() < 10 && is_load[UL])
			{
				if(R_UE_num[UL] > 0)
					Split_R(ue,virtualframe,virtual_capacity,DL,&TD_offset,R_UE_num[UL],group);
				else
					Split(ue,virtualframe,virtual_capacity,UL,group);
			}
			Mapping3(virtualframe,radioframe);
			for(int i = 0; i < 10; i++)
			{
				Discard_Pkt(ue,runtime+i);
				 //transmit packet of ue in a subframe for DL or UL
				/*u_len = radioframe[i].UE_num.size();*/
				drt = radioframe[i].drt;
				/*sort_ue(&u_len,&drt,&radioframe[i]);*/
				if(drt == S)
				{
					//create packet of UE in a subframe
					/*UE_1ms_pkt(ue,system_load,runtime+i);*/
					Create_UE_pkt(ue,runtime+i);
					continue;
				}
				bool is_send[UE_number] = {false};
				tmpRF[drt].push_back(radioframe[i]);
				tmpRF_len[drt] = tmpRF[drt].size();
				tmp_sc = system_capacity[drt];
				int send_packet = 0;

				for(int index = 0; index < tmpRF_len[drt]; index++)
				{
					sort_ue(&u_len,&drt,&tmpRF[drt][index]);
					for(int j = 0; j < u_len; j++)
					{
						/*ue_num = radioframe[i].UE_num[j];*/
						ue_num = tmpRF[drt][index].UE_num[j];
						if(is_send[ue_num])
							continue;
						ue[ue_num].awake++;
						is_send[ue_num] = true;
						if(tmp_sc <= 0)
							continue;
						/*p_len = ue[ue_num].pkt[drt].size();*/
						p_len = ue[ue_num].pkt_buffer[drt];
						for(int l = 0; l < p_len; l++)
						{
							tmp_pkt = ue[ue_num].pkt[drt][0];
							tmp_delay = (runtime + i) - tmp_pkt.arr_time;
							//if(ue[ue_num].delay_budget[drt] > 0 && (tmp_delay - ue[ue_num].delay_budget[drt]) > 0)
							//{
							//	//printf("%d\n",ue_num);
							//	ue[ue_num].pkt_buffer[drt]--;
							//	ue[ue_num].pkt[drt].pop_front();
							//	ue[ue_num].discard_pkt[drt]++;
							//	continue;
							//}
							if(tmp_pkt.pkt_size <= tmp_sc)
							{
								tmp_sc -= tmp_pkt.pkt_size;
								ue[ue_num].delay[drt] += tmp_delay; 
								/*ue[ue_num].pkt[drt].erase(ue[ue_num].pkt[drt].begin());*/
								ue[ue_num].pkt_buffer[drt]--;
								ue[ue_num].pkt[drt].pop_front();
								ue[ue_num].pkt_num[drt]++;
								send_packet++;
							}
							else
							{
								ue[ue_num].pkt[drt][0].pkt_size -= tmp_sc;
								tmp_sc = -1;
								break;
							}
							/*tmp_sc -= ue[ue_num].pkt[drt][l].pkt_size;
							if(tmp_sc >= 0)
							{
								ue[ue_num].delay += (runtime + i) - ue[ue_num].pkt[drt][l].arr_time;
								ue[ue_num].pkt_num++;
							}
							else
							{
								ue[ue_num].pkt[drt][l].pkt_size = -tmp_sc;
								ue[ue_num].pkt[drt].erase(ue[ue_num].pkt[drt].begin(),ue[ue_num].pkt[drt].begin()+l);
								break;
							}*/
						}
						/*if(tmp_sc >= 0)
							ue[ue_num].pkt[drt].clear();*/
					}
					if(tmp_sc >= 0)
						CompletedRFSize[drt]++;
				}
				for(int index = 0; index < CompletedRFSize[drt]; index++)
				{
					tmpRF[drt].pop_front();
				}
				CompletedRFSize[drt] = 0;
#ifdef RN_1st
			if(radioframe[i].is_RN || u_len > 0)
				ue[UE_number].awake++;
#endif
				
				 //create packet of UE in a subframe
				/*UE_1ms_pkt(ue,system_load,runtime+i);*/
				Create_UE_pkt(ue,runtime+i);
			}
			/*int packet_number = 0;
			for(int i = 0; i < UE_number; i++)
			{
				packet_number += ue[i].pkt[DL].size();
			}
			printf("packet number = %d\n",packet_number);*/
			runtime += 10;
		}
		Print_value(ue);
	}
}

/*
 * function name: FDD_simulation
 * brief: LBPS simulation for FDD environment.
 * param: none
 * return: none
 */
void FDD_simulation(void)
{
	Subframe RF;
	//deque<int> tmp_ue;
	//deque <Subframe> tmpRF;
	//int tmpRF_len = 0;
	//int CompletedRFSize = 0;
	srand((unsigned)time(NULL));
#ifdef RandomWayPoint
	Read_RWP_CQI();
#endif
	for(int ntime = 0; ntime < 10; ntime++)
	{
		int runtime = 0;

		system_initial();
		Configure_Load(is_load,R_UE_num);
		UE_initial(ue,system_load);
		while(runtime < RunTime)
		{printf("*");
			ue_10ms_cqi(ue);
			system_10ms_capacity(ue,system_capacity,system_load);
			virtual_10ms_capacity(system_capacity,virtual_capacity);
			for(int i = 0; i < 10; i++)
			{
				bool is_send[UE_number] = {false};
				 //transmit packet of ue in a subframe for DL or UL
				if(virtualframe[DL].size() <= 0/* && tmp_ue.size() <= 0*/)
				{
					/*for(int j = 0; j < UE_number; j++)
						ue[j].pkt[DL].clear();*/
					Split(ue,virtualframe,virtual_capacity,DL,group);
				}
				
				FD_mapping(virtualframe[DL],&RF);
				//sort_ue2(&u_len,&drt,&RF,tmp_ue);
				drt = RF.drt;
				tmpRF[drt].push_back(RF);
				tmpRF_len[drt] = tmpRF[drt].size();
				tmp_sc = system_capacity[drt];
			    
				for(int index = 0; index < tmpRF_len[drt]; index++)
				{
					sort_ue(&u_len,&drt,&tmpRF[drt][index]);
					for(int j = 0; j < u_len; j++)
					{
						//ue_num = RF.UE_num[j];
						ue_num = tmpRF[drt][index].UE_num[j];
						if(is_send[ue_num])
							continue;
						ue[ue_num].awake++;
						is_send[ue_num] = true;
						if(tmp_sc <= 0)
						{
							//tmp_ue.push_back(ue_num);
							continue;
						}
						//p_len = ue[ue_num].pkt[drt].size();
						p_len = ue[ue_num].pkt_buffer[drt];
						for(int l = 0; l < p_len; l++)
						{
							tmp_pkt = ue[ue_num].pkt[drt][0];
							if(tmp_pkt.pkt_size <= tmp_sc)
							{
								tmp_sc -= tmp_pkt.pkt_size;
								ue[ue_num].delay[drt] += (runtime + i) - tmp_pkt.arr_time; 
								/*ue[ue_num].pkt[drt].erase(ue[ue_num].pkt[drt].begin());*/
								ue[ue_num].pkt_buffer[drt]--;
								ue[ue_num].pkt[drt].pop_front();
								ue[ue_num].pkt_num[drt]++;
							}
							else
							{
								ue[ue_num].pkt[drt][0].pkt_size -= tmp_sc;
								tmp_sc = -1;
								//tmp_ue.push_back(ue_num);
								break;
							}
						}
					}

					if(tmp_sc >= 0)
						CompletedRFSize[drt]++;
				}
				for(int index = 0; index < CompletedRFSize[drt]; index++)
				{
					tmpRF[drt].pop_front();
				}
				CompletedRFSize[drt] = 0;
#ifdef RN_1st
			if(radioframe[i].is_RN || u_len > 0)
				ue[UE_number].awake++;
#endif
				 //create packet of UE in a subframe
				/*UE_1ms_pkt(ue,system_load,runtime+i);*/
				Create_UE_pkt(ue,runtime+i);
			}
			runtime += 10;
		}
		Print_value(ue);
	}
}

/*
 * function name: DRX_simulation
 * brief: DRX simulation for TDD(c0~6)/FDD(c7) environment.
 * param: none
 * return: none
 */
void DRX_simulation(void)
{
	srand((unsigned)time(NULL));
#ifdef RandomWayPoint
	Read_RWP_CQI();
#endif
	//Subframe RF;
	UE_Status ue_status[UE_number];
	for(int ntime = 0; ntime < 11; ntime++)
	{
		int runtime = 0;
		DRX_initial(ue_status,UE_number);
		system_initial();
		Configure_Load(is_load,R_UE_num);
		UE_initial(ue,system_load);
		while(runtime < RunTime)
		{printf("*");
			ue_10ms_cqi(ue);
			system_10ms_capacity(ue,system_capacity,system_load);
			DRX_Mapping(radioframe);
			for(int i = 0; i < 10; i++)
			{
				 //transmit packet of ue in a subframe for DL or UL
				/*u_len = radioframe[i].UE_num.size();*/
				Check_Status(ue,ue_status,&radioframe[i]);
				drt = radioframe[i].drt;
				//RF.drt = drt;
				/*sort_ue(&u_len,&drt,&radioframe[i]);*/
				if(drt == S)
				{
					//create packet of UE in a subframe
					/*UE_1ms_pkt(ue,system_load,runtime+i);*/
					u_len = radioframe[i].UE_num.size();
					for(int index = 0; index < u_len; index++)
						ue[index].awake++;
					Create_UE_pkt(ue,runtime+i);
					continue;
				}
				tmpRF[drt].push_back(radioframe[i]);
				tmpRF_len[drt] = tmpRF[drt].size();
				tmp_sc = system_capacity[drt];
				int send_packet = 0;

				for(int index = 0; index < tmpRF_len[drt]; index++)
				{
					sort_ue(&u_len,&drt,&tmpRF[drt][index]);
					for(int j = 0; j < u_len; j++)
					{
						/*ue_num = radioframe[i].UE_num[j];*/
						ue_num = tmpRF[drt][index].UE_num[j];;
						ue[ue_num].awake++;
						if(tmp_sc <= 0)
							continue;
						/*p_len = ue[ue_num].pkt[drt].size();*/
						p_len = ue[ue_num].pkt_buffer[drt];
						for(int l = 0; l < p_len; l++)
						{
							tmp_pkt = ue[ue_num].pkt[drt][0];
							if(tmp_pkt.pkt_size <= tmp_sc)
							{
								tmp_sc -= tmp_pkt.pkt_size;
								ue[ue_num].delay[drt] += (runtime + i) - tmp_pkt.arr_time; 
								/*ue[ue_num].pkt[drt].erase(ue[ue_num].pkt[drt].begin());*/
								ue[ue_num].pkt_buffer[drt]--;
								ue[ue_num].pkt[drt].pop_front();
								ue[ue_num].pkt_num[drt]++;
								send_packet++;
							}
							else
							{
								ue[ue_num].pkt[drt][0].pkt_size -= tmp_sc;
								tmp_sc = -1;
								break;
							}
						}

					}
					//if(tmp_sc >= 0)
					//	CompletedRFSize[drt]++;
				}
				//for(int index = 0; index < CompletedRFSize[drt]; index++)
				//{
					tmpRF[drt].pop_front();
				//}
				//CompletedRFSize[drt] = 0;
#ifdef RN_1st
			if(radioframe[i].is_RN || u_len > 0)
				ue[UE_number].awake++;
#endif
				 //create packet of UE in a subframe
				Create_UE_pkt(ue,runtime+i);
			}
			runtime += 10;
		}
		Print_value(ue);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	TDD_simulation();
	//FDD_simulation();
	//calculate_load();
	//DRX_simulation();
	return 0;
}

