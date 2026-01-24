/*
 * Pwm_VFDAB.c
 *
 *  Created on: Mar 5, 2025
 *      Author: levuo
 */
/*
 * Pwm_CFDAB.c
 *
 *  Created on: Dec 18, 2024
 *      Author: maytinh
 */

#include "Pwm_VFDAB.h"
#include "TPC_Setting.h"

void PWM_VFDAB(int period, int deadtime)
{
    EALLOW;
// Primary side : EPWM1A/B, EPWM2A/B
 // n = the ePWM module number, i.e. selects the target module for init.
 // ePWM(n) init.  Note EPWM(n) is the Master
     EPwm7Regs.TBCTL.bit.PRDLD = TB_SHADOW;             // set Immediate load
     EPwm7Regs.TBPRD = period;
     EPwm7Regs.CMPA.bit.CMPA = 0;
     EPwm7Regs.CMPB.bit.CMPB = 0;
//     EPwm1Regs.CMPC = period/4;
     EPwm7Regs.TBPHS.bit.TBPHS = 0;
     EPwm7Regs.TBCTR = 0;
     EPwm7Regs.TBCTL.bit.FREE_SOFT = 3;                 // Free run


     EPwm7Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
     EPwm7Regs.TBCTL.bit.PHSEN = TB_DISABLE;           // Slave module
     EPwm7Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;      // Sync "flow through" mode
     //EPwm7Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
     EPwm7Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;             // /1
     EPwm7Regs.TBCTL.bit.CLKDIV = TB_DIV1;                // /1  // fTBCLK = fEpwmclk = 2000 Hz

     EPwm7Regs.TBCTL.bit.PHSDIR = TB_UP;                // Continue count up after synchronization event

     EPwm7Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;      // load on CTR=Zero
     EPwm7Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
     EPwm7Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;      // load on CTR=Zero
     EPwm7Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

     EPwm7Regs.AQCTLA.bit.ZRO = AQ_SET;  //PWM1A
     EPwm7Regs.AQCTLA.bit.CAU = AQ_CLEAR;

     EPwm7Regs.AQCTLB.bit.PRD = AQ_CLEAR;  //PWM1B
     EPwm7Regs.AQCTLB.bit.CBD = AQ_SET;

     // activate shadow mode for DBCTL
     EPwm7Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
     // reload on CTR = 0
     EPwm7Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

     EPwm7Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;     // enable Dead-band module
     EPwm7Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;          // Active Hi Complimentary: EPWM1B is inverted
     EPwm7Regs.DBCTL.bit.IN_MODE = DBA_RED_DBB_FED;
     EPwm7Regs.DBRED.bit.DBRED = deadtime;              // Dead-time = DBRED / fTBCLK = 60/(200M) = 300 ns
     EPwm7Regs.DBFED.bit.DBFED = deadtime;              // dummy value for now

     // Enable EPWM_SOCA using EPWM1 Time-base
     EPwm7Regs.ETSEL.bit.SOCASELCMP = 1;                // Enable event time-base counter equal to CMPC
     EPwm7Regs.ETSEL.bit.SOCAEN   = 1;                  // Enable the ADC start of conversion A Pulse (ePWM1SOCA)
     EPwm7Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;        // Enable event time-base counter equal to CMPA when the timer is incrementing or CMPC when the timer is incrementing
     EPwm7Regs.ETPS.bit.SOCAPRD = ET_1ST;               // Generate pulse EPWM_SOCA on 1st event
     EPwm7Regs.ETCLR.bit.SOCA = 1;
     EPwm7Regs.ETPS.bit.SOCACNT = ET_1ST;               // Generate INT on 1st event

     // original
//     EPwm1Regs.ETSEL.bit.SOCAEN   = 1;                  // Enable the ADC start of conversion A Pulse (ePWM1SOCA)
//     EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_PRDZERO;      // CTR = 0 and PRD//
//     EPwm1Regs.ETPS.bit.SOCAPRD = ET_2ND;               // Generate pulse EPWM_SOCA on 2st event
//     EPwm1Regs.ETCLR.bit.SOCA = 1;
//     EPwm1Regs.ETPS.bit.SOCACNT = ET_2ND;               // Generate INT on 2st event

     // Enable CNT_zero interrupt using EPWM1 Time-base
     EPwm7Regs.ETSEL.bit.INTEN = 1;                      // enable EPWM1INT generation
     EPwm7Regs.ETSEL.bit.INTSEL = ET_CTR_PRDZERO;        // enable interrupt CNT_zero event
     EPwm7Regs.ETPS.bit.INTPRD = ET_2ND;                 // generate interrupt on the 2st event
     EPwm7Regs.ETPS.bit.INTCNT = ET_2ND;
     EPwm7Regs.ETCLR.bit.INT = 1;                        // enable more interrupts

 /*    EPwm7Regs.ETSEL.bit.INTEN = 1;                      // enable EPWM1INT generation
     EPwm7Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;           // enable interrupt CNT_zero event
     EPwm7Regs.ETPS.bit.INTPRD = ET_1ST;                 // generate interrupt on the 1st event
     EPwm7Regs.ETPS.bit.INTCNT = ET_1ST;
     EPwm7Regs.ETCLR.bit.INT = 1;                        // enable more interrupts
     */
     //-----------------------------------------------------
 // ePWM(n+1) init.  EPWM(n+1) is a slave
     EPwm8Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set Immediate load
     EPwm8Regs.TBPRD = period;
     EPwm8Regs.CMPA.bit.CMPA = 0;                  // Fix duty at 0%
     EPwm8Regs.CMPB.bit.CMPB = period;             // Fix duty at 0%

     EPwm8Regs.TBPHS.bit.TBPHS = period;           // Phase = 180 deg
     EPwm8Regs.TBCTR = 0;
     EPwm8Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

     EPwm8Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
     EPwm8Regs.TBCTL.bit.PHSEN = TB_ENABLE;           // Slave module
     EPwm8Regs.EPWMSYNCINSEL.bit.SEL = 7;       // Sync "flow through" mode
     EPwm8Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
     EPwm8Regs.TBCTL.bit.CLKDIV = TB_DIV1;
     EPwm8Regs.TBCTL.bit.PHSDIR = TB_DOWN;            // Count DOWN on sync (=180 deg)

     EPwm8Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
     EPwm8Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
     EPwm8Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
     EPwm8Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

     EPwm8Regs.AQCTLA.bit.ZRO = AQ_SET;
     EPwm8Regs.AQCTLA.bit.CAU = AQ_CLEAR;

     EPwm8Regs.AQCTLB.bit.PRD = AQ_CLEAR;
     EPwm8Regs.AQCTLB.bit.CBD = AQ_SET;

     // activate shadow mode for DBCTL
     EPwm8Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
     // reload on CTR = 0
     EPwm8Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

     EPwm8Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;     // enable Dead-band module
     EPwm8Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;          // Active Hi Complimentary: EPWM1B is inverted
     EPwm8Regs.DBCTL.bit.IN_MODE = DBA_RED_DBB_FED;
     EPwm8Regs.DBRED.bit.DBRED = deadtime;              // Dead-time = DBRED / fTBCLK = 60/(200M) = 300 ns
     EPwm8Regs.DBFED.bit.DBFED = deadtime;              // dummy value for now


// Secondary side : EPWM5A/B, EPWM6A/B
// ePWM(n+1) init.  EPWM(n+1) is a slave

     EPwm9Regs.TBCTL.bit.PRDLD = TB_SHADOW;          // set shadow load
     EPwm9Regs.TBPRD = period;
     EPwm9Regs.CMPA.bit.CMPA = period;               // Fix duty at 0%
     EPwm9Regs.TBPHS.bit.TBPHS = period/5;           // Phase = 90 deg
     EPwm9Regs.TBCTR = 0;
     EPwm9Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

     EPwm9Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
     EPwm9Regs.TBCTL.bit.PHSEN = TB_ENABLE;           // Slave module
     EPwm9Regs.EPWMSYNCINSEL.bit.SEL = 7;       // Sync "flow through" mode
     EPwm9Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
     EPwm9Regs.TBCTL.bit.CLKDIV = TB_DIV1;
     EPwm9Regs.TBCTL.bit.PHSDIR = TB_UP;            // Count up on sync (=180 deg)

     EPwm9Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
     EPwm9Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
     EPwm9Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
     EPwm9Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

     EPwm9Regs.AQCTLA.bit.CAU = AQ_SET;
     EPwm9Regs.AQCTLA.bit.CAD = AQ_CLEAR;

     //EPwm9Regs.AQCTLA.bit.CAU = AQ_CLEAR;
     //EPwm9Regs.AQCTLA.bit.CAD = AQ_SET;

     // activate shadow mode for DBCTL
     EPwm9Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
     // reload on CTR = 0
     EPwm9Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

     EPwm9Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
     EPwm9Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;                        // Active Hi Complimentary
     //EPwm9Regs.DBCTL.bit.POLSEL = DB_ACTV_LOC;
     EPwm9Regs.DBCTL.bit.IN_MODE = DBA_ALL;
     EPwm9Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
     EPwm9Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now
     //----------------------------------------------------

     // ePWM(n+1) init.  EPWM(n+1) is a slave
     EPwm10Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set shadow load
     EPwm10Regs.TBPRD = period;
     EPwm10Regs.CMPA.bit.CMPA = period;             // Fix duty at 100%
     EPwm10Regs.TBPHS.bit.TBPHS = period/4;         // Phase = 180 deg
     EPwm10Regs.TBCTR = 0;
     EPwm10Regs.TBCTL.bit.FREE_SOFT = 3;            // Free run

     EPwm10Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
     EPwm10Regs.TBCTL.bit.PHSEN = TB_ENABLE;           // Slave module
     EPwm10Regs.EPWMSYNCINSEL.bit.SEL = 7;       // Sync "flow through" mode
     EPwm10Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
     EPwm10Regs.TBCTL.bit.CLKDIV = TB_DIV1;
     EPwm10Regs.TBCTL.bit.PHSDIR = TB_UP;              // Count up on sync (=180 deg)

     EPwm10Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
     EPwm10Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
     EPwm10Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
     EPwm10Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

     EPwm10Regs.AQCTLA.bit.CAU = AQ_CLEAR;
     EPwm10Regs.AQCTLA.bit.CAD = AQ_SET;

     //EPwm10Regs.AQCTLA.bit.CAU = AQ_SET;
     //EPwm10Regs.AQCTLA.bit.CAD = AQ_CLEAR;
     // activate shadow mode for DBCTL
     EPwm10Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
     // reload on CTR = 0
     EPwm10Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

     EPwm10Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
     EPwm10Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;                        // Active Hi Complimentary
     //EPwm10Regs.DBCTL.bit.POLSEL = DB_ACTV_LOC;
     EPwm10Regs.DBCTL.bit.IN_MODE = DBA_ALL;
     EPwm10Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
     EPwm10Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now
     EDIS;
}





