#ifndef _LTE_TYPEDEFINE_H
#define _LTE_TYPEDEFINE_H

typedef enum {H = 0, M = 1, L = 2} CQI_Type;	// CQI environment
typedef enum {DL = 0, UL = 1} Direction;		// DL or UL environment
typedef enum {D = 0, U = 1, S =2} SF_Config;	// TD subframe configuration 
typedef enum {IN = 0, ON = 1, Slp = 2} DRX_Status;  // status of DRX

/*
 * attribute of packet
 */
typedef struct {
	int pkt_size ;				 // size of a packet
	double arr_time;             // packet arrive time
} Packet;

/*
 * attribute of UE
 */
typedef struct {
	int    pkt_num[2];	              // number of packet processed
	int    arrive_pkt[2];        // number of packet arrived
	int    discard_pkt[2];        // number of packet discarded
	int    CQI;                   // CQI of UE
	int    awake;                 // amout of times that UE wakes up
	int    k[2];				  // k of UE. k[0] and k[1] are in DL and UL,respectively.
	double load[2];	              // load of UE. load[0] and load[1] are in DL and UL,respectively.
	double delay[2];                 // amout of packet delay
	int	   pkt_buffer[2];    	  // number of packet in buffer
	short delay_budget[2];        // constraint of delay. The value which is -1 show non-real traffic.
	deque<Packet> pkt[2];         // buffer of packet. pkt[0] and pkt[1] are in DL and UL, respectively.	
} UE;

/*
 * attribute of Group
 */
typedef struct {
	bool is_RN;
	int k;                        // k of group. k[0] and k[1] are in DL and UL,respectively.
	int k_sharp;                  // new k of gruop. newk[0] and newk[1] are in DL and UL,respectively.
	int exist_pkt_num;				  // number of packet exist in a group.
	double load;                  // load of Group. load[0] and load[1] are in DL and UL,respectively.
	short delay_budget;           // constraint of delay in the Group.
	deque<int> UE_num;           // number of UE in the Group
} Group;

/*
 * attribute of Configuration
 */
typedef struct {
	int sf_num[2];
	SF_Config Config[10];
} Configuration;

/*
 * attribute of Subframe
 */
typedef struct {
	bool is_RN;					  // denote whether RN or not
	SF_Config drt;                // D,U or S
	deque<int> UE_num;           // number of UE in a subframe
} Subframe;

/*
 * attribute of UE for sorting
 */
typedef struct 
{
	int ue_num;
	int pkt_num;
	short delay_budget;
} Sort_UE;

/*
 * attribute of status for UE
 */
typedef struct
{
	DRX_Status now_status;
	DRX_Status next_status;
	short shrt_t;
	short timer;
} UE_Status;

#endif