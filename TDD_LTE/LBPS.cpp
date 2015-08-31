#include "stdafx.h"
#include <time.h>
#include <math.h>
#include <algorithm>
#include "LBPS.h"
#include <random>
#include <fstream>
/*
 * Global value:
 * CQI_eff[16] : CQI efficiency table
 * DU_rnage[2] : range of UE for DL(DU_range[DL][]) and UL(DU_range[UL][]), and 
 *            the value(DU_range[][0 or 1]) is between 0 and UE_number
 * UE_load[2] : UE's load for DL(UE_load[DL]) and UL(UE_load[UL]), and
 *           mutiple values can be used
 * delay_range[2] : range of UE for different delay dubgets
 * delay_budget[2] : UE's delay budget for DL(UE_load[DL]) and UL(UE_load[UL]), and
 *                mutiple values can be used
 * ratio[2] : for random UE (This is not used.)
 * CQI_buffer : Buffer which is stored CQI for random way point project
 * create_pkt_sf : To store subframe number which should be created packet(This is not used.)
 */
double CQI_eff[16] = {0,0.1523,0.2344,0.3770,0.6016,0.8770,1.1758,1.4766,
	                  1.9141,2.4063,2.7305,3.3223,3.9023,4.5234,5.1152,5.5547};
deque<int> DU_range[2];
deque<double> UE_load[2];
deque<short> delay_range[2];
deque<short> delay_budget[2];
deque<int> ratio[2]; 
deque<char> CQI_buffer;
deque<int> create_pkt_sf;

/*
 * function name: UE_initial
 * brief: initial all of UE with DL and UL load, and calculate system load for DL and UL
 * param: ue,address of UE's deque 
 * param: DL_end,indicate final index of DL UE, DL_end is between 0 and UE_number(number of DL UE = DL_end)
 * param: UL_start,indicate initial index of UL UE, UL_start is between 0 and UE_number(number of UL UE = 
 *        UE_number - UL_start)
 * param: system_load,address of system_load which is global value
 * return: none
 */
void UE_initial(deque<UE> &ue, double * system_load)
{
	UE aUE = {{0,0},{0,0},{0,0},0,0,{0,0},{0,0},{0,0},{0,0},{0,0}};
	int len = 0;
	for(int index = 0; index < UE_number; index++)
		ue.push_back(aUE);
	for(int i = 0; i < 2; i++)
	{
		len = UE_load[i].size();
		for(int j = 0; j < len; j++)
		{
			for(int k = DU_range[i][j]; k < DU_range[D][j+1]; k++)
			{
				ue[k].load[i] = UE_load[i][j];
				system_load[i] += ue[k].load[i];
			}
		}
	}

	for(int i = 0; i < 2; i++)
	{
		len = delay_budget[i].size();
		for(int j = 0; j < len; j++)
		{
			for(int k = delay_range[i][j]; k < delay_range[D][j+1]; k++)
			{
				ue[k].delay_budget[i] = delay_budget[i][j];
			}
		}
	}
	//for(int index = DU_range[DL][0]; index < DU_range[DL][1]; index++)
	//{
	//	ue[index].load[DL] = UE_DL_load;
	//	system_load[DL] += ue[index].load[DL];
	//}
	//for(int index = DU_range[UL][0]; index < DU_range[UL][1]; index++)
	//{
	//	ue[index].load[UL] = UE_UL_load;
	//	system_load[UL] += ue[index].load[UL];
	//}
#ifdef RN_1st
	ue.push_back(aUE);
#endif
}

/*
 * function name: Aggr
 * brief: LBPS aggr
 * param: ue,address of UE's deque 
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: system_capacity, address of system_capacity which is a global parameter, and be array
 * param: det,direction for DL or UL
 * param: A_Group,address of group's deque for test
 * return: none
 */
void Aggr(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, deque<Group> &A_Group)
{
	int k, range_start = DU_range[drt].front(), range_end = DU_range[drt].back();
	int data_th = (int)(system_capacity[drt] * Prob_th) / Pkt_size;
	Group tmp_group = {false,0,0,0,0};
	deque<Group> tmp_vf;
	deque<Group> aGroup;
	aGroup.push_back(tmp_group);
	for(int index = range_start; index < range_end; index++)
	{
		//if(ue[index].load[drt] > 0)
		//{
			aGroup[0].load += ue[index].load[drt];
			aGroup[0].UE_num.push_back(index);
			//aGroup[0].exist_pkt_num += pkt_buffer[index].size();
			//aGroup[0].exist_pkt_num += buffer_len[index];
			//aGroup[0].exist_pkt_num += ue[index].pkt[drt].size();
			aGroup[0].exist_pkt_num += ue[index].pkt_buffer[drt];
		//}
	}
	aGroup[0].k = calculate_K(aGroup[0].load,data_th /*- aGroup[0].exist_pkt_num*/);
	modify_group(aGroup,A_Group);
	k = aGroup[0].k;
#ifdef RN_1st
	if(k < 2)
		k = 2;
	for(int index = 0; index < k; index++)
		tmp_vf.push_back(tmp_group);
	Group RN_group = {true,0,0,0,0};
	RN_group.UE_num.push_back(UE_number);
	tmp_vf[0] = RN_group;
	tmp_vf[1] = aGroup[0];
#else
	for(int index = 0; index < k; index++)
		tmp_vf.push_back(tmp_group);
	tmp_vf[k-1] = aGroup[0];
#endif
	do
	{
		virtualframe[drt].insert(virtualframe[drt].end(),tmp_vf.begin(),tmp_vf.end());
	}while(virtualframe[drt].size() < 10);
}

/*
 * function name: Split
 * brief: LBPS Split
 * param: ue,address of UE's deque 
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: system_capacity, address of system_capacity which is a global parameter, and be array
 * param: drt,direction for DL or UL
 * param: S_Group,address of group's deque for test
 * return: none
 */
void Split(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, deque<Group> &S_Group)
{
	int min_k = 1;
	int g_size_condition = UE_number / 2;
	int g_size = 1;
	int g_index = 0;
	int tmp_min_k = 1;
	int data_th = (int)(system_capacity[drt] * Prob_th) / Pkt_size;
	int range_start = DU_range[drt].front(), range_end = DU_range[drt].back();
	Group tmp_group = {false,0,0,0,0};
	deque<Group> aGroup;
	deque<Group> tmpGroup;
	deque<Group> tmp_vf;
	/*** split ***/
	do 
	{
#ifdef RN_1st
		tmpGroup.clear();
		tmpGroup = aGroup;
#endif
		if(tmp_min_k > g_size_condition)
			g_size = g_size_condition;
		else
			g_size = tmp_min_k;

		min_k = tmp_min_k;
		aGroup.clear();
		/*aGroup.resize(min_k);*/
		for(int index = 0; index < g_size; index++)
			aGroup.push_back(tmp_group);
		/*** insert ue to group***/
		for(int index = range_start; index < range_end; index++)
		{
			//if(ue[index].load[drt] > 0)
			//{
				g_index = index % g_size;
				aGroup[g_index].load += ue[index].load[drt];
				aGroup[g_index].UE_num.push_back(index);
				//aGroup[g_index].exist_pkt_num += ue[index].pkt[drt].size();
				aGroup[g_index].exist_pkt_num += ue[index].pkt_buffer[drt];
			//}
		}
		/*************************/
		/*** calculate k of group ***/
		tmp_min_k = INT_MAX;
		for(int index = 0; index < g_size; index++)
		{
			aGroup[index].k = calculate_K(aGroup[index].load,data_th/* - aGroup[index].exist_pkt_num*/);
			if(tmp_min_k > aGroup[index].k)
				tmp_min_k = aGroup[index].k;
		}
		/*if(tmp_min_k > UE_number / 2)
			g_size = UE_number / 2;*///tmp_min_k = UE_number / 2;//? UE_number / 2
		/****************************/
	}while((tmp_min_k != min_k) && (g_size != g_size_condition));
	/*************/
	
	for(int index = 0; index < tmp_min_k; index++)
		tmp_vf.push_back(tmp_group);

#ifdef RN_1st
	modify_group(tmpGroup,S_Group);
	Group RN_group = {true,0,0,0,0};
	RN_group.UE_num.push_back(UE_number);
	tmpGroup.insert(tmpGroup.begin(),&RN_group);
	virtualframe[drt].insert(virtualframe[drt].end(),tmpGroup.begin(),tmpGroup.end());
#else
	modify_group(aGroup,S_Group);
	for(int index = 1; index <= g_size; index++)
		tmp_vf[tmp_min_k-index] = aGroup[g_size - index];
	do
	{
		virtualframe[drt].insert(virtualframe[drt].end(),tmp_vf.begin(),tmp_vf.end());
	}while(virtualframe[drt].size() < 10);
#endif
}

/*
 * function name: Merge
 * brief: LBPS Merge
 * param: ue,address of UE's deque 
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: system_capacity, address of system_capacity which is a global parameter, and be array
 * param: drt,direction for DL or UL
 * param: S_Group,address of group's deque for test
 * return: none
 */
void Merge(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, deque<Group> &M_Group)
{
	int data_th = (int)(system_capacity[drt] * Prob_th/*60000*/) / Pkt_size;
	int range_start = DU_range[drt].front(), range_end = DU_range[drt].back();
	int range = range_end - range_start;
	int g_size,g_index,max_k;
	Group nd_group = {false,0,0,0,0},d_group = {false,0,0,0,0};
	Group tmp_group = {false,0,0,0,0};
	deque<Group> aGroup(range);
	deque<Group> tmp_vf;
	/*** initial merge group ***/
	for(int index = range_start,g_index = 0; index < range_end; index++,g_index++)
	{
		aGroup[g_index].is_RN = false;
		aGroup[g_index].load = ue[index].load[drt];
		//aGroup[g_index].exist_pkt_num = ue[index].pkt[drt].size();
		aGroup[g_index].exist_pkt_num = ue[index].pkt_buffer[drt];
		aGroup[g_index].k = calculate_K(aGroup[g_index].load,data_th /*- aGroup[g_index].load*//*-aGroup[g_index].exist_pkt_num*/);
		aGroup[g_index].k_sharp = calculate_K_sharp(aGroup[g_index].k);
		aGroup[g_index].UE_num.push_back(index);
	}
	sort(aGroup.begin(),aGroup.end(),compare_load);
	/**************************/
	/*** start to merge ***/
	while(schedulability(aGroup) > 1 || is_single_UE(aGroup))
	{
		g_size = aGroup.size();
		/*** non-degraded merge ***/
		for(g_index = g_size-2; g_index >= 0; g_index--)
		{
			nd_group.load = aGroup[g_size-1].load + aGroup[g_index].load;
			nd_group.exist_pkt_num = aGroup[g_size-1].exist_pkt_num + aGroup[g_index].exist_pkt_num;
			nd_group.k = calculate_K(nd_group.load,data_th/* - aGroup[g_index].load*//* - nd_group.exist_pkt_num*/);
			nd_group.k_sharp = calculate_K_sharp(nd_group.k);
			nd_group.UE_num = aGroup[g_index].UE_num;
			nd_group.UE_num.insert(nd_group.UE_num.end(),aGroup[g_size-1].UE_num.begin(),aGroup[g_size-1].UE_num.end());
			if(g_index == g_size-2)
				d_group = nd_group;
			if(nd_group.k_sharp < aGroup[g_index].k_sharp)
				continue;
			aGroup[g_index] = nd_group;
			aGroup.pop_back();
			break;
		}
		/***************************/
		/*** degraded merge ***/
		if(g_index < 0)
		{
			aGroup[g_size-2] = d_group;
			aGroup.pop_back();
		}
		/**********************/
		sort(aGroup.begin(),aGroup.end(),compare_load);
	}
	modify_group(aGroup,M_Group);
	max_k = aGroup[aGroup.size()-1].k_sharp;
	tmp_vf.clear();
	/*tmp_vf.resize(max_k);*/
	for(int index = 0; index < max_k; index++)
		tmp_vf.push_back(tmp_group);
	allocation(aGroup,tmp_vf);
#ifdef RN_1st
	Group RN_group = {true,0,0,0,0};
	RN_group.UE_num.push_back(UE_number);
	tmp_vf.insert(tmp_vf.begin(),&RN_group);
	tmp_vf.pop_back();
#endif
	do{
	virtualframe[drt].insert(virtualframe[drt].end(),tmp_vf.begin(),tmp_vf.end());
	}while(virtualframe[drt].size() < 10);
}

/*
 * function name: allocation
 * brief: Merge groups are allocated to virtualsubframe.
 * param: aGroup,address of Merge group's deque
 * param: tmp_vf,address of temp virtual frame
 * return: none
 */
void allocation(deque<Group> &aGroup,deque<Group> &tmp_vf)
{
	int max_k = tmp_vf.size();
	int g_size = aGroup.size();
	int g_freq, tmp_ksharp;
	for(int index = 0; index < g_size; index++)
	{
		tmp_ksharp = aGroup[index].k_sharp;
		g_freq = max_k / tmp_ksharp;
		/*for(int j = 0; j < max_k; j++)
		{
			if(tmp_vf[j].UE_num.size() > 0)
				continue;
			for(int k = 0; k < g_freq; k++)
			{
				tmp_vf[j] = aGroup[index];
				j += tmp_ksharp;
			}
			break;
		}*/
		for(int j = max_k-1; j >= 0; j--)
		{
			if(tmp_vf[j].UE_num.size() > 0)
				continue;
			for(int k = 0; k < g_freq; k++)
			{
				tmp_vf[j] = aGroup[index];
				j -= tmp_ksharp;
			}
			break;
		}
	}
	
}

/*
 * function name: schedulability
 * brief: To check schedulability for Merge group.
 * param: aGroup,address of Merge group's deque
 * return: sch
 */
double schedulability(deque<Group> &aGroup)
{
	double sch = 0;
	int g_size = aGroup.size();
	for(int index = 0; index < g_size; index++)
	{
		sch += 1.0 / aGroup[index].k_sharp;
	}
#ifdef RN_1st
	sch += 1.0 / aGroup[g_size-1].k_sharp;
#endif
	return sch;
}

/*
 * function name: is_single_UE
 * brief: To check whether there are single UE in a Merge group.
 * param: aGroup,address of Merge group's deque
 * return: is_single
 */
bool is_single_UE(deque<Group> &aGroup)
{
	bool is_single = false;
	int g_size = aGroup.size();
	for(int index = 0; index < g_size; index++)
	{
		if(aGroup[index].UE_num.size() > 1)
			continue;
		is_single = true;
		break;
	}
	return is_single;
}

/*
 * function name: calculate_K
 * brief: value of K is calculated by load
 * param: load
 * param: data_th, it is indicated number of packet. 
 * return: K
 */
int calculate_K(double load,int data_th)
{
	int k = 1;
	// exp, load*k, (load*k)^i/i!, sum of tmp,probability
	double e,lk,tmp,sum,p;
	
	while(1)
	{
		tmp = 1;
		sum = 1; // for i = 0
		lk = load * k;
		e = exp(-lk);
		for(int i = 1; i <= data_th; i++)
		{
			tmp *= lk/i;
			sum += tmp;
		}
		p = 1-sum*e;
		if(p < 0.8)
			k++;
		else
			break;
	}
	return k;
}

/*
 * function name: calculate_K_sharp
 * brief: value of k_sharp is calculated by k
 * param: k
 * return: k_sharp
 */
int calculate_K_sharp(int k)
{
	int k_sharp = 1;
	while(k_sharp <= k)
	{
		k_sharp *= 2;
	}
	k_sharp = k_sharp / 2;
	return k_sharp;
}

/*
 * function name: getCQI
 * brief: There are 2 methods for getint CQI. They are called "3 type" and "Random Way Point" respectively. 
 *        Switching 2 methods with RandomWayPoint defined. To change 3 types for H, M or L with CQI_Type_3 defined.
 * param: none
 * return: CQI
 */
int getCQI(void)
{
	int tmp_cqi = 0;
#ifdef RandomWayPoint
	static int CQI_index = 0;
	static int CQI_buffer_len = CQI_buffer.size();
	tmp_cqi = 80 - (int) CQI_buffer[CQI_index];
	if(++CQI_index >= CQI_buffer_len)
		CQI_index = 0;
#else
	switch(CQI_Type_3)
	{
	case H:
		tmp_cqi = rand()%6+10;
		break;
	case M:
		tmp_cqi = rand()%3+7;
		break;
	case L:
		tmp_cqi = rand()%6+1;
		break;
	default:
		printf("CQI type three error \n");
		break;
	}
#endif
	return tmp_cqi;
}

/*
 * function name: ue_10ms_cqi
 * brief: UE get new CQI every 10 ms(radioframe).
 * param: ue,address of UE's deque
 * return: none
 */
void ue_10ms_cqi(deque<UE> & ue)
{
	for(int index = 0; index < UE_number; index++)
	{
		ue[index].CQI = getCQI();
	}
}

/*
 * function name: get_data_th
 * brief: Data_th, number of packet, is calcuated by capacity of all UE.
 * param: ue,address of UE's deque 
 * param: drt,direction for DL or UL
 * return: data_th
 */
int get_data_th(deque<UE> &ue, Direction drt)
{
	int data_th = 0;
	double avg_c = 0;
	double total_load = 0;
	for(int index = 0; index < UE_number; index++)
	{
		avg_c += wideband_capacity(ue[index].CQI)*ue[index].load[drt];
		total_load += ue[index].load[drt];
	}
	avg_c = avg_c / total_load;
	data_th = (int)(avg_c / Pkt_size);
	return data_th;
}

/*
 * function name: Wideband_Capacity
 * brief: Capacity of UE in a subframe is calculated with wideband CQI of UE.
 * param: cqi
 * return: capacity
 */
int wideband_capacity(int cqi)
{
	double tmp_c = 0;
	tmp_c = RE * CQI_eff[cqi] * RB;
	return (int)tmp_c;
}

/*
 * function name: Subband_Capacity
 * brief: Capacity of UE in a subframe is calculated with subband CQI of UE.
 * param: cqi
 * return: capacity
 */
int subband_capacity(int cqi)
{
	double tmp_c = 0;
	int tmp_cqi = 0;
	// 100 RB can be divided into 13 subcarrier
	for(int index = 0; index < 13; index++)
	{
		switch(cqi)
		{
		case 0:
			break;
		case 1:
			tmp_cqi = cqi + (rand()%2);
			break;
		case 15:
			tmp_cqi = cqi + (rand()%2) - 1;
			break;
		default:
			tmp_cqi = cqi + (rand()%3) - 1;
			break;
		}
		tmp_c += RE * CQI_eff[tmp_cqi];
		printf("tmp_cqi = %d \n",tmp_cqi);
	}
	tmp_c = (tmp_c/13)*RB;
	return (int)tmp_c;
}

/*
 * function name: system_10ms_capacity
 * brief: Capacity of system in a subframe is calculated every 10ms(radioframe).
 * param: ue,address of UE's deque
 * param: system_capacity,address of system_capacity which is global value
 * param: system_load, address of system_load which is global value
 * return: none
 */
void system_10ms_capacity(deque<UE> & ue, int * system_capacity, double * system_load)
{
	int ue_capacity = 0;
	double tmp_sc[2] = {0,0};
	for(int index = 0; index < UE_number; index++)
	{
		ue_capacity = wideband_capacity(ue[index].CQI);
		//printf("ue capacity = %d \n",ue_capacity);
		tmp_sc[DL] += ue_capacity * ue[index].load[DL]; 
		tmp_sc[UL] += ue_capacity * ue[index].load[UL]; 
	}
	for(int index = 0; index < 2; index++)
	{
		if(system_load[index] <= 0)
			continue;
		system_capacity[index] = (int)(tmp_sc[index] / system_load[index]);
	}
}

/*
 * function name: UE_1ms_pkt
 * brief: Packets of UE are created with system_load(sum of UE_lamda) for DL and UL in 1 ms.
 * param: ue,address of UE's deque
 * param: system_load, address of system_load which is global value
 * param: subframe,subframe is equal to runtime + i
 * return: none
 */
void UE_1ms_pkt(deque<UE> &ue, double * system_load, int subframe)
{
	double pkt_arr_timer;
	Packet tmp_pkt;
	int /*range,*/ue_num,pkt_num = 0;
	for(int index = 0; index < 2; index++)
	{
		//range = DU_range[index][1] - DU_range[index][0];
		if(/*range <= 0 || */system_load[index] <= 0)
			continue;
		pkt_arr_timer = Exp_Ran_Time(system_load[index]);
		while(pkt_arr_timer < 1)
		{
			tmp_pkt.pkt_size = Pkt_size;
			tmp_pkt.arr_time = subframe+pkt_arr_timer;
			//ue_num = (rand() % range) + DU_range[index][0];
			ue_num = Rand_UE(index);
			ue[ue_num].pkt[index].push_back(tmp_pkt);
			pkt_arr_timer += Exp_Ran_Time(system_load[index]);
			pkt_num++;
		}
	}
	//printf("pkt_num = %d \n",pkt_num);
}

/*
 * function name: Create_UE_pkt
 * brief: Number of Packets in each buffer of UE are calculated for DL and UL in 1 ms.
 *        Besides, the Create_nTime_pkt function is included in this function. 
 * param: ue,address of UE's deque
 * param: subframe,subframe is equal to runtime + i
 * return: none
 */
void Create_UE_pkt(deque<UE> &ue, int subframe)
{
	int pkt_index = 0;
	int constraint = 0;
	int pkt_num = 0;
	//if((subframe % 1000) == 0)
	//	create_pkt_sf.push_back(subframe);
	//if(ue[0].pkt[0].size() < 100 && create_pkt_sf.size() > 0)
	//{
	//	int tmp_sf = create_pkt_sf[0];
	//	create_pkt_sf.pop_front();
	//	Create_nTime_pkt(ue,tmp_sf,1000);
	//}
	if((subframe % 1000) == 0)
		Create_nTime_pkt(ue,subframe,1000);
	constraint = subframe + 1;
	for(int index = 0; index < UE_number; index++)
	{
		for(int j = 0; j < 2; j++)
		{
			pkt_index = ue[index].pkt_buffer[j];
			pkt_num = ue[index].pkt[j].size();
			while(/*!ue[index].pkt[j].empty()*/pkt_index < pkt_num)
			{
				if(ue[index].pkt[j][pkt_index].arr_time < constraint)
				{
					pkt_index++;
					ue[index].arrive_pkt[j]++;
				}
				else
					break;
			}
			ue[index].pkt_buffer[j] = pkt_index;
		}
	}
	
}

/*
 * function name: Create_nTime_pkt
 * brief: Packets of UE are created with each lamda of UE for DL and UL in time_range ms.
 * param: ue,address of UE's deque
 * param: subframe,subframe is equal to runtime + i
 * param: time_range, range of time which is created packet
 * return: none
 */
void Create_nTime_pkt(deque<UE> &ue, int subframe, int time_range)
{
	double pkt_arr_timer;
	Packet tmp_pkt;
	double lamda;
	for(int index = 0; index < UE_number; index++)
	{
		for(int j = 0; j < 2; j++)
		{
			lamda = ue[index].load[j];
			if(lamda <= 0)
				continue;
			pkt_arr_timer = Exp_Ran_Time(lamda);
			while(pkt_arr_timer < time_range)
			{
				tmp_pkt.pkt_size = Pkt_size;
				tmp_pkt.arr_time = subframe + pkt_arr_timer;
				ue[index].pkt[j].push_back(tmp_pkt);
				pkt_arr_timer += Exp_Ran_Time(lamda);
			}
		}
	}
}

/*
 * function name: Exp_Ran_Time
 * brief: This function is used to calculate inter-arrival time.
 * param: load
 * return: arr_time
 */
double Exp_Ran_Time(double load)
{
	double tmp = (double)(rand()+1)/(double)(RAND_MAX+1);
    double arr_time = (double)log(tmp)*(double)(-1/load);
    return arr_time;
}

/*
 * function name: modify_group
 * brief: According to from_Group which is a deque modify to_Group.
 * param: from_Group,address of group deque 
 * param: to_Group,address of group deque 
 * return: arr_time
 */
void modify_group(deque<Group> &from_Group, deque<Group> &to_Group)
{
	to_Group.assign(from_Group.begin(),from_Group.end());
}

/*
 * function name: compare_load
 * brief: Group's deque is sorted by this function with load.
 * param: group1
 * param: group2
 * return: sort result
 */
bool compare_load(Group group1, Group group2)
{
	return group1.load > group2.load;
}

/*
 * function name: Read_RWP_CQI
 * brief: Read all UE's CQI which is stored in txt file for random way point simulation.
 * param: none
 * return: none
 */
void Read_RWP_CQI(void)
{
	char tmp;
	ifstream RWP_file(RWP_CQI);
	while(!RWP_file.eof())
	{
		RWP_file.get(tmp);
		if(tmp == '\n')
			continue;
		CQI_buffer.push_back(tmp);
	}
}

/*
 * function name: Configure_Load
 * brief: Configure all UE's load, and user should appropriately modify.
 * param: is_load, address of bool array which is indicated UL or DL in this simulation.
 * return: none
 */
void Configure_Load(bool * is_load, short * R_UE_num)
{
	static int ntime = 1;
	/***clear***/
	for(int index = 0; index < 2; index++)
	{
		DU_range[index].clear();
		UE_load[index].clear();
		delay_range[index].clear();
		delay_budget[index].clear();
		//ratio[index].clear();
	}
	/**********/
	/***user modify***/
	DU_range[D].push_back(0);
	DU_range[D].push_back(UE_number);
	UE_load[D].push_back(ntime*UE_DL_load);
	delay_range[D].push_back(0);
	delay_range[D].push_back(20);
	delay_range[D].push_back(UE_number);
	delay_budget[D].push_back(-1);
	delay_budget[D].push_back(6);
	R_UE_num[D] = UE_number - delay_range[D][1];
	//ratio[D].push_back(RAND_MAX);
	is_load[D] = true;
	//DU_range[U].push_back(0);
	//DU_range[U].push_back(UE_number);
	//UE_load[U].push_back(ntime*UE_UL_load);
	//ratio[U].push_back(RAND_MAX);
	//is_load[U] = true;
	ntime++;
	//create_pkt_sf.clear();
}

/*
 * function name: Rand_UE
 * brief: Rand a UE's number.
 * param: drt, ,direction for DL or UL
 * return: none
 */
int Rand_UE(int drt)
{
	int ue_num,range,len;
	len = ratio[drt].size();
	ue_num = rand();
	for(int index = 0; index < len; index++)
	{
		if(ue_num <= ratio[drt][index])
		{
			range = DU_range[drt][index+1] - DU_range[drt][index];
			ue_num = (ue_num % range)+ DU_range[drt][index];
			break;
		}
	}
	return ue_num;
}

/*
 * function name: Print_value
 * brief: Print all value user want to know, and user should appropriately modify.
 * param: ue,address of UE's deque
 * return: none
 */
void Print_value(deque<UE> & ue)
{
	static int ntime = 1;
	//int awake = 0;
	//double delay[2] = {0.0,0.0};
	//double pse = 0.0;

	int awake[2] = {0,0};
	double delay[2] = {0.0,0.0};
	double pse[2] = {0.0,0.0};
	int arrive_pkt[2] = {0,0};
	int discard_pkt[2] = {0,0};
	double PLR[2] = {0.0,0.0};
	for(int i = 0; i < UE_number; i++)
	{
		//printf("ue_num = %d, awake = %d, pkt[DL] = %d, delay[DL] = %lf, pkt[UL] = %d, delay[UL] = %lf\n",i,ue[i].awake,ue[i].pkt_num[DL],ue[i].delay[DL],ue[i].pkt_num[UL],ue[i].delay[UL]);
		//awake += ue[i].awake;
		//pse += (double)(RunTime - ue[i].awake) / RunTime;
		//if(ue[i].pkt_num[DL] > 0)
		//	delay[DL] += ue[i].delay[DL] / ue[i].pkt_num[DL];
		//if(ue[i].pkt_num[UL] > 0)
		//	delay[UL] += ue[i].delay[UL] / ue[i].pkt_num[UL];

		if(ue[i].delay_budget[DL] < 0)
		{
			awake[0] += ue[i].awake;
			delay[0] += ue[i].delay[DL] / ue[i].pkt_num[DL];
			arrive_pkt[0] += ue[i].arrive_pkt[DL];
			discard_pkt[0] += ue[i].discard_pkt[DL];
		}
		else
		{
			awake[1] += ue[i].awake;
			delay[1] += ue[i].delay[DL] / ue[i].pkt_num[DL];
			arrive_pkt[1] += ue[i].arrive_pkt[DL];
			discard_pkt[1] += ue[i].discard_pkt[DL];
		}

	}
	//awake = awake / UE_number;
	//delay[DL] = delay[DL] / UE_number;
	//delay[UL] = delay[UL] / UE_number;
	//pse = (double)(RunTime - awake) / RunTime;

	//printf("%lf \n",pse);
	//printf("%lf \n",delay[DL]);
	//printf("%lf \n",delay[UL]);
	//printf("%lf, %lf\n",pse,delay);

	awake[0] = awake[0] / delay_range[D][1];
	awake[1] = awake[1] / (UE_number-delay_range[D][1]);
	delay[0] = delay[0] / delay_range[D][1];
	delay[1] = delay[1] / (UE_number-delay_range[D][1]);
	pse[0] = (double)(RunTime - awake[0]) / RunTime;
	pse[1] = (double)(RunTime - awake[1]) / RunTime;
	PLR[0] = (double)discard_pkt[0] / arrive_pkt[0];
	PLR[1] = (double)discard_pkt[1] / arrive_pkt[1];

	printf("%lf \n",pse[0]);
	printf("%lf \n",pse[1]);
	printf("%lf \n",delay[DL]);
	printf("%lf \n",delay[UL]);
	printf("%lf \n",PLR[0]);
	printf("%lf \n",PLR[1]);

	ofstream myfile ("Result.txt",ios::out | ios::app);
	if (myfile.is_open())
	{
		//myfile << "pse = " << pse << " , " <<"delay[DL] = " << delay[DL] << " , " << "delay[UL] = " <<  delay[UL] << endl;
		myfile << "pse[n] = " << pse[0] << " , " << "pse[r] = " << pse[1] << " , " <<"delay[n] = " << delay[0] << " , " << "delay[r] = " <<  delay[1] << " , " << "PLR[n] = " << PLR[0] << " , " << "PLR[r] = " <<  PLR[1] <<endl;
	}
	myfile.close();
	printf("ntime = %d \n",ntime);
	ntime++;
}

/*
 * function name: calculate_load
 * brief: To calculate lamdas with avg capacity of 3 types. 
 * param: none
 * return: none
 */
void calculate_load(void)
{
	double h_type = 0,h_load = 0;
	double m_type = 0,m_load = 0;
	double l_type = 0,l_load = 0;
	for(int i = 1; i <= 6; i++)
	{
		l_type += CQI_eff[i];
	}
	l_type /= 6;

	for(int i = 7; i <= 9; i++)
	{
		m_type += CQI_eff[i];
	}
	m_type /= 3;

	for(int i = 10; i <= 15; i++)
	{
		h_type += CQI_eff[i];
	}
	h_type /= 6;
	
	l_type = 144 * 100 * 1000 * l_type;
	m_type = 144 * 100 * 1000 * m_type;
	h_type = 144 * 100 * 1000 * h_type;
	l_load = l_type / (40 * 799 * 1000 * 10);
	m_load = m_type / (40 * 799 * 1000 * 10);
	h_load = h_type / (40 * 799 * 1000 * 10);
	printf("l_type capacity = %lf , l_load = %lf \n", l_type,l_load);
	printf("m_type capacity = %lf , m_load = %lf \n", m_type,m_load);
	printf("h_type capacity = %lf , h_load = %lf \n", h_type,h_load);
}

//
// for real and non-real traffic
//

/*
 * function name: Aggr_R
 * brief: LBPS aggr for Real-time traffic
 * param: ue,address of UE's deque 
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: system_capacity, address of system_capacity which is a global parameter, and be array
 * param: det,direction for DL or UL
 * param: offset,address of TD_offset which is a global parameter
 * param: R_UE_num,number of RT UE
 * param: A_Group,address of group's deque for test
 * return: none
 */
void Aggr_R(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, short * offset, short R_UE_num, deque<Group> &A_Group)
{
	int k, range_start = DU_range[drt].front(), range_end = DU_range[drt].back();
	int data_th = (int)(system_capacity[drt] * Prob_th) / Pkt_size;
	Group tmp_group = {false,0,0,0,0,-1};
	deque<Group> tmp_vf;
	deque<Group> aGroup;
	aGroup.push_back(tmp_group);
	for(int index = range_start; index < range_end; index++)
	{
		aGroup[0].load += ue[index].load[drt];
		aGroup[0].UE_num.push_back(index);
		aGroup[0].exist_pkt_num += ue[index].pkt_buffer[drt];
		Get_delay_budget(&aGroup[0].delay_budget,&ue[index].delay_budget[drt]);
	}
	aGroup[0].k = calculate_K(aGroup[0].load,data_th);
	if(aGroup[0].delay_budget > -1)
		Modify_K(&aGroup[0].k,aGroup[0].delay_budget - *offset);
	modify_group(aGroup,A_Group);
	k = aGroup[0].k;
#ifdef RN_1st
	if(k < 2)
		k = 2;
	for(int index = 0; index < k; index++)
		tmp_vf.push_back(tmp_group);
	Group RN_group = {true,0,0,0,0};
	RN_group.UE_num.push_back(UE_number);
	tmp_vf[0] = RN_group;
	tmp_vf[1] = aGroup[0];
#else
	for(int index = 0; index < k; index++)
		tmp_vf.push_back(tmp_group);
	tmp_vf[k-1] = aGroup[0];
#endif
	do
	{
		virtualframe[drt].insert(virtualframe[drt].end(),tmp_vf.begin(),tmp_vf.end());
	}while(virtualframe[drt].size() < 10);
}

/*
 * function name: Split_R
 * brief: LBPS Split for Real-time traffic
 * param: ue,address of UE's deque 
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: system_capacity, address of system_capacity which is a global parameter, and be array
 * param: drt,direction for DL or UL
 * param: det,direction for DL or UL
 * param: offset,address of TD_offset which is a global parameter
 * param: S_Group,address of group's deque for test
 * return: none
 */
void Split_R(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, short * offset, short R_UE_num, deque<Group> &S_Group)
{
	int min_k = 1;
	int g_size_condition = UE_number;
	int g_size = 1;
	int g_index = 0;
	int tmp_min_k = 1;
	int data_th = (int)(system_capacity[drt] * Prob_th) / Pkt_size;
	int range_start = DU_range[drt].front(), range_end = DU_range[drt].back();
	Group tmp_group = {false,0,0,0,0,-1};
	deque<Group> aGroup;
	deque<Group> tmpGroup;
	deque<Group> tmp_vf;
	/*** split ***/
	do 
	{
#ifdef RN_1st
		tmpGroup.clear();
		tmpGroup = aGroup;
#endif
		if(tmp_min_k > g_size_condition)
			g_size = g_size_condition;
		else
			g_size = tmp_min_k;

		min_k = tmp_min_k;
		aGroup.clear();
		/*aGroup.resize(min_k);*/
		for(int index = 0; index < g_size; index++)
			aGroup.push_back(tmp_group);
		/*** insert ue to group***/
		for(int index = range_start; index < range_end; index++)
		{
			//if(ue[index].load[drt] > 0)
			//{
				g_index = index % g_size;
				aGroup[g_index].load += ue[index].load[drt];
				aGroup[g_index].UE_num.push_back(index);
				//aGroup[g_index].exist_pkt_num += ue[index].pkt[drt].size();
				aGroup[g_index].exist_pkt_num += ue[index].pkt_buffer[drt];
				Get_delay_budget(&aGroup[g_index].delay_budget,&ue[index].delay_budget[drt]);
			//}
		}
		//if(g_size == 2)
		//{
		//	aGroup.clear();
		//	for(int index = 0; index < g_size; index++)
		//	  aGroup.push_back(tmp_group);
		//	g_index = aGroup.size()-1;
		//	for(int index = range_start; index < range_end; index++)
		//	{
		//		aGroup[g_index].load += ue[index].load[drt];
		//		aGroup[g_index].UE_num.push_back(index);
		//		//aGroup[g_index].exist_pkt_num += ue[index].pkt[drt].size();
		//		aGroup[g_index].exist_pkt_num += ue[index].pkt_buffer[drt];
		//		Get_delay_budget(&aGroup[g_index].delay_budget,&ue[index].delay_budget[drt]);
		//		if(index == 19)
		//			g_index--;
		//	}
		//}
		/*************************/
		/*** calculate k of group ***/
		tmp_min_k = INT_MAX;
		for(int index = 0; index < g_size; index++)
		{
			aGroup[index].k = calculate_K(aGroup[index].load,data_th);
			if(aGroup[index].delay_budget > -1)
				Modify_K(&aGroup[index].k,aGroup[index].delay_budget - *offset);
			if(tmp_min_k > aGroup[index].k)
				tmp_min_k = aGroup[index].k;
		}
		/****************************/
	}while((tmp_min_k != min_k) && (g_size != g_size_condition));
	/*************/
	
	for(int index = 0; index < tmp_min_k; index++)
		tmp_vf.push_back(tmp_group);

#ifdef RN_1st
	modify_group(tmpGroup,S_Group);
	Group RN_group = {true,0,0,0,0};
	RN_group.UE_num.push_back(UE_number);
	tmpGroup.insert(tmpGroup.begin(),&RN_group);
	virtualframe[drt].insert(virtualframe[drt].end(),tmpGroup.begin(),tmpGroup.end());
#else
	modify_group(aGroup,S_Group);
	for(int index = 1; index <= g_size; index++)
		tmp_vf[tmp_min_k-index] = aGroup[g_size - index];
	do
	{
		virtualframe[drt].insert(virtualframe[drt].end(),tmp_vf.begin(),tmp_vf.end());
	}while(virtualframe[drt].size() < 10);
#endif
}

/*
 * function name: Merge_R
 * brief: LBPS Merge for Real-time traffic
 * param: ue,address of UE's deque 
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: system_capacity, address of system_capacity which is a global parameter, and be array
 * param: drt,direction for DL or UL
 * param: det,direction for DL or UL
 * param: offset,address of TD_offset which is a global parameter
 * param: S_Group,address of group's deque for test
 * return: none
 */
void Merge_R(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, short * offset, short R_UE_num, deque<Group> &M_Group)
{
	int data_th = (int)(system_capacity[drt] * Prob_th) / Pkt_size;
	int range_start = DU_range[drt].front(), range_end = DU_range[drt].back();
	int range = range_end - range_start;
	int g_size,g_index,max_k,tmp_g_index;
	short N_R_UE[2] = {UE_number - R_UE_num,R_UE_num};
	short N_R = 0;
	bool grouped_first;
	Group nd_group = {false,0,0,0,0,-1},d_group = {false,0,0,0,0,-1};
	Group tmp_group = {false,0,0,0,0,-1};
	deque<Group> aGroup(range);
	deque<Group> tmp_vf;
	/*** initial merge group ***/
	for(int index = range_start,g_index = 0; index < range_end; index++,g_index++)
	{
		aGroup[g_index].is_RN = false;
		aGroup[g_index].load = ue[index].load[drt];
		aGroup[g_index].delay_budget = ue[index].delay_budget[drt];
		aGroup[g_index].exist_pkt_num = ue[index].pkt_buffer[drt];
		aGroup[g_index].k = calculate_K(aGroup[g_index].load,data_th);
		if(aGroup[g_index].delay_budget > -1)
				Modify_K(&aGroup[g_index].k,aGroup[g_index].delay_budget - *offset);
		aGroup[g_index].k_sharp = calculate_K_sharp(aGroup[g_index].k);
		aGroup[g_index].UE_num.push_back(index);
	}
	sort(aGroup.begin(),aGroup.end(),compare_load);
	/**************************/
	/*** start to merge ***/
	while(schedulability(aGroup) > 1 /*|| is_single_UE(aGroup)*/)
	{
		g_size = aGroup.size();
		if(aGroup[g_size-1].delay_budget > -1)
			N_R = 1;
		else
			N_R = 0;
		grouped_first = true;
		/*** non-degraded merge ***/
		for(g_index = g_size-2; g_index >= 0; g_index--)
		{
			if((N_R_UE[N_R] > 1) && ((aGroup[g_size-1].delay_budget * aGroup[g_index].delay_budget) < 0))
				continue;
			nd_group.delay_budget = aGroup[g_size-1].delay_budget;
			Get_delay_budget(&nd_group.delay_budget,&aGroup[g_index].delay_budget);
			nd_group.load = aGroup[g_size-1].load + aGroup[g_index].load;
			nd_group.exist_pkt_num = aGroup[g_size-1].exist_pkt_num + aGroup[g_index].exist_pkt_num;
			nd_group.k = calculate_K(nd_group.load,data_th);
			if(nd_group.delay_budget > -1)
				Modify_K(&nd_group.k,nd_group.delay_budget - *offset);
			nd_group.k_sharp = calculate_K_sharp(nd_group.k);
			nd_group.UE_num = aGroup[g_index].UE_num;
			nd_group.UE_num.insert(nd_group.UE_num.end(),aGroup[g_size-1].UE_num.begin(),aGroup[g_size-1].UE_num.end());
			if(grouped_first)
			{
				tmp_g_index = g_index;
				d_group = nd_group;
				grouped_first = false;
			}
			if(nd_group.k_sharp < aGroup[g_index].k_sharp)
				continue;
			aGroup[g_index] = nd_group;
			aGroup.pop_back();
			break;
		}
		/***************************/
		/*** degraded merge ***/
		if(g_index < 0)
		{
			aGroup[tmp_g_index] = d_group;
			aGroup.pop_back();
		}
		/**********************/
		if(--N_R_UE[N_R] <= 1)
			N_R_UE[N_R] = 1;
		sort(aGroup.begin(),aGroup.end(),compare_load);
	}
	sort(aGroup.begin(),aGroup.end(),compare_k);
	modify_group(aGroup,M_Group);
	max_k = aGroup[aGroup.size()-1].k_sharp;
	tmp_vf.clear();
	/*tmp_vf.resize(max_k);*/
	for(int index = 0; index < max_k; index++)
		tmp_vf.push_back(tmp_group);
	allocation(aGroup,tmp_vf);
#ifdef RN_1st
	Group RN_group = {true,0,0,0,0};
	RN_group.UE_num.push_back(UE_number);
	tmp_vf.insert(tmp_vf.begin(),&RN_group);
	tmp_vf.pop_back();
#endif
	do{
	virtualframe[drt].insert(virtualframe[drt].end(),tmp_vf.begin(),tmp_vf.end());
	}while(virtualframe[drt].size() < 10);
}

/*
 * function name: Get_delay_budget
 * brief: To select the shortest delay budget.
 * param: delay_budget1,address of delay_budget1
 * param: delay_budget2,address of delay_budget2
 * return: none
 */
void Get_delay_budget(short * delay_budget1, short * delay_budget2)
{
	if(*delay_budget1 != 50)
		if((*delay_budget1 > -1) && (*delay_budget2 > -1))
		{
			if(*delay_budget1 > *delay_budget2)
				*delay_budget1 = *delay_budget2;
		}
		else
		{
			if(*delay_budget1 < *delay_budget2)
				*delay_budget1 = *delay_budget2;
		}
}

/*
 * function name: Modify_K
 * brief: To check sleep cycle (k) with constraint of delay budget for real-time traffic
 * param: k,address of k which is sleep cycle
 * param: constraint,delay budget for real-time traffic
 * return: none
 */
void Modify_K(int * k, short constraint)
{
	if(*k > constraint)
		*k = constraint;
}

/*
 * function name: compare_k
 * brief: Group's deque is sorted by this function with k.
 * param: group1
 * param: group2
 * return: sort result
 */
bool compare_k(Group group1, Group group2)
{
	return group1.k < group2.k;
}

/*
 * function name: Discard_Pkt
 * brief: To discard packets with delay budget for real-time traffic.
 * param: ue,address of UE's deque 
 * param: runtime,value of runtime
 * return: none
 */
void Discard_Pkt(deque<UE> &ue, int runtime)
{
	int p_len = 0;
	double tmp_delay = 0.0;
	for(int i = 0; i < 2; i++)
		for(int j = 0; j < UE_number; j++)
		{
			p_len = ue[j].pkt_buffer[i];
			for(int k = 0; k < p_len; k++)
			{
				tmp_delay = (double)runtime - ue[j].pkt[i][0].arr_time;
				if(ue[j].delay_budget[i] > 0 && (tmp_delay - ue[j].delay_budget[i]) > 0)
				{
					ue[j].pkt_buffer[i]--;
					ue[j].pkt[i].pop_front();
					ue[j].discard_pkt[i]++;
				}
				else
					break;
			}
		}
}