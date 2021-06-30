#ifndef __STPMC1_H
#define __STPMC1_H

#include "main.h"
#define STPMC1_REG_NUM 28

#define PARITY(X) 			(((X) & 0xF0000000) >> 28)
#define ENERGY(X)			(((X) & 0x0FFFFF00) >> 8)
#define STATUS(X)			(((X) & 0x000000FF) >> 0)
#define uMOM(X)				(((X) & 0x0FFF0000) >> 16)
#define iMOM(X)				(((X) & 0x0000FFFF) >> 0)
#define uRMS(X)				(((X) & 0x0FFF0000) >> 16)
#define iRMS(X)				(((X) & 0x0000FFFF) >> 0)
#define CFG(X)				(((X) & 0x0FFFFFFF) >> 0)
// #define BIT(X,Y)			(((X) >> (Y)) & 0x1)


typedef struct 
{
	uint32_t DAP;
	uint32_t DRP;
	uint32_t DFP;
	uint32_t PRD;
	
	uint32_t DMR;
	uint32_t DMS;
	uint32_t DMT;
	uint32_t DMN;
	
	uint32_t DER;
	uint32_t DES;
	uint32_t DET;
	uint32_t DEN;
	
	uint32_t DAR;
	uint32_t DAS;
	uint32_t DAT;
	uint32_t CF0;
	
	uint32_t DRR;
	uint32_t DRS;
	uint32_t DRT;
	uint32_t CF1;
	
	uint32_t DFR;
	uint32_t DFS;
	uint32_t DFT;
	uint32_t CF2;
	
	uint32_t ACR;
	uint32_t ACS;
	uint32_t ACT;
	uint32_t CF3;
} stpmc1_regMap;

// for R S T N energy
typedef struct {
	uint32_t energy_active_fundamental;
	uint8_t  energy_active_fundamental_status;
	uint32_t energy_reactive;
	uint8_t  energy_reactive_status;
	uint32_t energy_active_wideband;
	uint8_t energy_active_wideband_status;
	uint32_t uRMS;
	uint32_t iRMS;
	uint32_t uMOM;
	uint32_t iMOM;
} stpmc1_measure;

// for 3phase energy
typedef struct{
	uint32_t energy_active_fundamental;
	uint32_t energy_reactive;
	uint32_t energy_active_wideband;
	uint16_t status;
}stpmc1_measure2;

typedef struct {
	uint16_t _0_15;
	uint16_t _16_31;
	uint16_t _32_47;
	uint16_t _48_63;
	uint16_t _64_79;
	uint16_t _80_95;
	uint16_t _96_111;
} stpmc1_config;

typedef struct {
    bool meas_event;
    uint8_t reg[STPMC1_REG_NUM * 4];
	stpmc1_regMap regMap;
	
	stpmc1_measure2 phase3;
	
	stpmc1_measure R;
	stpmc1_measure S;
	stpmc1_measure T;
	stpmc1_measure N;
	
	stpmc1_config config;
	
	uint8_t TSG;
	uint16_t period;
	uint16_t DC;
	
} stpmc1_t;

stpmc1_t* stpmc1_init(void);
void stpmc1_remoteReset(void);
void stpmc1_remoteLatch(void);

void stpmc1_set_config(uint8_t configAddr);
void stpmc1_clr_config(uint8_t configAddr);

void stpmc1_update_measures(void);
void stpmc1_get_measures(void);

#endif /* __STPMC1_H */
