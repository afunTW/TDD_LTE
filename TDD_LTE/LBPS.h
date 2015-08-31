#ifndef _LBPS_H
#define _LBPS_H
#include "LTE_TypeDefine.h"
// for general configuration
#define RunTime   10000          // run time
#define UE_number    40		     // number of UE
//#define RN_1st
//#define RandomWayPoint
#define RWP_CQI	  "RWP_Walk.txt"
#define Pkt_size  799            // packet size
#define RB        100            // number of Resource Block
#define Prob_th   0.8			 // threshold of probability for calculating k
#define UE_DL_load   H_load*0.6      // load of DL UE for UE_initial() in LBPS.cpp
#define UE_UL_load   H_load*0.3      // load of UL UE for UE_initial() in LBPS.cpp
// for load of 3 type configuration
/*
lamda(H_load) = bit rate / packet size => 0.176 * 40 * 799 * 1000= 5.6Mbps
RE * eff[cqi] * RB = (12*12) * 3.9023 * 100 * 1000(1s = 1000ms) = 56.2Mbps
*/
#define H_load    0.189          // H_type load //0.176
#define M_load    0.0871          // M_type load //0.076
#define L_load    0.026          // L_type load //0.017
#define CQI_Type_3   H           // CQI type may be H, M or L for getCQI() in LBPS.cpp
// for detail resource element configuration
#define subcarrier   12          // number of subcarrier in a RB is 12
#define total_symbol 14          // number of symbol in a RB is 14
#define ctl_symbol   2           // number of symbol for control signal in a RB is between 1 to 3
#define RE        (total_symbol-ctl_symbol)*subcarrier  // number of Resource element in a RB






void UE_initial(deque<UE> &ue, double * system_load);
void Aggr(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, deque<Group> &A_Group);
void Split(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, deque<Group> &S_Group);
void Merge(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, deque<Group> &M_Group);
void allocation(deque<Group> &aGroup,deque<Group> &tmp_vf);
double schedulability(deque<Group> &aGroup);
bool is_single_UE(deque<Group> &aGroup);
int calculate_K(double load,int data_th);
int calculate_K_sharp(int k);
int getCQI(void);
void ue_10ms_cqi(deque<UE> & ue);
int get_data_th(deque<UE> &ue, Direction drt);
int wideband_capacity(int cqi);
int subband_capacity(int cqi);
void system_10ms_capacity(deque<UE> & ue, int * system_capacity, double * system_load);
void UE_1ms_pkt(deque<UE> &ue, double * system_load, int subframe);
void Create_UE_pkt(deque<UE> &ue, int subframe);
void Create_nTime_pkt(deque<UE> &ue, int subframe, int time_range);
double Exp_Ran_Time(double load);
void modify_group(deque<Group> &from_Group, deque<Group> &to_Group);
bool compare_load(Group group1, Group group2);
void Read_RWP_CQI(void);
void Configure_Load(bool * is_load, short * R_UE_num);
int Rand_UE(int drt);
void Print_value(deque<UE> & ue);


//
// for real and non-real traffic
//
void Aggr_R(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, short * offset, short R_UE_num, deque<Group> &A_Group);
void Split_R(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, short * offset, short R_UE_num, deque<Group> &S_Group);
void Merge_R(deque<UE> &ue, deque<Group> * virtualframe, int * system_capacity, Direction drt, short * offset, short R_UE_num, deque<Group> &M_Group);
void Get_delay_budget(short * delay_budget1, short * delay_budget2);
void Modify_K(int * k, short constraint);
bool compare_k(Group group1, Group group2);
void Discard_Pkt(deque<UE> &ue, int runtime);
/*
void UE_initial(void);
void Aggr(vecotr<Group> A_Group);
int Merge(deque<Group> M_Group);
int Split(deque<Group> S_Group);
void Mapping(deque<Group>* RF,deque<Group>DL_vf,deque<Group>UL_vf);
void calculate_K(double load);
*/
void calculate_load(void);
#endif