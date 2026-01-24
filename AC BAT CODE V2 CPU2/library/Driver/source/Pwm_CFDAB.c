/*
 * Pwm_CFDAB.c
 *
 *  Created on: Dec 18, 2024
 *      Author: maytinh
 */

#include "Pwm_CFDAB.h"
#include "TPC_Setting.h"

void PWM_CFDAB(int period, int deadtime)
{

#if(TPC_MODE_RUN == CFDAB_MODE)
    EALLOW;

    // ePWM(n+1) init.  EPWM(n+1) is a slave
    EPwm7Regs.TBCTL.bit.PRDLD = TB_SHADOW;             // set Immediate load
    EPwm7Regs.TBPRD = period;
    EPwm7Regs.CMPA.bit.CMPA = period;
    EPwm7Regs.TBPHS.bit.TBPHS = 0;
    EPwm7Regs.TBCTR = 0;
    EPwm7Regs.TBCTL.bit.FREE_SOFT = 3;                 // Free run

    EPwm7Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;         // COUNTER_UP
    EPwm7Regs.TBCTL.bit.PHSEN   = TB_DISABLE;          // Master module
    //EPwm1Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    EPwm7Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    EPwm7Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm7Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm7Regs.TBCTL.bit.PHSDIR = TB_UP;

    EPwm7Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;      // load on CTR=Zero
    EPwm7Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm7Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;      // load on CTR=Zero
    EPwm7Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm7Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
    EPwm7Regs.AQCTLA.bit.CAU = AQ_SET;

    EPwm7Regs.AQCTLB.bit.ZRO = AQ_SET;
    EPwm7Regs.AQCTLB.bit.CBU = AQ_CLEAR;

    // activate shadow mode for DBCTL
    EPwm7Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm7Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm7Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;     // enable Dead-band module
    EPwm7Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;          // Active Hi Complimentary
    EPwm7Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm7Regs.DBRED.bit.DBRED = deadtime;              // dummy value for now
    EPwm7Regs.DBFED.bit.DBFED = deadtime;              // dummy value for now

    EPwm7Regs.ETSEL.bit.SOCAEN   = 1;
    EPwm7Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;         // CTR = 0//
    EPwm7Regs.ETPS.bit.SOCAPRD = ET_1ST;               // Generate pulse on 1st event
    EPwm7Regs.ETCLR.bit.SOCA = 1;
    EPwm7Regs.ETPS.bit.SOCACNT = ET_1ST ;              // Generate INT on 1st event

    // Enable CNT_zero interrupt using EPWM1 Time-base
    EPwm7Regs.ETSEL.bit.INTEN = 1;                      // enable EPWM1INT generation
    EPwm7Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;           // enable interrupt CNT_zero event
    EPwm7Regs.ETPS.bit.INTPRD = ET_1ST;                 // generate interrupt on the 1st event
    EPwm7Regs.ETPS.bit.INTCNT = ET_1ST;
    EPwm7Regs.ETCLR.bit.INT = 1;                        // enable more interrupts


    // Q2a   // ePWM(n+1) init.  EPWM(n+1) is a slave
    EPwm8Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set Immediate load
    EPwm8Regs.TBPRD = period;
    EPwm8Regs.CMPA.bit.CMPA = period;             // Fix duty at 100%
    EPwm8Regs.TBPHS.bit.TBPHS = period/2;           // Phase = 180 deg
    EPwm8Regs.TBCTR = 0;
    EPwm8Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

    EPwm8Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm8Regs.TBCTL.bit.PHSEN = TB_ENABLE;           // Slave module
    EPwm8Regs.EPWMSYNCINSEL.bit.SEL = 7;       // Sync "flow through" mode
    EPwm8Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm8Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm8Regs.TBCTL.bit.PHSDIR = TB_UP;            // Count DOWN on sync (=180 deg)

    EPwm8Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm8Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm8Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm8Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm8Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
    EPwm8Regs.AQCTLA.bit.CAU = AQ_SET;

    EPwm8Regs.AQCTLB.bit.ZRO = AQ_SET;
    EPwm8Regs.AQCTLB.bit.CBU = AQ_CLEAR;

    // activate shadow mode for DBCTL
    EPwm8Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm8Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm8Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm8Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;        // Active Hi Complimentary
    EPwm8Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm8Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
    EPwm8Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now


    // Van S1
    EPwm9Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set shadow load
    EPwm9Regs.TBPRD = period;
    EPwm9Regs.CMPA.bit.CMPA = period;             // Fix duty at 100%
    EPwm9Regs.TBPHS.bit.TBPHS = period/2;           // Phase = 180 deg
    EPwm9Regs.TBCTR = 0;
    EPwm9Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

    EPwm9Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm9Regs.TBCTL.bit.PHSEN = TB_ENABLE;           // Slave module
    EPwm9Regs.EPWMSYNCINSEL.bit.SEL = 7;       // Sync "flow through" mode
    EPwm9Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm9Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm9Regs.TBCTL.bit.PHSDIR = TB_UP;            // Count DOWN on sync (=180 deg) // chi dung khi up-down

    EPwm9Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm9Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm9Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm9Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm9Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm9Regs.AQCTLA.bit.CAU = AQ_CLEAR;

    EPwm9Regs.AQCTLB.bit.ZRO = AQ_SET;
    EPwm9Regs.AQCTLB.bit.CBU = AQ_CLEAR;
   /* EPwm9Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
    EPwm9Regs.AQCTLA.bit.CAU = AQ_SET;

    EPwm9Regs.AQCTLB.bit.ZRO = AQ_SET;
    EPwm9Regs.AQCTLB.bit.CBU = AQ_CLEAR;*/
    // activate shadow mode for DBCTL
    EPwm9Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm9Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm9Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm9Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;        // Active Hi Complimentary
    EPwm9Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm9Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
    EPwm9Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now

    //-----------------------------------------------------

    // ePWM(n+1) init.  EPWM(n+1) is a slave
    EPwm10Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set shadow load
    EPwm10Regs.TBPRD = period;
    EPwm10Regs.CMPA.bit.CMPA = 0;             // Fix duty at 100%
    EPwm10Regs.CMPB.bit.CMPB = period;             // Fix duty at 100%
    EPwm10Regs.TBPHS.bit.TBPHS = period/2;           // Phase = 180 deg
    EPwm10Regs.TBCTR = 0;
    EPwm10Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

    EPwm10Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm10Regs.TBCTL.bit.PHSEN = TB_ENABLE;           // Slave module
    EPwm10Regs.EPWMSYNCINSEL.bit.SEL = 7;       // Sync "flow through" mode
    EPwm10Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm10Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm10Regs.TBCTL.bit.PHSDIR = TB_UP;            // Count DOWN on sync (=180 deg)

    EPwm10Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm10Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm10Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm10Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm10Regs.AQCTLA.bit.ZRO = AQ_SET;                                                     // tao them 1 phan xung S2
    EPwm10Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm10Regs.AQCTLA.bit.CBU = AQ_SET;

    EPwm10Regs.AQCTLB.bit.ZRO = AQ_SET;
    EPwm10Regs.AQCTLB.bit.CAU = AQ_CLEAR;
    EPwm10Regs.AQCTLB.bit.CBU = AQ_SET;

    // activate shadow mode for DBCTL
    EPwm10Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm10Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm10Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm10Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;        // Active Hi Complimentary
    EPwm10Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm10Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
    EPwm10Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now

    EDIS;
#endif

#if(TPC_MODE_RUN == VFDAB_MODE)
//VFDAB viet o day nhe
#endif

}

