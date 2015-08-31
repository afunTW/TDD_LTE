#include "stdafx.h"
#include "TDD_Map.h"


/*
 * Global value:
 * TD_Config[8] : TDD Configuration table
 * TD_Offset[8] : Offset table for different TDD configuration
 * RN_map : (This is not finish.)
 */

Configuration TD_Config[8] = {{{2,6},{D,S,U,U,U,D,S,U,U,U}},
							  {{4,4},{D,S,U,U,D,D,S,U,U,D}},
							  {{6,2},{D,S,U,D,D,D,S,U,D,D}},
							  {{6,3},{D,S,U,U,U,D,D,D,D,D}},
							  {{7,2},{D,S,U,U,D,D,D,D,D,D}},
							  {{8,1},{D,S,U,D,D,D,D,D,D,D}},
							  {{3,5},{D,S,U,U,U,D,S,U,U,D}},
							  {{10,0},{D,D,D,D,D,D,D,D,D,D}}};
short TD_Offset[8] = {0,3,4,4,3,2,3,0};
int RN_map[7][2][10] = {0};


/*
 * function name: DRX_Mapping
 * brief: Mapping to realsubframe for DRX project.
 * param: rf, address of radioframe which is a global parameter, and be Subframe array
 * return: none
 */
void DRX_Mapping(Subframe * rf)
{
	Clear_RealSubFrame(rf);
	for(int index = 0; index < 10; index++)
	{
		if(rf[index].is_RN)
			continue;
		switch(TD_Config[TD_Con_num].Config[index])
		{
			case D:
				rf[index].drt = D;
				break;
			case U:
				rf[index].drt = U;
				break;
			case S:
				rf[index].drt = S;
				break;
			default:
				break;
		}
	}
}

/*
 * function name: FD_mapping
 * brief: Mapping virtualframe to realsubframe for FD project.
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: rf, address of radioframe which is a global parameter, and be Subframe array
 * return: none
 */
void FD_mapping(deque<Group> & virtualframe, Subframe *rf)
{
	rf->drt = D;
	rf->is_RN = false;
	if(virtualframe.empty())
	{
		rf->UE_num.clear();
	}
	else
	{
		rf->UE_num = virtualframe[0].UE_num;
		virtualframe.pop_front();
	}
}

/*
 * function name: Mapping1
 * brief: According to TD-configuration map virtualsubframe to realsubframe.
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: rf, address of radioframe which is a global parameter, and be Subframe array
 * return: none
 */
void Mapping1(deque<Group> * virtualframe, Subframe * rf)
{
	Subframe tmp_rf[2];
	Clear_RealSubFrame(rf);
	if(!virtualframe[D].empty())
		RF_Insrt_UE1(virtualframe[D],&tmp_rf[D]);
	if(!virtualframe[U].empty())
		RF_Insrt_UE1(virtualframe[U],&tmp_rf[U]);
	for(int index = 0; index < 10; index++)
	{
		if(rf[index].is_RN)
			continue;
		switch(TD_Config[TD_Con_num].Config[index])
		{
			case D:
				rf[index].drt = D;
				//if(!virtualframe[D].empty())
					rf[index].UE_num = tmp_rf[D].UE_num;
				break;
			case U:
				rf[index].drt = U;
				//if(!virtualframe[U].empty())
					rf[index].UE_num = tmp_rf[U].UE_num;
				break;
			case S:
				rf[index].drt = S;
				break;
			default:
				break;
		}
	}
}

/*
 * function name: Mapping2
 * brief: According to TD-configuration map virtualsubframe to realsubframe.
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: rf, address of radioframe which is a global parameter, and be Subframe array
 * return: none
 */
void Mapping2(deque<Group> * virtualframe, Subframe * rf)
{
	int rf_num[2] ,vf_num[2] = {10,10},rest_c[2];
	RN_Mapping(virtualframe,rf,rf_num,vf_num);
	rest_c[0] = rf_num[0];
	rest_c[1] = rf_num[1];
	for(int index = 0; index < 10; index++)
	{
		if(rf[index].is_RN)
			continue;
		switch(TD_Config[TD_Con_num].Config[index])
		{
			case D:
				rf[index].drt = D;
				if(!virtualframe[D].empty())
					RF_Insrt_UE2(virtualframe[D],&rf[index],&rest_c[D],rf_num[D],vf_num[D]);
				break;
			case U:
				rf[index].drt = U;
				if(!virtualframe[U].empty())
					RF_Insrt_UE2(virtualframe[U],&rf[index],&rest_c[U],rf_num[U],vf_num[U]);
				break;
			case S:
				rf[index].drt = S;
				break;
			default:
				break;
		}
	}
	//virtualframe[DL].erase(virtualframe[DL].begin(),virtualframe[DL].begin()+10);
	//virtualframe[UL].erase(virtualframe[UL].begin(),virtualframe[UL].begin()+10);
}

/*
 * function name: Mapping3
 * brief: According to TD-configuration map virtualsubframe to realsubframe.
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: rf, address of radioframe which is a global parameter, and be Subframe array
 * return: none
 */
void Mapping3(deque<Group> * virtualframe, Subframe * rf)
{
	int rf_num[2] ,vf_num[2] = {10,10},rest_c[2];
	RN_Mapping(virtualframe,rf,rf_num,vf_num);
	rest_c[0] = rf_num[0];
	rest_c[1] = rf_num[1];
	for(int index = 0; index < 10; index++)
	{
		if(rf[index].is_RN)
			continue;
		switch(TD_Config[TD_Con_num].Config[index])
		{
			case D:
				rf[index].drt = D;
				if(!virtualframe[D].empty())
					RF_Insrt_UE3(virtualframe[D],&rf[index],&rest_c[D],rf_num[D],vf_num[D]);
				break;
			case U:
				rf[index].drt = U;
				if(!virtualframe[U].empty())
					RF_Insrt_UE3(virtualframe[U],&rf[index],&rest_c[U],rf_num[U],vf_num[U]);
				break;
			case S:
				rf[index].drt = S;
				break;
			default:
				break;
		}
	}
	if(!virtualframe[D].empty())
		virtualframe[D].erase(virtualframe[D].begin(),virtualframe[D].begin()+10);
	if(!virtualframe[U].empty())
		virtualframe[U].erase(virtualframe[U].begin(),virtualframe[U].begin()+10);
}

/*
 * function name: Clear_RealSubFrame
 * brief: To clear realsubframe
 * param: rf, address of radioframe which is a global parameter, and be Subframe array
 * return: none
 */
void Clear_RealSubFrame(Subframe * rf)
{
	for(int i = 0; i < 10; i++)
	{
		rf[i].is_RN = false;
		rf[i].drt = D;
		rf[i].UE_num.clear();
	}
}

/*
 * function name: RN_Mapping
 * brief: RN group in virtualframe is mapped to realframe.
 * param: virtualframe,address of virtualframe which is a global parameter, and be deque array 
 * param: rf, address of radioframe which is a global parameter, and be Subframe array
 * param: rf_num, address of rf_num which is array and stored available number of realsubframe(except RN)
 * param: vf_num, address of vf_num which is array and stored available number of virtualsubframe(except RN)
 * return: none
 */
void RN_Mapping(deque<Group> * virtualframe, Subframe * rf, int * rf_num, int * vf_num)
{
	int tmp_index;
	int rn_vf_count[2] = {0,0};
	Clear_RealSubFrame(rf);
	for(int i = 0; i < 2; i++)
	{
		if(virtualframe[i].empty())
			continue;
		rf_num[i] = TD_Config[TD_Con_num].sf_num[i];
		vf_num[i] = 10;
		for(int j = 0; j < 10; j++)
		{
			if(virtualframe[i][j].is_RN)
			{
				tmp_index = RN_map[TD_Con_num][i][j];//?????
				rf[tmp_index].is_RN = true;
				rf[tmp_index].drt = (SF_Config)i;
				//rf[tmp_index].UE_num.push_back(virtualframe[i][j].UE_num[0]);
				rn_vf_count[i]++;
			}
		}
		if(rn_vf_count[i] > 0)
		{
			rf_num[i]--;
			vf_num[i] -= rn_vf_count[i];
		}
	}
}

/*
 * function name: filter_duplicate_ue
 * brief: UE is inserted to realsubframe.
 * param: rf, address of a radioframe which is Subframe type
 * param: aGroup, address of a virtualframe which is Group type
 * return: none
 */
void filter_duplicate_ue(Subframe * rf, Group * aGroup)
{
	int g_u_len = aGroup->UE_num.size();
	int rf_u_len = 0;
	bool is_duplicate = false;
	for(int i = 0; i < g_u_len; i++)
	{
		is_duplicate = false;
		rf_u_len = rf->UE_num.size();
		for(int j = 0; j < rf_u_len; j++)
		{
			if(aGroup->UE_num[i] == rf->UE_num[j])
			{
				is_duplicate = true;
				break;
			}
		}
		if(!is_duplicate)
			rf->UE_num.push_back(aGroup->UE_num[i]);
	}
}

/*
 * function name: RF_Insrt_UE1
 * brief: UE is inserted to realsubframe.
 * param: virtualframe,address of virtualframe's deque for DL or UL
 * param: rf, address of radioframe which is Subframe type
 * return: none
 */
void RF_Insrt_UE1(deque<Group> & virtualframe, Subframe * rf)
{
	for(int i = 0; i < 10; i++)
	{
		filter_duplicate_ue(rf,&virtualframe[0]);
		virtualframe.pop_front();
	}
}
/*
 * function name: RF_Insrt_UE2
 * brief: UE is inserted to realsubframe.
 * param: virtualframe,address of virtualframe's deque for DL or UL
 * param: rf, address of radioframe which is Subframe type
 * param: rest_c, address of rest_c which is array and stored rest of capacity in a available realsubframe
 * param: rf_num, available number of realsubframe(except RN)
 * param: vf_num, available number of virtualsubframe(except RN)
 * return: none
 */
void RF_Insrt_UE2(deque<Group> & virtualframe, Subframe * rf, int * rest_c, int rf_num, int vf_num)
{
	int r_c = vf_num;
	do
	{
		if(virtualframe[0].is_RN)
		{
			//virtualframe.erase(virtualframe.begin());
			virtualframe.pop_front();
			continue;
		}
		//rf->UE_num.insert(rf->UE_num.end(),virtualframe[0].UE_num.begin(),virtualframe[0].UE_num.end());
		filter_duplicate_ue(rf,&virtualframe[0]);
		r_c -= *rest_c;
		if(r_c < 0)
		{
			*rest_c = -r_c;
			break;
		}
		//virtualframe.erase(virtualframe.begin());
		virtualframe.pop_front();
		*rest_c = rf_num;
	}while(r_c != 0);
}

/*
 * function name: RF_Insrt_UE3
 * brief: UE is inserted to realsubframe.
 * param: virtualframe,address of virtualframe's deque for DL or UL
 * param: rf, address of radioframe which is Subframe type
 * param: rest_c, address of rest_c which is array and stored rest of capacity in a available realsubframe
 * param: rf_num, available number of realsubframe(except RN)
 * param: vf_num, available number of virtualsubframe(except RN)
 * return: none
 */
void RF_Insrt_UE3(deque<Group> & virtualframe, Subframe * rf, int * rest_c, int rf_num, int vf_num)
{
	int r_c = vf_num;
	static char i = 0, j = rf_num;
	while(r_c >= rf_num)
	{
		filter_duplicate_ue(rf,&virtualframe[i++]);
		r_c -= rf_num;
	}
	while(r_c != 0)
	{
		filter_duplicate_ue(rf,&virtualframe[j]);
		r_c -= *rest_c;
		if(r_c < 0)
		{
			*rest_c = -r_c;
			break;
		}
		j++;
		*rest_c = rf_num;
	}
	if(j >= 10 || i >= 10)
	{
		i = 0;
		j = rf_num;
	}
}

/*
 * function name: virtual_10ms_capacity
 * brief: Capacity in a virtualsubframe is calculated every 10ms(radioframe).
 * param: system_capacity,address of system_capacity which is global value
 * param: virtual_capacity, address of virtual_capacity which is global value
 * return: none
 */
void virtual_10ms_capacity(int * system_capacity, int * virtual_capacity)
{
	virtual_capacity[D] = system_capacity[D] * TD_Config[TD_Con_num].sf_num[D] / 10;
	virtual_capacity[U] = system_capacity[U] * TD_Config[TD_Con_num].sf_num[U] / 10;
}

/*
 * function name: Get_offset
 * brief: To get mapping offset with TD-configuration.
 * param: system_capacity,address of system_capacity which is global value
 * param: virtual_capacity, address of virtual_capacity which is global value
 * return: none
 */
void Get_offset(short * offset)
{
	*offset = TD_Offset[TD_Con_num];
}











