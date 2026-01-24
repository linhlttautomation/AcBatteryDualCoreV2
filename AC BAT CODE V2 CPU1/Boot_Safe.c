/*
 * Boot_Safe.c
 *
 *  Created on: Oct 26, 2025
 *      Author: LUU LINH K65
 */

#include "f28x_project.h"

void _DisableEpwm_Boot(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM3 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;
    EDIS;

    EPwm1Regs.DBCTL.bit.OUT_MODE = 0;
    EPwm2Regs.DBCTL.bit.OUT_MODE = 0;
    EPwm3Regs.DBCTL.bit.OUT_MODE = 0;
    EPwm4Regs.DBCTL.bit.OUT_MODE = 0;

    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm1Regs.CMPA.bit.CMPA = 0;
    EPwm1Regs.CMPB.bit.CMPB = 0;
    EPwm2Regs.CMPA.bit.CMPA = 0;
    EPwm2Regs.CMPB.bit.CMPB = 0;
    EPwm3Regs.CMPA.bit.CMPA = 0;
    EPwm3Regs.CMPB.bit.CMPB = 0;
    EPwm4Regs.CMPA.bit.CMPA = 0;
    EPwm4Regs.CMPB.bit.CMPB = 0;
}
