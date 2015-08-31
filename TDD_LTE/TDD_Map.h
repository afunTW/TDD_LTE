#ifndef _TDD_MAP_H
#define _TDD_MAP_H
#include "LTE_TypeDefine.h"

#define TD_Con_num		3  // TD_Configuration number between 0 and 6



void DRX_Mapping(Subframe * rf);
void FD_mapping(deque<Group> & virtualframe, Subframe *rf);
void Mapping1(deque<Group> * virtualframe, Subframe * rf);
void Mapping2(deque<Group> * virtualframe, Subframe * rf);
void Mapping3(deque<Group> * virtualframe, Subframe * rf);
void Clear_RealSubFrame(Subframe * rf);
void RN_Mapping(deque<Group> * virtualframe, Subframe * rf, int * rf_num, int * vf_num);
void RF_Insrt_UE1(deque<Group> & virtualframe, Subframe * rf);
void RF_Insrt_UE2(deque<Group> & virtualframe, Subframe * rf, int * rest_c, int rf_num, int vf_num);
void RF_Insrt_UE3(deque<Group> & virtualframe, Subframe * rf, int * rest_c, int rf_num, int vf_num);
void filter_duplicate_ue(Subframe * rf, Group * aGroup);
void virtual_10ms_capacity(int * system_capacity, int * virtual_capacity);
void Get_offset(short * offset);
//bool compare_pkt_num(deque<UE> & ue, int num1, int num2);



#endif