/*
 * CPU1_Main.c
 *
 *  Created on: 27/7/2025
 *  Author: LUU LINH K65
 */

//
// Included Files
//
#include <FLC_Setting.h>
#include <FLC_Variables.h>
#include "F28x_Project.h"
#include "f2838x_GlobalPrototypes.h"
#include <sdfm_ex1_filter_cla_sync_shared_cpu1.h>
#include "f2838x_sdfm_drivers.h"
#include "f2838x_struct.h"
#include <string.h>

//
// Defines
//
#define MAX_SAMPLES               1024
#define SDFM_PIN_MUX_OPTION1      1
#define SDFM_PIN_MUX_OPTION2      2
#define SDFM_PIN_MUX_OPTION3      3
#define WAITSTEP                  asm(" RPT #255 || NOP")

#define ENABLE_ERROR_MARCO      { EALLOW ; GpioDataRegs.GPCCLEAR.bit.GPIO92   = 1 ; EDIS; }
#define DISABLE_ERROR_MARCO     { EALLOW ; GpioDataRegs.GPCSET.bit.GPIO92 = 1 ; EDIS; }

#define ENABLE_OPER_MARCO       { EALLOW ; GpioDataRegs.GPCCLEAR.bit.GPIO93   = 1 ; EDIS; }
#define DISABLE_OPER_MARCO      { EALLOW ; GpioDataRegs.GPCSET.bit.GPIO93 = 1 ; EDIS; }

#define ENABLE_STANDBY_MARCO    { EALLOW ; GpioDataRegs.GPCCLEAR.bit.GPIO94   = 1 ; EDIS; }
#define DISABLE_STANDBY_MARCO   { EALLOW ; GpioDataRegs.GPCSET.bit.GPIO94 = 1 ; EDIS; }
//
// Globals
//
Uint16 gPeripheralNumber;

extern volatile CPU_TO_CLA CpuToCLA;
extern volatile CLA_TO_CPU ClaToCPU;

static inline void dlog1(Uint16 value);
static inline void dlog2(Uint16 value);
static inline void dlog3(Uint16 value);

#define DLOG_SIZE_1 1000
Uint16 DataLog1[DLOG_SIZE_1];
#pragma DATA_SECTION(DataLog1, "DLOG");

#define DLOG_SIZE_2 1000
Uint16 DataLog2[DLOG_SIZE_2];
#pragma DATA_SECTION(DataLog2, "DLOG");

#define DLOG_SIZE_3 1000
Uint16 DataLog3[DLOG_SIZE_3];
#pragma DATA_SECTION(DataLog3, "DLOG");

Uint16 ndx1 = 0;
Uint16 ndx2 = 0;
Uint16 ndx3 = 0;

Uint16 Task1_Isr = 0;
Uint16 Task8_Isr = 0;

Uint16 START_FLC = 0;

#pragma DATA_SECTION(data_TPC_u16, "data_TPCbuff");
volatile Uint16 data_TPC_u16[10];

Uint16 j;

typedef enum {
    FLC_OFF,
    FLC_ON
} eFLCSts;

eFLCSts e_FLC_Sts = FLC_OFF;

Uint16 ON_RELAY = 0;

PROTECT_CHANEL protect_chanel;

Uint16 wd_count = 0;

Uint16 ClrPrtFlg = 1;

Uint16 ClrPrtFLg_Fst = 0;

Uint16 RunTask8Flag = 0;

#pragma DATA_SECTION(FLC_RstFlg,"RAMGS0");
volatile Uint16 FLC_RstFlg = 1;

Uint16 Epwm1 = 0;
Uint16 AdcB0 = 0;
Uint16 count_FLC_start_up = 0;
int16 dac_val = 0;

volatile float Adc_Udc_Voltage = 0;
volatile float AdcValue_Vc = 0;
// ---------------------------------------------------

volatile float SW_Udc_Upper_Protection = 450.0f;
volatile float SW_Udc_Under_Protection = 380.0f;

volatile float CMPSS_Vg_Rms_Protection = 230.0f;
volatile float CMPSS_Ig_Rms_Protection = 6.0f;

// ---------------------------------------------------

volatile Uint32 seconds_counter_cmpss = 0;
volatile Uint32 CMPSS_Protect_Time = 0;

// CMPSS parameters for Over Current Protection FLC
Uint16  clkPrescale_1 = 6,
        sampwin_1     = 30,
        thresh_1      = 18;

//
// Function Prototypes
//
void Sdfm_configurePins(Uint16);
void Cla_initMemoryMap(void);
void CLA_initCpu1Cla(void);

void DelayUs(unsigned long us)
{
    unsigned long count;
    for(count = 0; count < (us * 200); count++) {
        __asm(" NOP");
    }
}

void DelayMs(unsigned long ms)
{
    unsigned long count = 0;
    for(count = 0; count < ms; count++)
    {
        DelayUs(1000);
    }
}

void DelayS(unsigned long s)
{
    unsigned long count = 0;
    for(count = 0; count < s; count++)
    {
        DelayMs(1000);
    }
}

void Button_STOP_Debounce(void)
{
    Uint16 stableCount = 0;
    Uint16 i;

    for(i = 0; i < 5; i++)
    {
        if(GpioDataRegs.GPBDAT.bit.GPIO48 == 0)
        {
            stableCount++;
        }
        else
        {
            stableCount = 0;
            break;
        }
        DELAY_US(100);
    }

    if(stableCount == 5)
    {
        e_FLC_Sts = FLC_OFF;
        START_FLC = 0;
    }
}

void InitCpuTimer0(void)
{
    CpuTimer0Regs.TCR.bit.TSS = 1;           // Stop timer
    CpuTimer0Regs.PRD.all     = 200000000UL - 1;  // Set period for 1s (200MHz)
    CpuTimer0Regs.TPR.all     = 0;           // No prescaler
    CpuTimer0Regs.TPRH.all    = 0;

    CpuTimer0Regs.TCR.bit.TRB = 1;           // Reload timer
    CpuTimer0Regs.TCR.bit.TIE = 1;           // Enable timer interrupt
    CpuTimer0Regs.TCR.bit.TSS = 0;           // Start timer
}

void InitCpuTimer1(void)
{
    CpuTimer1Regs.TCR.bit.TSS = 1;           // Stop timer
    CpuTimer1Regs.PRD.all     = 4000UL - 1;  // Set period for 20u (200MHz)
    CpuTimer1Regs.TPR.all     = 0;           // No prescaler
    CpuTimer1Regs.TPRH.all    = 0;

    CpuTimer1Regs.TCR.bit.TRB = 1;           // Reload timer
    CpuTimer1Regs.TCR.bit.TIE = 1;           // Enable timer interrupt
    CpuTimer1Regs.TCR.bit.TSS = 0;           // Start timer
}

void Action(void)
{
    static Uint16 initialized = 1;
    static Uint16 prevState;
    static Uint16 flcState = 0;

    Uint16 currState = GpioDataRegs.GPBDAT.bit.GPIO48;

    if (!initialized)
    {
        prevState = currState;
        initialized = 1;
    }

    if (prevState == 1 && currState == 0)
    {
        flcState = !flcState;
        e_FLC_Sts = (flcState) ? FLC_ON : FLC_OFF;
    }

    prevState = currState;
}

__interrupt void Cpu_Timer0_ISR(void)
{
    if(e_FLC_Sts == FLC_ON)
    {
        seconds_counter_cmpss++; // 1s
    }

    CpuTimer0Regs.TCR.bit.TIF = 1;          // Clear interrupt flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge interrupt
}

void SWUdcProtection(void)
{
    if (UDC_HCPL > 2.0f)
    {
        Adc_Udc_Voltage = 1.15f*800.0f*(UDC_HCPL - 2.0f)/(4096.0f - 2.0f);
    }
    else Adc_Udc_Voltage = 0.0;

//    if(VC_HCPL > 1.0)
//    {
//        AdcValue_Vc = 1.085f*600.0f*(VC_HCPL + 18.0f)/(4096.0f - 1.0f);
//    }
//    else AdcValue_Vc = 0.0;

    #if(SW_PROTECT_UDC_UPPER)
        // Udc Upper Protection
        if(Adc_Udc_Voltage > SW_Udc_Upper_Protection)
        {
            e_FLC_Sts = FLC_OFF;
            protect_chanel.Udc_upper = 1;
        }
        else
        {
            protect_chanel.Udc_under = 0;
        }
    #endif

    #if(SW_PROTECT_UDC_UNDER)
        // Udc Under Protection
        if(Adc_Udc_Voltage < SW_Udc_Under_Protection)
        {
            e_FLC_Sts = FLC_OFF;
            protect_chanel.Udc_under = 1;
        }
        else
        {
            protect_chanel.Udc_under = 0;
        }
    #endif
}

__interrupt void Cpu_Timer1_ISR(void)
{
    SWUdcProtection();

    CpuTimer0Regs.TCR.bit.TIF = 1;          // Clear interrupt flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge interrupt
}

__interrupt void epwm1_isr(void)
{
    Epwm1++;

//    AdcValue_VaG = (1.04f * (VaG_HCPL - 2425.0f) / (4096.0f - 2425.0f));
//
//    dac_val = (int16)((AdcValue_VaG + 1.0f) * 2047.5f);
//
//    if(dac_val < 0)
//    {
//        dac_val = 0;
//    }
//    if(dac_val > 4095)
//    {
//        dac_val = 4095;
//    }
//
//    DacbRegs.DACVALS.all = dac_val;


    EPwm1Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void adc_isr(void)
{
    AdcB0++;

    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

// Init DAC B
void Init_DAC_B(void)
{
    EALLOW;

    DacbRegs.DACCTL.bit.DACREFSEL = 1;   // Internal VDAC reference
    DacbRegs.DACCTL.bit.LOADMODE  = 0;   // Immediate load
    DacbRegs.DACCTL.bit.SYNCSEL   = 0;   // No sync
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;  // Enable DAC output
    EDIS;

    DacbRegs.DACVALS.all = 0;
}

void Init_DAC(void)
{
    EALLOW;

    DacbRegs.DACCTL.bit.DACREFSEL = 1;

    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;

    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 3;

    EDIS;
}

// Init ADC B
void Init_ADC_B()
{
    Uint16 i;

    EALLOW;

    //
    //write configurations
    //
    AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcbRegs.ADCCTL2.bit.RESOLUTION = 0;  // 12-bit resolution
    AdcbRegs.ADCCTL2.bit.SIGNALMODE = 0;  // Single-ended mode
    //
    //Set pulse positions to late
    //
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    //
    //power up the ADC
    //
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;

    //
    //delay for > 1ms to allow ADC time to power up
    //
    for(i = 0; i < 1000; i++)
    {
        asm("   RPT#255 || NOP");
    }
    EDIS;

    EALLOW;

    // Iz_adc
    AdcbRegs.ADCSOC2CTL.bit.CHSEL = 0;          //SOC2 will convert pin B0 -> Iz_adc
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

    // Udc
    AdcbRegs.ADCSOC1CTL.bit.CHSEL = 1;          //SOC1 will convert pin B1 -> Udc
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
    AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

    // Ic_adc
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 2;          //SOC0 will convert pin B2 -> Ic_adc
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

    // Trigger CLA
    AdcbRegs.ADCINTSOCSEL1.all = 0x0000;        // No ADCInterrupt will trigger SOCx
    AdcbRegs.ADCINTSOCSEL2.all = 0x0000;
    AdcbRegs.ADCINTSEL1N2.bit.INT1SEL = 0;      // EOC1 is trigger for ADCINT1
    AdcbRegs.ADCINTSEL1N2.bit.INT1E = 1;        // enable ADC interrupt 1
    AdcbRegs.ADCINTSEL1N2.bit.INT1CONT = 1;     // ADCINT1 pulses are generated whenever an EOC pulse is generated irrespective of whether the flag bit is cleared or not.
                                                // 0 No further ADCINT2 pulses are generated until ADCINT2 flag (in ADCINTFLG register) is cleared by user.
    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      //make sure INT1 flag is cleared
    EDIS;
}

void Init_ADC_D()
{
    Uint16 i;

    EALLOW;

    //
    //write configurations
    //
    AdcdRegs.ADCCTL2.bit.PRESCALE = 6;    //set ADCCLK divider to /4
    AdcdRegs.ADCCTL2.bit.RESOLUTION = 0;  // 12-bit resolution
    AdcdRegs.ADCCTL2.bit.SIGNALMODE = 0;  // Single-ended mode

    //
    //Set pulse positions to late
    //
    AdcdRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    //
    //power up the ADC
    //
    AdcdRegs.ADCCTL1.bit.ADCPWDNZ = 1;

    //
    //delay for > 1ms to allow ADC time to power up
    //
    for(i = 0; i < 1000; i++)
    {
        asm("   RPT#255 || NOP");
    }
    EDIS;

    EALLOW;

    // Ia_adc
    AdcdRegs.ADCSOC0CTL.bit.CHSEL = 0;          //SOC0 will convert pin D0 -> Ib_adc
    AdcdRegs.ADCSOC0CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
    AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

    // Ib_adc
    AdcdRegs.ADCSOC1CTL.bit.CHSEL = 1;          //SOC1 will convert pin D1 -> Ia_adc
    AdcdRegs.ADCSOC1CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
    AdcdRegs.ADCSOC1CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

    // VaN
    AdcdRegs.ADCSOC2CTL.bit.CHSEL = 2;          //SOC2 will convert pin D2 -> VaN
    AdcdRegs.ADCSOC2CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
    AdcdRegs.ADCSOC2CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

    // VcN
    AdcdRegs.ADCSOC3CTL.bit.CHSEL = 3;          //SOC3 will convert pin D3 -> VcN
    AdcdRegs.ADCSOC3CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
    AdcdRegs.ADCSOC3CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

    // VbN
    AdcdRegs.ADCSOC4CTL.bit.CHSEL = 4;          //SOC4 will convert pin D4 -> VbN
    AdcdRegs.ADCSOC4CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
    AdcdRegs.ADCSOC4CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

    EDIS;
}

void CMPSS_Protection_FLC(void)
{
    EALLOW;
    // Cmpss Protect for TPC
        EPwmXbarRegs.TRIP4MUX0TO15CFG.all  = 0x0000;
        EPwmXbarRegs.TRIP4MUX16TO31CFG.all = 0x0000;
        EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX0  = 0;
        EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX2  = 0;
        EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX6  = 0;
    //    EPwmXbarRegs.TRIP5MUX0TO15CFG.bit.MUX6  = 0;

        EPwmXbarRegs.TRIP4MUXENABLE.all = 0x0000;
        EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX0  = 1;
        EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX2  = 1;
        EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX6  = 1;
    //    EPwmXbarRegs.TRIP5MUXENABLE.bit.MUX6  = 1;
    //-----------------------------------------------------
    #if(CMPSS_PROTECT_VaG_UPPER == 1)
        Cmpss8Regs.COMPCTL.bit.COMPDACE = 1;
        Cmpss8Regs.COMPCTL.bit.COMPHSOURCE = 0;
        Cmpss8Regs.COMPDACCTL.bit.DACSOURCE = 0;
        Cmpss8Regs.COMPDACCTL.bit.SWLOADSEL = 0;

        // VaG Upper protection
        Cmpss8Regs.DACHVALS.bit.DACVAL = (CpuToCLA.ADCoffset_VaG + (CMPSS_Vg_Rms_Protection*can2/400.0)*CpuToCLA.ADCoffset_VaG + 50)/1.1;
        Cmpss8Regs.COMPCTL.bit.COMPLINV = 0;
        Cmpss8Regs.COMPCTL.bit.CTRIPLSEL = 2;

        EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX14 = 0 ; // Cmpss8 trip H
        EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX14 = 1; // VagH

        Cmpss8Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale_1; // Set time between samples, max : 1023
        Cmpss8Regs.CTRIPHFILCTL.bit.SAMPWIN        = sampwin_1; // # Of samples in window, max : 31
        Cmpss8Regs.CTRIPHFILCTL.bit.THRESH         = thresh_1; // Recommended : thresh > sampwin/2
        Cmpss8Regs.CTRIPHFILCTL.bit.FILINIT        = 1; // Init samples to filter input value
        Cmpss8Regs.COMPSTSCLR.bit.HLATCHCLR = 1; // Clear the status register for latched comparator events
    #endif
    //-----------------------------------------------------
    #if(CMPSS_PROTECT_VaG_LOWER == 1)
        Cmpss8Regs.COMPCTL.bit.COMPDACE = 1;
        Cmpss8Regs.COMPCTL.bit.COMPLSOURCE = 0;
        Cmpss8Regs.COMPDACCTL.bit.DACSOURCE = 0;
        Cmpss8Regs.COMPDACCTL.bit.SWLOADSEL = 0;

        // VaG Lower protecion
        Cmpss8Regs.DACLVALS.bit.DACVAL = (CpuToCLA.ADCoffset_VaG - (CMPSS_Vg_Rms_Protection*can2/400.0)*CpuToCLA.ADCoffset_VaG + 460)/1.1;
        Cmpss8Regs.COMPCTL.bit.COMPLINV = 1;
        Cmpss8Regs.COMPCTL.bit.CTRIPLSEL = 2;

        EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX15 = 0 ; // Cmpss8 trip L
        EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX15  = 1; // VagL

        Cmpss8Regs.CTRIPLFILCLKCTL.bit.CLKPRESCALE = clkPrescale_1; // Set time between samples, max : 1023
        Cmpss8Regs.CTRIPLFILCTL.bit.SAMPWIN        = sampwin_1; // # Of samples in window, max : 31
        Cmpss8Regs.CTRIPLFILCTL.bit.THRESH         = thresh_1; // Recommended : thresh > sampwin/2
        Cmpss8Regs.CTRIPLFILCTL.bit.FILINIT        = 1; // Init samples to filter input value
        Cmpss8Regs.COMPSTSCLR.bit.LLATCHCLR = 1; // Clear the status register for latched comparator events
    #endif

    //-----------------------------------------------------
    #if(CMPSS_PROTECT_Ia_inv_UPPER == 1)
        Cmpss7Regs.COMPCTL.bit.COMPDACE = 1;
        Cmpss7Regs.COMPCTL.bit.COMPHSOURCE = 0;
        Cmpss7Regs.COMPDACCTL.bit.DACSOURCE = 0;
        Cmpss7Regs.COMPDACCTL.bit.SWLOADSEL = 0;

        // Ia Upper protection
        Cmpss7Regs.DACHVALS.bit.DACVAL = (CpuToCLA.ADCoffset_Ia_inv + (CMPSS_Ig_Rms_Protection*can2/10.0)*CpuToCLA.ADCoffset_Ia_inv + 150)/1.1;
        Cmpss7Regs.COMPCTL.bit.COMPHINV = 0;
        Cmpss7Regs.COMPCTL.bit.CTRIPHSEL = 2;

        EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX12 = 0 ; // Cmpss7 trip H
        EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX12  = 1; // IaH

        // High protect
        Cmpss7Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale_1; // Set time between samples, max : 1023
        Cmpss7Regs.CTRIPHFILCTL.bit.SAMPWIN        = sampwin_1; // # Of samples in window, max : 31
        Cmpss7Regs.CTRIPHFILCTL.bit.THRESH         = thresh_1; // Recommended : thresh > sampwin/2
        Cmpss7Regs.CTRIPHFILCTL.bit.FILINIT        = 1; // Init samples to filter input value
        Cmpss7Regs.COMPSTSCLR.bit.HLATCHCLR = 1; // Clear the status register for latched comparator events
    #endif
    //-----------------------------------------------------
    #if(CMPSS_PROTECT_Ia_inv_LOWER == 1)
        Cmpss7Regs.COMPCTL.bit.COMPDACE = 1;
        Cmpss7Regs.COMPCTL.bit.COMPLSOURCE = 0;
        Cmpss7Regs.COMPDACCTL.bit.DACSOURCE = 0;
        Cmpss7Regs.COMPDACCTL.bit.SWLOADSEL = 0;

        // Ia Lower protecion
        Cmpss7Regs.DACLVALS.bit.DACVAL = (CpuToCLA.ADCoffset_Ia_inv - (CMPSS_Ig_Rms_Protection*can2/10.0)*CpuToCLA.ADCoffset_Ia_inv + 270)/1.1;
        Cmpss7Regs.COMPCTL.bit.COMPLINV = 1;
        Cmpss7Regs.COMPCTL.bit.CTRIPLSEL = 2;

        EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX13 = 0; // Cmpss7 trip L
        EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX13  = 1; // IaL

        Cmpss7Regs.CTRIPLFILCLKCTL.bit.CLKPRESCALE = clkPrescale_1; // Set time between samples, max : 1023
        Cmpss7Regs.CTRIPLFILCTL.bit.SAMPWIN        = sampwin_1; // # Of samples in window, max : 31
        Cmpss7Regs.CTRIPLFILCTL.bit.THRESH         = thresh_1; // Recommended : thresh > sampwin/2
        Cmpss7Regs.CTRIPLFILCTL.bit.FILINIT        = 1; // Init samples to filter input value
        Cmpss7Regs.COMPSTSCLR.bit.LLATCHCLR = 1; // Clear the status register for latched comparator events
    #endif

    //-----------------------------------------------------
    #if(CMPSS_PROTECT_Ic_inv_UPPER == 1)
        Cmpss3Regs.COMPCTL.bit.COMPDACE = 1;
        Cmpss3Regs.COMPCTL.bit.COMPHSOURCE = 0;
        Cmpss3Regs.COMPDACCTL.bit.DACSOURCE = 0;
        Cmpss3Regs.COMPDACCTL.bit.SWLOADSEL = 0;

        // Ic Upper protection
        Cmpss3Regs.DACHVALS.bit.DACVAL = (CpuToCLA.ADCoffset_Ic_inv + (CMPSS_Ig_Rms_Protection*can2/10.0)*CpuToCLA.ADCoffset_Ic_inv + 150)/1.1;
        Cmpss3Regs.COMPCTL.bit.COMPHINV = 0;
        Cmpss3Regs.COMPCTL.bit.CTRIPHSEL = 2;

        EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX4 = 0 ; // Cmpss3 trip H
        EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX4  = 1; // IcH

        // High protect
        Cmpss3Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale_1; // Set time between samples, max : 1023
        Cmpss3Regs.CTRIPHFILCTL.bit.SAMPWIN        = sampwin_1; // # Of samples in window, max : 31
        Cmpss3Regs.CTRIPHFILCTL.bit.THRESH         = thresh_1; // Recommended : thresh > sampwin/2
        Cmpss3Regs.CTRIPHFILCTL.bit.FILINIT        = 1; // Init samples to filter input value
        Cmpss3Regs.COMPSTSCLR.bit.HLATCHCLR = 1; // Clear the status register for latched comparator events
    #endif
    //-----------------------------------------------------
    #if(CMPSS_PROTECT_Ic_inv_LOWER == 1)
        Cmpss3Regs.COMPCTL.bit.COMPDACE = 1;
        Cmpss3Regs.COMPCTL.bit.COMPLSOURCE = 0;
        Cmpss3Regs.COMPDACCTL.bit.DACSOURCE = 0;
        Cmpss3Regs.COMPDACCTL.bit.SWLOADSEL = 0;

        // Ic Lower protecion
        Cmpss3Regs.DACLVALS.bit.DACVAL = (CpuToCLA.ADCoffset_Ic_inv - (CMPSS_Ig_Rms_Protection*can2/10.0)*CpuToCLA.ADCoffset_Ic_inv + 250)/1.1;
        Cmpss3Regs.COMPCTL.bit.COMPLINV = 1;
        Cmpss3Regs.COMPCTL.bit.CTRIPLSEL = 2;

        EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX5 = 0 ; // Cmpss3 trip L
        EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX5  = 1; // IcL

        Cmpss3Regs.CTRIPLFILCLKCTL.bit.CLKPRESCALE = clkPrescale_1; // Set time between samples, max : 1023
        Cmpss3Regs.CTRIPLFILCTL.bit.SAMPWIN        = sampwin_1; // # Of samples in window, max : 31
        Cmpss3Regs.CTRIPLFILCTL.bit.THRESH         = thresh_1; // Recommended : thresh > sampwin/2
        Cmpss3Regs.CTRIPLFILCTL.bit.FILINIT        = 1; // Init samples to filter input value
        Cmpss3Regs.COMPSTSCLR.bit.LLATCHCLR = 1; // Clear the status register for latched comparator events
    #endif
    //-----------------------------------------------------

    // DC Trip select
    EPwm1Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3 ; // Tripin4
    EPwm1Regs.TZDCSEL.bit.DCAEVT1 = 4 ; // DCAL high , DCAH don't care
    EPwm1Regs.DCTRIPSEL.bit.DCALCOMPSEL = 3 ; // Tripin4

    EPwm2Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3 ; // Tripin4
    EPwm2Regs.TZDCSEL.bit.DCAEVT1 = 4 ; // DCAL high , DCAH don't care
    EPwm2Regs.DCTRIPSEL.bit.DCALCOMPSEL = 3 ; // Tripin4

    EPwm3Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3 ; // Tripin4
    EPwm3Regs.TZDCSEL.bit.DCAEVT1 = 4 ; // DCAL high , DCAH don't care
    EPwm3Regs.DCTRIPSEL.bit.DCALCOMPSEL = 3 ; // Tripin4

    EPwm4Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3 ; // Tripin4
    EPwm4Regs.TZDCSEL.bit.DCAEVT1 = 4 ; // DCAL high , DCAH don't care
    EPwm4Regs.DCTRIPSEL.bit.DCALCOMPSEL = 3 ; // Tripin4

    // Tripzone Select
    EPwm1Regs.TZSEL.bit.DCAEVT1 = 1;
    EPwm2Regs.TZSEL.bit.DCAEVT1 = 1;
    EPwm3Regs.TZSEL.bit.DCAEVT1 = 1;
    EPwm4Regs.TZSEL.bit.DCAEVT1 = 1;

    EPwm1Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm1Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    EPwm2Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm2Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    EPwm3Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm3Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    EPwm4Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm4Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    // Clear any spurious OV trip
    EPwm1Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm2Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm3Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm4Regs.TZCLR.bit.DCAEVT1 = 1;

    EDIS;
}

void ClearProtectFlagFcn(void)
{
    EALLOW;

    Cmpss8Regs.COMPSTSCLR.bit.HLATCHCLR  = 1;  // Clear latched status
    Cmpss8Regs.COMPSTSCLR.bit.LLATCHCLR  = 1;  // Clear latched status

    Cmpss7Regs.COMPSTSCLR.bit.HLATCHCLR  = 1;  // Clear latched status
    Cmpss7Regs.COMPSTSCLR.bit.LLATCHCLR  = 1;  // Clear latched status

    Cmpss3Regs.COMPSTSCLR.bit.HLATCHCLR  = 1;  // Clear latched status
    Cmpss3Regs.COMPSTSCLR.bit.LLATCHCLR  = 1;  // Clear latched status

    // Clear EPWM1 Trip Zone flags
    EPwm1Regs.TZCLR.bit.OST    = 1;    // One-shot trip clear
    EPwm1Regs.TZCLR.bit.CBC    = 1;    // Cycle-by-cycle trip clear
    EPwm1Regs.TZCLR.bit.INT    = 1;    // Interrupt clear
    EPwm1Regs.TZCLR.bit.DCAEVT1 = 1;   // Digital Compare A Event 1 clear

    // Clear EPWM2 Trip Zone flags
    EPwm2Regs.TZCLR.bit.OST    = 1;
    EPwm2Regs.TZCLR.bit.CBC    = 1;
    EPwm2Regs.TZCLR.bit.INT    = 1;
    EPwm2Regs.TZCLR.bit.DCAEVT1 = 1;

    // Clear EPWM3 Trip Zone flags
    EPwm3Regs.TZCLR.bit.OST    = 1;
    EPwm3Regs.TZCLR.bit.CBC    = 1;
    EPwm3Regs.TZCLR.bit.INT    = 1;
    EPwm3Regs.TZCLR.bit.DCAEVT1 = 1;

    // Clear EPWM4 Trip Zone flags
    EPwm4Regs.TZCLR.bit.OST    = 1;
    EPwm4Regs.TZCLR.bit.CBC    = 1;
    EPwm4Regs.TZCLR.bit.INT    = 1;
    EPwm4Regs.TZCLR.bit.DCAEVT1 = 1;

    EPwm1Regs.TZCLR.all = 0xFFFF;
    EPwm2Regs.TZCLR.all = 0xFFFF;
    EPwm3Regs.TZCLR.all = 0xFFFF;
    EPwm4Regs.TZCLR.all = 0xFFFF;

    EDIS;
}

void DisableEpwm(void)
{
    EPwm1Regs.DBCTL.bit.OUT_MODE = 0;
    EPwm3Regs.DBCTL.bit.OUT_MODE = 0;
    EPwm2Regs.DBCTL.bit.OUT_MODE = 0;
    EPwm4Regs.DBCTL.bit.OUT_MODE = 0;

    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm1Regs.CMPA.bit.CMPA = 0;
    EPwm1Regs.CMPB.bit.CMPB = 0;

    EPwm3Regs.CMPA.bit.CMPA = 0;
    EPwm3Regs.CMPB.bit.CMPB = 0;

    EPwm2Regs.CMPA.bit.CMPA = 0;
    EPwm2Regs.CMPB.bit.CMPB = 0;

    EPwm4Regs.CMPA.bit.CMPA = 0;
    EPwm4Regs.CMPB.bit.CMPB = 0;
}

void UpdateProtectValue(void)
{
    EALLOW;

    EDIS;
}

//void Boot_CPU2_FromFlash(void)
//{
//    EALLOW;
//    DevCfgRegs.CPU2RESCTL.bit.RESET = 1;
//
//    DevCfgRegs.CPU2RESCTL.bit.RESET = 0;
//    EDIS;
//}

//
// Main
// linh ta
int main(void)
{
    Uint16  pinMuxoption;
    Uint16  HLT, LLT;

    int period = 1000;
    int duty_relay = 2500;
    int duty_relay_current = 2500;
    int relay_activated = 0;

    #if(SET_MODE_RUN == THREE_PHASE_MODE)
        int deadtime = 60;
    #endif
#if(SET_MODE_RUN == SINGLE_PHASE_MODE)
    int deadtime = 60;
#endif
    //
    // Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the f2838x_sysctrl.c file.
    //

//    SetAllEPwmPinsAsInput();

    InitSysCtrl();

//    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
//    InitFlash();
//
//    Boot_CPU2_FromFlash();

    for(ndx1=0; ndx1<DLOG_SIZE_1; ndx1++)
    {
        DataLog1[ndx1] = 0;
    }
    ndx1 = 0;

    for(ndx2=0; ndx2<DLOG_SIZE_2; ndx2++)
    {
        DataLog2[ndx2] = 0;
    }
    ndx2 = 0;

    for(ndx3=0; ndx3<DLOG_SIZE_3; ndx3++)
    {
        DataLog3[ndx3] = 0;
    }
    ndx3 = 0;

    Init_ADC_B();
    Init_ADC_D();

    #if(ALLLOW_DAC == 1)
        Init_DAC_B();
    #endif

    EALLOW;

       CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
       CpuSysRegs.PCLKCR2.bit.EPWM3 = 1;
       CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
       CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;
       CpuSysRegs.PCLKCR2.bit.EPWM6 = 1;

       CpuSysRegs.PCLKCR13.bit.ADC_B = 1;
       CpuSysRegs.PCLKCR13.bit.ADC_D = 1;

       CpuSysRegs.PCLKCR14.bit.CMPSS3 = 1;
       CpuSysRegs.PCLKCR14.bit.CMPSS7 = 1;
       CpuSysRegs.PCLKCR14.bit.CMPSS8 = 1;

       #if(ALLLOW_DAC == 1)
           CpuSysRegs.PCLKCR16.bit.DAC_B = 1;
       #endif

    EDIS;

    EALLOW;

        DevCfgRegs.CPUSEL0.bit.EPWM7 = 1; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL0.bit.EPWM8 = 1; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL0.bit.EPWM9 = 1; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL0.bit.EPWM10 = 1; // 1: CPU2, 0: CPU1

        DevCfgRegs.CPUSEL11.bit.ADC_B = 0; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL11.bit.ADC_D = 0; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL11.bit.ADC_A = 1; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL11.bit.ADC_C = 1; // 1: CPU2, 0: CPU1

        DevCfgRegs.CPUSEL12.bit.CMPSS3 = 0; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL12.bit.CMPSS7 = 0; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL12.bit.CMPSS8 = 0; // 1: CPU2, 0: CPU1

        DevCfgRegs.CPUSEL12.bit.CMPSS1 = 1; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL12.bit.CMPSS2 = 1; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL12.bit.CMPSS4 = 1; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL12.bit.CMPSS5 = 1; // 1: CPU2, 0: CPU1
        DevCfgRegs.CPUSEL12.bit.CMPSS6 = 1; // 1: CPU2, 0: CPU1

        #if(ALLLOW_DAC == 1)
            DevCfgRegs.CPUSEL14.bit.DAC_B = 0; // 1: CPU2, 0: CPU1
        #endif

    EDIS;

   EALLOW;


   //  GPIO-00 - PIN FUNCTION = PWM1A
       GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO0 = 0;

   //  GPIO-01 - PIN FUNCTION = PWM1B
       GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO1 = 0;

   //  GPIO-02 - PIN FUNCTION = PWM2A
       GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO2 = 0;

   //  GPIO-03 - PIN FUNCTION = PWM2B
       GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO3 = 0;

   //  GPIO-04 - PIN FUNCTION = PWM3A
       GpioCtrlRegs.GPAGMUX1.bit.GPIO4 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO4 = 0;

   //  GPIO-05 - PIN FUNCTION = PWM3B
       GpioCtrlRegs.GPAGMUX1.bit.GPIO5 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO5 = 0;

   //  GPIO-06 - PIN FUNCTION = PWM4A
       GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO6 = 0;

   //  GPIO-07 - PIN FUNCTION = PWM4B
       GpioCtrlRegs.GPAGMUX1.bit.GPIO7 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO7 = 0;

   //  GPIO-10 - PIN FUNCTION = GPIO10
       GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;
       GpioCtrlRegs.GPADIR.bit.GPIO10 = 0;
       GpioCtrlRegs.GPAPUD.bit.GPIO10 = 0;
       GpioCtrlRegs.GPACSEL2.bit.GPIO10 = 2;

   //  GPIO-11 - PIN FUNCTION = PWM6B
       GpioCtrlRegs.GPAGMUX1.bit.GPIO11 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO11 = 0;

   //  GPIO-12 - PIN FUNCTION = PWM7A
       GpioCtrlRegs.GPAGMUX1.bit.GPIO12 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO12 = 0;

   //  GPIO-13 - PIN FUNCTION = PWM7B
       GpioCtrlRegs.GPAGMUX1.bit.GPIO13 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO13 = 0;

   //  GPIO-14 - PIN FUNCTION = PWM 8A
       GpioCtrlRegs.GPAGMUX1.bit.GPIO14 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO14 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;

   //  GPIO-15 - PIN FUNCTION = PWM8B
       GpioCtrlRegs.GPAGMUX1.bit.GPIO15 = 0;
       GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0;

   //  GPIO-16 - PIN FUNCTION = PWM9A
       GpioCtrlRegs.GPAGMUX2.bit.GPIO16 = 1;
       GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO16 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO16 = 0;

   //  GPIO-17 - PIN FUNCTION = PWM9B
       GpioCtrlRegs.GPAGMUX2.bit.GPIO17 = 1;
       GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO17 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO17 = 0;

   //  GPIO-18 - PIN FUNCTION = PWM10A
       GpioCtrlRegs.GPAGMUX2.bit.GPIO18 = 1;
       GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO18 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;

   //  GPIO-19 - PIN FUNCTION = PWM10B
       GpioCtrlRegs.GPAGMUX2.bit.GPIO19 = 1;
       GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 1;
       GpioCtrlRegs.GPADIR.bit.GPIO19 = 1;
       GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;

   //  STOP BUTTON
       GpioCtrlRegs.GPBMUX2.bit.GPIO48 = 0;
       GpioCtrlRegs.GPBDIR.bit.GPIO48 = 0;
       GpioCtrlRegs.GPBPUD.bit.GPIO48 = 0;

   //  ERROR LED
       GpioCtrlRegs.GPCMUX2.bit.GPIO92 = 0;
       GpioCtrlRegs.GPCDIR.bit.GPIO92 = 1;
       GpioDataRegs.GPCCLEAR.bit.GPIO92 = 1;
       GpioCtrlRegs.GPCPUD.bit.GPIO92 = 0;

   //  OPER LED
       GpioCtrlRegs.GPCMUX2.bit.GPIO93 = 0;
       GpioCtrlRegs.GPCDIR.bit.GPIO93 = 1;
       GpioDataRegs.GPCCLEAR.bit.GPIO93 = 1;
       GpioCtrlRegs.GPCPUD.bit.GPIO93 = 0;

   //  STANDBY LED
       GpioCtrlRegs.GPCMUX2.bit.GPIO94 = 0;
       GpioCtrlRegs.GPCDIR.bit.GPIO94 = 1;
       GpioDataRegs.GPCCLEAR.bit.GPIO94 = 1;
       GpioCtrlRegs.GPCPUD.bit.GPIO94 = 0;

   EDIS;

   EALLOW;
       CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
   EDIS;

   #if(SET_MODE_RUN == THREE_PHASE_MODE)
       EALLOW;

       /* Time-Base Control (TBCTL) */
       EPwm1Regs.TBCTL.bit.FREE_SOFT = 3;
       EPwm1Regs.TBCTL.bit.PHSDIR = TB_DOWN;
       EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
       EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
       EPwm1Regs.TBCTL.bit.SWFSYNC = 0;
       EPwm1Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
       EPwm1Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
       EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
       EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

       EPwm3Regs.TBCTL.bit.FREE_SOFT = 3;
       EPwm3Regs.TBCTL.bit.PHSDIR = TB_DOWN;
       EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;
       EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
       EPwm3Regs.TBCTL.bit.SWFSYNC = 0;
       EPwm3Regs.EPWMSYNCINSEL.bit.SEL = 0;
       EPwm3Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
       EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
       EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

       EPwm2Regs.TBCTL.bit.FREE_SOFT = 3;
       EPwm2Regs.TBCTL.bit.PHSDIR = TB_DOWN;
       EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;
       EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
       EPwm2Regs.TBCTL.bit.SWFSYNC = 0;
       EPwm2Regs.EPWMSYNCINSEL.bit.SEL = 0;
       EPwm2Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
       EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
       EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

       EPwm4Regs.TBCTL.bit.FREE_SOFT = 3;
       EPwm4Regs.TBCTL.bit.PHSDIR = TB_DOWN;
       EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;
       EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
       EPwm4Regs.TBCTL.bit.SWFSYNC = 0;
       EPwm4Regs.EPWMSYNCINSEL.bit.SEL = 0;
       EPwm4Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
       EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;
       EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

       EPwm6Regs.TBCTL.bit.FREE_SOFT = 3;
       EPwm6Regs.TBCTL.bit.PHSDIR = TB_DOWN;
       EPwm6Regs.TBCTL.bit.CLKDIV = TB_DIV1;
       EPwm6Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
       EPwm6Regs.TBCTL.bit.SWFSYNC = 0;
       EPwm6Regs.EPWMSYNCINSEL.bit.SEL = 0;
       EPwm6Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
       EPwm6Regs.TBCTL.bit.PHSEN = TB_ENABLE;
       EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

       /* Initialization */
       EPwm1Regs.CMPA.bit.CMPA = 0;
       EPwm1Regs.TBPHS.bit.TBPHS = 0;
       EPwm1Regs.TBCTR = 0;
       EPwm1Regs.TBPRD = period;

       EPwm3Regs.CMPA.bit.CMPA = 0;
       EPwm3Regs.TBPHS.bit.TBPHS = 0;
       EPwm3Regs.TBCTR = 0;
       EPwm3Regs.TBPRD = period;

       EPwm2Regs.CMPA.bit.CMPA = 0;
       EPwm2Regs.TBPHS.bit.TBPHS = 0;
       EPwm2Regs.TBCTR = 0;
       EPwm2Regs.TBPRD = period;

       EPwm4Regs.CMPA.bit.CMPA = 0;
       EPwm4Regs.TBPHS.bit.TBPHS = 0;
       EPwm4Regs.TBCTR = 0;
       EPwm4Regs.TBPRD = period;

       EPwm6Regs.CMPA.bit.CMPA = 0;
       EPwm6Regs.TBPHS.bit.TBPHS = 0;
       EPwm6Regs.TBCTR = 0;
       EPwm6Regs.TBPRD = duty_relay;

       /* Counter-Compare (CC) */
       EPwm1Regs.CMPCTL.all = 0x000C;
       EPwm3Regs.CMPCTL.all = 0x000C;
       EPwm2Regs.CMPCTL.all = 0x000C;
       EPwm4Regs.CMPCTL.all = 0x000C;

       /* Action-Qualifier (AQ) */
       EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
       EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

       EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
       EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;

       EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
       EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;

       EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
       EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;

       /* Dead Band (DB) */
       EPwm1Regs.DBCTL.all = 0x03CB;
       EPwm1Regs.DBFED.bit.DBFED = deadtime;
       EPwm1Regs.DBRED.bit.DBRED = deadtime;

       EPwm3Regs.DBCTL.all = 0x03CB;
       EPwm3Regs.DBFED.bit.DBFED = deadtime;
       EPwm3Regs.DBRED.bit.DBRED = deadtime;

       EPwm2Regs.DBCTL.all = 0x03CB;
       EPwm2Regs.DBFED.bit.DBFED = deadtime;
       EPwm2Regs.DBRED.bit.DBRED = deadtime;

       EPwm4Regs.DBCTL.all = 0x03CB;
       EPwm4Regs.DBFED.bit.DBFED = deadtime;
       EPwm4Regs.DBRED.bit.DBRED = deadtime;

       /* Event Trigger (ET) */
       EPwm1Regs.ETSEL.bit.SOCAEN = 1;
       EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;
       EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;
       EPwm1Regs.ETCLR.bit.SOCA = 1;
       EPwm1Regs.ETPS.bit.SOCACNT = ET_1ST;

       // Enable CNT_zero interrupt using EPWM1 Time-base
       EPwm1Regs.ETSEL.bit.INTEN = 1;
       EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;
       EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;
       EPwm1Regs.ETPS.bit.INTCNT = ET_1ST;
       EPwm1Regs.ETCLR.bit.INT = 1;

       EDIS;

   #endif
#if(SET_MODE_RUN == SINGLE_PHASE_MODE)
      EALLOW;

         /* Time-Base Control (TBCTL) */
         EPwm1Regs.TBCTL.bit.FREE_SOFT = 3;
         EPwm1Regs.TBCTL.bit.PHSDIR = TB_DOWN;
         EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
         EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
         EPwm1Regs.TBCTL.bit.SWFSYNC = 0;
         EPwm1Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
         EPwm1Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
         EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
         EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

         EPwm3Regs.TBCTL.bit.FREE_SOFT = 3;
         EPwm3Regs.TBCTL.bit.PHSDIR = TB_DOWN;
         EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;
         EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
         EPwm3Regs.TBCTL.bit.SWFSYNC = 0;
         EPwm3Regs.EPWMSYNCINSEL.bit.SEL = 0;
         EPwm3Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
         EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
         EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

         EPwm2Regs.TBCTL.bit.FREE_SOFT = 3;
         EPwm2Regs.TBCTL.bit.PHSDIR = TB_DOWN;
         EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;
         EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
         EPwm2Regs.TBCTL.bit.SWFSYNC = 0;
         EPwm3Regs.EPWMSYNCINSEL.bit.SEL = 0;
         EPwm2Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
         EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
         EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

         EPwm4Regs.TBCTL.bit.FREE_SOFT = 3;
         EPwm4Regs.TBCTL.bit.PHSDIR = TB_DOWN;
         EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;
         EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
         EPwm4Regs.TBCTL.bit.SWFSYNC = 0;
         EPwm3Regs.EPWMSYNCINSEL.bit.SEL = 0;
         EPwm4Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
         EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;
         EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

         EPwm6Regs.TBCTL.bit.FREE_SOFT = 3;
         EPwm6Regs.TBCTL.bit.PHSDIR = TB_DOWN;
         EPwm6Regs.TBCTL.bit.CLKDIV = TB_DIV1;
         EPwm6Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
         EPwm6Regs.TBCTL.bit.SWFSYNC = 0;
         EPwm3Regs.EPWMSYNCINSEL.bit.SEL = 0;
         EPwm6Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
         EPwm6Regs.TBCTL.bit.PHSEN = TB_ENABLE;
         EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

         /* Initialization */
         EPwm1Regs.CMPA.bit.CMPA = period;
         EPwm1Regs.TBPHS.bit.TBPHS = 0;
         EPwm1Regs.TBCTR = 0;
         EPwm1Regs.TBPRD = period;

         EPwm3Regs.CMPA.bit.CMPA = period;
         EPwm3Regs.TBPHS.bit.TBPHS = 0;
         EPwm3Regs.TBCTR = 0;
         EPwm3Regs.TBPRD = period;

         EPwm2Regs.CMPA.bit.CMPA = period;
         EPwm2Regs.TBPHS.bit.TBPHS = period;
         EPwm2Regs.TBCTR = 0;
         EPwm2Regs.TBPRD = period;

         EPwm4Regs.CMPA.bit.CMPA = period;
         EPwm4Regs.TBPHS.bit.TBPHS = period;
         EPwm4Regs.TBCTR = 0;
         EPwm4Regs.TBPRD = period;

         EPwm6Regs.CMPA.bit.CMPA = 0;
         EPwm6Regs.TBPHS.bit.TBPHS = 0;
         EPwm6Regs.TBCTR = 0;
         EPwm6Regs.TBPRD = duty_relay;
         /* Counter-Compare (CC) */
         EPwm1Regs.CMPCTL.all = 0x000C;
         EPwm3Regs.CMPCTL.all = 0x000C;
         EPwm2Regs.CMPCTL.all = 0x000C;
         EPwm4Regs.CMPCTL.all = 0x000C;

         EPwm1Regs.CMPCTL.bit.LOADAMODE = 0x01;
         EPwm3Regs.CMPCTL.bit.LOADAMODE = 0x01;
         EPwm2Regs.CMPCTL.bit.LOADAMODE = 0x01;
         EPwm4Regs.CMPCTL.bit.LOADAMODE = 0x01;

         /* Action-Qualifier (AQ) */
         EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
         EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

         EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
         EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;

         EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
         EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;

         EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
         EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;

         /* Dead Band (DB) */
         EPwm1Regs.DBCTL.all = 0x03CB;
         EPwm1Regs.DBFED.bit.DBFED = deadtime;
         EPwm1Regs.DBRED.bit.DBRED = deadtime;

         EPwm3Regs.DBCTL.all = 0x03CB;
         EPwm3Regs.DBFED.bit.DBFED = deadtime;
         EPwm3Regs.DBRED.bit.DBRED = deadtime;

         EPwm2Regs.DBCTL.all = 0x03CB;
         EPwm2Regs.DBFED.bit.DBFED = deadtime;
         EPwm2Regs.DBRED.bit.DBRED = deadtime;

         EPwm4Regs.DBCTL.all = 0x03CB;
         EPwm4Regs.DBFED.bit.DBFED = deadtime;
         EPwm4Regs.DBRED.bit.DBRED = deadtime;


         /* Event Trigger (ET) */
         EPwm1Regs.ETSEL.bit.SOCAEN = 1;
         EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;
         EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;
         EPwm1Regs.ETCLR.bit.SOCA = 1;
         EPwm1Regs.ETPS.bit.SOCACNT = ET_1ST;

         // Enable CNT_zero interrupt using EPWM1 Time-base
         EPwm1Regs.ETSEL.bit.INTEN = 1;
         EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;
         EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;
         EPwm1Regs.ETPS.bit.INTCNT = ET_1ST;
         EPwm1Regs.ETCLR.bit.INT = 1;
         EDIS;
  #endif

    CpuToCLA.ADCoffset_Udc = 2.0f;
    CpuToCLA.ADCoffset_VaG = 2425.0f;
    CpuToCLA.ADCoffset_VbG = 2419.0f;
    CpuToCLA.ADCoffset_VcG = 2408.5f;
    CpuToCLA.ADCoffset_Ia_inv = 1850.0f;
    CpuToCLA.ADCoffset_Ib_inv = 1847.0f;
    CpuToCLA.ADCoffset_Ic_inv = 1852.0f;

    CpuToCLA.ADCgain_Udc = 1.15f;
    CpuToCLA.ADCgain_VaG = 1.04f;
    CpuToCLA.ADCgain_VbG = 1.04f;
    CpuToCLA.ADCgain_VcG = 1.04f;
    CpuToCLA.ADCgain_Ia_inv = 1.55f;
    CpuToCLA.ADCgain_Ib_inv = 1.5f;
    CpuToCLA.ADCgain_Ic_inv = 1.53f;

    CMPSS_Protection_FLC();

    EALLOW;
        CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    #if(ALLOW_TIMER)
        InitCpuTimer0();
        InitCpuTimer1();
    #endif

    //
    // Clear all __interrupts and initialize PIE vector table:
    // Disable CPU __interrupts
    //
    DINT;

    //
    // Initialize PIE control registers to their default state.
    // The default state is all PIE __interrupts disabled and flags
    // are cleared.
    // This function is found in the f2838x_piectrl.c file.
    //
    InitPieCtrl();

    //
    // Disable CPU __interrupts and clear all CPU __interrupt flags:
    //
    IER = 0x0000;
    IFR = 0x0000;

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the __interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in f2838x_sysctrl.c.
    // This function is found in f2838x_sysctrl.c.
    //
    InitPieVectTable();

    //
    // Enable CPU INT11 which is connected to CLA
    //
    IER |= M_INT11;

    #if(ALLOW_TIMER == 1)
        EALLOW;
        PieVectTable.TIMER0_INT = &Cpu_Timer0_ISR;  // ISR for Timer0
        EDIS;

        EALLOW;
        PieVectTable.TIMER1_INT = &Cpu_Timer1_ISR;  // ISR for Timer1
        EDIS;
    #endif

    #if(ALLOW_EPWM_INT == 1)
        EALLOW;
        PieVectTable.EPWM1_INT = &epwm1_isr;
        EDIS;
    #endif

    #if(ALLOW_ADC_INT == 1)
        EALLOW;
        PieVectTable.ADCB1_INT = &adc_isr;
        EDIS;
    #endif

    #if(ALLOW_TIMER == 1)
        PieCtrlRegs.PIEIER1.bit.INTx7 = 1;  // Timer0 interrupt = Group 1, INT7
        IER |= M_INT1;                      // Enable group 1 interrupt
        EINT;                               // Enable global interrupt

        PieCtrlRegs.PIEIER1.bit.INTx7 = 1;  // Timer0 interrupt = Group 1, INT7
        IER |= M_INT13;                     // Enable group 1 interrupt
        EINT;                               // Enable global interrupt
    #endif

    #if(ALLOW_EPWM_INT == 1)
        EALLOW;
        IER |= M_INT3;
        PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
        EDIS;
    #endif

    EINT;

#ifdef CPU1
    pinMuxoption = SDFM_PIN_MUX_OPTION2;

    //
    // Configure GPIO pins as SDFM pins
    //
    //Sdfm_configurePins(pinMuxoption);

    CONNECT_SD1(TO_CPU1);         //Connect SDFM1 to CPU1
    CONNECT_SD2(TO_CPU2);         //Connect SDFM2 to CPU2
#endif

    //
    // Configure the CLA memory spaces
    //
    Cla_initMemoryMap();

#ifdef CPU1
    //
    // Configure the CLA task vectors for CPU1
    //
    CLA_initCpu1Cla();
#endif
#ifdef CPU2
    //
    // Configure the CLA task vectors for CPU2
    //
    CLA_initCpu2Cla();
#endif

    Cla1ForceTask8andWait();
    WAITSTEP;

    EALLOW;

    //
    // Trigger Source for TASK1 of CLA1 = ADCBINT1
    //
    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK1 = CLA_TRIG_ADCBINT1;

    //
    // Lock CLA1TASKSRCSEL1 register
    //
    DmaClaSrcSelRegs.CLA1TASKSRCSELLOCK.bit.CLA1TASKSRCSEL1 = 1;
    EDIS;

    gPeripheralNumber = SDFM1; // Select SDFM1

    //
    // Input Control Module
    //
    // Configure Input Control Mode: Modulator Clock rate = Modulator data rate
    //
    Sdfm_configureInputCtrl(gPeripheralNumber, FILTER1, MODE_0);
    Sdfm_configureInputCtrl(gPeripheralNumber, FILTER2, MODE_0);
    Sdfm_configureInputCtrl(gPeripheralNumber, FILTER3, MODE_0);
    Sdfm_configureInputCtrl(gPeripheralNumber, FILTER4, MODE_0);

    //
    // Comparator Module
    //
    HLT = 0x7FFF;    // Over value threshold settings
    LLT = 0x0000;    // Under value threshold settings

    //
    // Configure Comparator module's comparator filter type and comparator's OSR
    // value, higher threshold, lower threshold
    //
    Sdfm_configureComparator(gPeripheralNumber, FILTER1, SINC3, OSR_32,
                             HLT, LLT);
    Sdfm_configureComparator(gPeripheralNumber, FILTER2, SINC3, OSR_32,
                             HLT, LLT);
    Sdfm_configureComparator(gPeripheralNumber, FILTER3, SINC3, OSR_32,
                             HLT, LLT);
    Sdfm_configureComparator(gPeripheralNumber, FILTER4, SINC3, OSR_32,
                             HLT, LLT);

    //
    // Data filter Module
    //
    // Configure Data filter modules filter type, OSR value and
    // enable / disable data filter
    //
    Sdfm_configureData_filter(gPeripheralNumber, FILTER1, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_16_BIT, SHIFT_10_BITS);
    Sdfm_configureData_filter(gPeripheralNumber, FILTER2, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_16_BIT, SHIFT_10_BITS);
    Sdfm_configureData_filter(gPeripheralNumber, FILTER3, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_16_BIT, SHIFT_10_BITS);
    Sdfm_configureData_filter(gPeripheralNumber, FILTER4, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_16_BIT, SHIFT_10_BITS);

    //
    // Enable Master filter bit: Unless this bit is set none of the filter
    // modules can be enabled.
    // All the filter modules are synchronized when master filter bit is enabled
    // after individual filter modules are enabled.
    //
    Sdfm_enableMFE(gPeripheralNumber);

    //
    // PWM11.CMPC, PWM11.CMPD signals can synchronize SDFM1 filters and
    // PWM12.CMPC and PWM12.CMPD signals can synchronize SDFM2 filters. This
    // option is not being used in this example for SDFM1.
    //
    Sdfm_configureExternalreset(gPeripheralNumber,FILTER_1_EXT_RESET_DISABLE,
                                FILTER_2_EXT_RESET_DISABLE,
                                FILTER_3_EXT_RESET_DISABLE,
                                FILTER_4_EXT_RESET_DISABLE);

    //
    // Enable interrupts
    //
    // Following SDFM interrupts can be enabled / disabled using this function.
    //  Enable / disable comparator high threshold
    //  Enable / disable comparator low threshold
    //  Enable / disable modulator clock failure
    //  Enable / disable filter acknowledge
    //
    Sdfm_configureInterrupt(gPeripheralNumber, FILTER1, IEH_DISABLE,
                            IEL_DISABLE, MFIE_ENABLE, AE_ENABLE);
    Sdfm_configureInterrupt(gPeripheralNumber, FILTER2, IEH_DISABLE,
                            IEL_DISABLE, MFIE_ENABLE, AE_ENABLE);
    Sdfm_configureInterrupt(gPeripheralNumber, FILTER3, IEH_DISABLE,
                            IEL_DISABLE, MFIE_ENABLE, AE_ENABLE);
    Sdfm_configureInterrupt(gPeripheralNumber, FILTER4, IEH_DISABLE,
                            IEL_DISABLE, MFIE_ENABLE, AE_ENABLE);

    //
    // Enable master interrupt so that any of the filter interrupts can trigger
    // SDFM interrupt to CPU
    //
    Sdfm_enableMIE(gPeripheralNumber);

    EINT;
    ERTM;

    CpuToCLA.EnableADC = 0;
    DelayMs(100);
    CpuToCLA.EnableADC = 1;
    CpuToCLA.EnableFlag = 0;

    #if(BUILDLEVEL == LEVEL4)

        #if(SET_MODE_CONTROL == VAC_LOAD_64_FLC_30)
            CpuToCLA.VdTesting = VAC_LOAD_64_FLC_30;
        #endif

        #if(SET_MODE_CONTROL == VAC_LOAD_64_FLC_60)
            CpuToCLA.VdTesting = VAC_LOAD_64_FLC_60;
        #endif

        #if(SET_MODE_CONTROL == VAC_LOAD_64_FLC_100)
            CpuToCLA.VdTesting = VAC_LOAD_64_FLC_100;
        #endif

        #if(SET_MODE_CONTROL == VAC_LOAD_64_FLC_103)
            CpuToCLA.VdTesting = VAC_LOAD_64_FLC_103;
        #endif

    #endif

    CpuToCLA.IdTesting = 1.0f;

    CpuToCLA.ADCoffset_Udc = 2.0f;
    CpuToCLA.ADCoffset_VaG = 2425.0f;
    CpuToCLA.ADCoffset_VbG = 2419.0f;
    CpuToCLA.ADCoffset_VcG = 2408.5f;
    CpuToCLA.ADCoffset_Ia_inv = 1850.0f;
    CpuToCLA.ADCoffset_Ib_inv = 1847.0f;
    CpuToCLA.ADCoffset_Ic_inv = 1852.0f;

    CpuToCLA.ADCgain_Udc = 1.15f;
    CpuToCLA.ADCgain_VaG = 1.04f;
    CpuToCLA.ADCgain_VbG = 1.04f;
    CpuToCLA.ADCgain_VcG = 1.04f;
    CpuToCLA.ADCgain_Ia_inv = 1.55f;
    CpuToCLA.ADCgain_Ib_inv = 1.5f;
    CpuToCLA.ADCgain_Ic_inv = 1.53f;

    FLC_RstFlg = 1;

    DelayMs(100);

    while(1)
    {
        #if(ALLOW_FLC_AUTO_START_UP == 1)
            if (1.0*(UDC_HCPL - 4)*800.0/(4096.0 - 4) > 300.0f)
            {
                if (e_FLC_Sts != FLC_ON)
                {
                    count_FLC_start_up++;

                    if (count_FLC_start_up > 150)
                    {
                        e_FLC_Sts = FLC_ON;
                        count_FLC_start_up = 0;
                    }
                }
            }
            else
            {
                count_FLC_start_up = 0;
            }
        #endif

        Action();

        if(e_FLC_Sts == FLC_ON)
        {
            #if(BUILDLEVEL == LEVEL1 ||BUILDLEVEL == LEVEL2|| BUILDLEVEL == LEVEL3 || BUILDLEVEL == LEVEL4 || BUILDLEVEL == LEVEL5 || BUILDLEVEL == LEVEL6 || BUILDLEVEL == LEVEL7)
                START_FLC = 1;
                CpuToCLA.EnableFlag = 1;
            #endif
        }
        else
        {
            START_FLC = 0;
            DisableEpwm();
            CpuToCLA.EnableFlag = 0;
        }

        // AC BAT LED STATUS
        if(EPwm1Regs.TZFLG.bit.OST == 1)
        {
            ENABLE_ERROR_MARCO;
            DISABLE_OPER_MARCO;
            DISABLE_STANDBY_MARCO;
        }
        else
        {
            DISABLE_ERROR_MARCO;

            if(e_FLC_Sts == FLC_OFF)
            {
                ENABLE_STANDBY_MARCO;
                DISABLE_OPER_MARCO;
            }
            else if(e_FLC_Sts == FLC_ON)
            {
                ENABLE_OPER_MARCO;
                DISABLE_STANDBY_MARCO;
            }
            else
            {
                DISABLE_OPER_MARCO;
                DISABLE_STANDBY_MARCO;
            }
        }

        // Neu co su kien bao ve thi khong cho phep START_FLC = 1 va e_FLC_Sts = FLC_ON;
        if(EPwm1Regs.TZFLG.bit.OST == 1)
        {
            e_FLC_Sts = FLC_OFF;
            START_FLC = 0;
            CpuToCLA.EnableFlag = 0;
            DisableEpwm();
        }

//        Button_STOP_Debounce();

        // Protect Monitor

        if(Cmpss8Regs.COMPSTS.bit.COMPHLATCH == 1 && EPwm1Regs.TZFLG.bit.OST == 1 && CMPSS_PROTECT_VaG_UPPER == 1)
        {
            protect_chanel.VaG_upper = 1;
        }

        else if(Cmpss8Regs.COMPSTS.bit.COMPHLATCH == 0 && EPwm1Regs.TZFLG.bit.OST == 0)
        {
            protect_chanel.VaG_upper = 0;
        }

        if(Cmpss8Regs.COMPSTS.bit.COMPLLATCH == 1 && EPwm1Regs.TZFLG.bit.OST == 1 && CMPSS_PROTECT_VaG_LOWER == 1)
        {
            protect_chanel.VaG_lower = 1;
        }

        else if(Cmpss8Regs.COMPSTS.bit.COMPLLATCH == 0 && EPwm1Regs.TZFLG.bit.OST == 0)
        {
            protect_chanel.VaG_lower = 0;

        }

        if(Cmpss7Regs.COMPSTS.bit.COMPHLATCH == 1 && EPwm1Regs.TZFLG.bit.OST == 1 && CMPSS_PROTECT_Ia_inv_UPPER == 1)
        {
            protect_chanel.Ia_upper = 1;
        }

        else if(Cmpss7Regs.COMPSTS.bit.COMPHLATCH == 0 && EPwm1Regs.TZFLG.bit.OST == 0)
        {
            protect_chanel.Ia_upper = 0;
        }

        if(Cmpss7Regs.COMPSTS.bit.COMPLLATCH == 1 && EPwm1Regs.TZFLG.bit.OST == 1 && CMPSS_PROTECT_Ia_inv_LOWER == 1)
        {
            protect_chanel.Ia_lower = 1;
        }

        else if(Cmpss7Regs.COMPSTS.bit.COMPLLATCH == 0 && EPwm1Regs.TZFLG.bit.OST == 0)
        {
            protect_chanel.Ia_lower = 0;
        }

        if(Cmpss3Regs.COMPSTS.bit.COMPHLATCH == 1 && EPwm1Regs.TZFLG.bit.OST == 1 && CMPSS_PROTECT_Ic_inv_UPPER == 1)
        {
            protect_chanel.Ic_upper = 1;
        }

        else if(Cmpss3Regs.COMPSTS.bit.COMPHLATCH == 0 && EPwm1Regs.TZFLG.bit.OST == 0)
        {
            protect_chanel.Ic_upper = 0;
        }

        if(Cmpss3Regs.COMPSTS.bit.COMPLLATCH == 1 && EPwm1Regs.TZFLG.bit.OST == 1 && CMPSS_PROTECT_Ic_inv_LOWER == 1)
        {
            protect_chanel.Ic_lower = 1;
        }

        else if(Cmpss3Regs.COMPSTS.bit.COMPLLATCH == 0 && EPwm1Regs.TZFLG.bit.OST == 0)
        {
            protect_chanel.Ic_lower = 0;
        }

        if(EPwm1Regs.TZFLG.bit.OST == 1)
        {
            CMPSS_Protect_Time = seconds_counter_cmpss;
        }
        else
        {
            CMPSS_Protect_Time = 0;
        }

        if(FLC_RstFlg == 1) // Flag reset FLC
        {
            ClrPrtFlg = 1;
            RunTask8Flag = 1;
//            UpdateProtectValue();
            seconds_counter_cmpss = 0;
            FLC_RstFlg = 0;
        }

        if(ClrPrtFlg == 1) // Flag reset protect CMPSS
        {
            ClearProtectFlagFcn();
            ClrPrtFlg = 0;
        }

        if(RunTask8Flag == 1)
        {
            EALLOW;
            Cla1Regs.MIFRC.bit.INT8 = 1; // Force Task8 run
            EDIS;
            RunTask8Flag = 0;
        }

        #if(SET_MODE_RUN == THREE_PHASE_MODE)

//            ON_RELAY = 1;

            if (ON_RELAY == 1)
            {
                if (relay_activated == 0)
                {
                    duty_relay_current = duty_relay;
                    relay_activated = 1;
                }
                EPwm6Regs.CMPA.bit.CMPA = duty_relay_current;
                if (duty_relay_current > duty_relay/2)
                {
                    duty_relay_current -= 10;
                    if (duty_relay_current < duty_relay/2)
                    {
                        duty_relay_current = duty_relay/2;
                    }
                }
            }
            else
            {
                EPwm6Regs.CMPA.bit.CMPA = 0;
                duty_relay_current = 0;
                relay_activated = 0;
            }

        #endif
//#if(SET_MODE_RUN == SINGLE_PHASE_MODE)
//
//            if (ON_RELAY == 0)
//            {
//                if (relay_activated == 1)
//                {
//                    relay_activated = 0;
//                }
//                EPwm6Regs.CMPA.bit.CMPA = 1250;
//
//           }
//       #endif
        #if(ALLOW_IPC_CPU == 1)

            for(j = 0; j < 10; j++)
            {
                data_TPC_u16[j] = 0;
            }

            IpcRegs.IPCSET.bit.IPC0 = 1;

            while(IpcRegs.IPCFLG.bit.IPC0 == 1);

            DELAY_US(100000);

        #endif
        }
    }

//
// Sdfm_configurePins - Configure SDFM pin muxing GPIOs. SDFM1 GPIO pins are
// mapped to CPU1 while SDFM2 GPIO pins are mapped to CPU2.
//
void Sdfm_configurePins(Uint16 sdfmPinOption)
{
    Uint16 pin;
    switch (sdfmPinOption)
    {
        case SDFM_PIN_MUX_OPTION1:
            for(pin = 16; pin <= 23; pin++)
            {
                GPIO_SetupPinOptions(pin, GPIO_INPUT, GPIO_ASYNC);
                GPIO_SetupPinMux(pin,GPIO_MUX_CPU1,7);
            }
            for(pin = 24; pin <= 31; pin++)
            {
                GPIO_SetupPinOptions(pin, GPIO_INPUT, GPIO_ASYNC);
                GPIO_SetupPinMux(pin,GPIO_MUX_CPU2,7);
            }
            break;

        case SDFM_PIN_MUX_OPTION2:
            for(pin = 48; pin <= 55; pin++)
            {
                GPIO_SetupPinOptions(pin, GPIO_INPUT, GPIO_ASYNC);
                GPIO_SetupPinMux(pin,GPIO_MUX_CPU1,7);
            }
            for(pin = 56; pin <= 63; pin++)
            {
                GPIO_SetupPinOptions(pin, GPIO_INPUT, GPIO_ASYNC);
                GPIO_SetupPinMux(pin,GPIO_MUX_CPU2,7);
            }
            break;

        case SDFM_PIN_MUX_OPTION3:
            for(pin = 122; pin <= 129; pin++)
            {
                GPIO_SetupPinOptions(pin, GPIO_INPUT, GPIO_ASYNC);
                GPIO_SetupPinMux(pin,GPIO_MUX_CPU1,7);
            }
            for(pin = 130; pin <= 137; pin++)
            {
                GPIO_SetupPinOptions(pin, GPIO_INPUT, GPIO_ASYNC);
                GPIO_SetupPinMux(pin,GPIO_MUX_CPU2,7);
            }
            break;
    }
}

//
// Cla_initMemoryMap - Initialize CLA memory map
//
void Cla_initMemoryMap(void)
{
    EALLOW;

    //
    // Initialize and wait for CLA1ToCPUMsgRAM
    //
    MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU != 1){};

    //
    // Initialize and wait for CPUToCLA1MsgRAM
    //
    MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 != 1){};

    //
    // Copy the program and constants from FLASH to RAM before configuring
    // the CLA
    //
#if defined(_FLASH)
    memcpy((uint32_t *)&Cla1funcsRunStart, (uint32_t *)&Cla1funcsLoadStart,
        (uint32_t)&Cla1funcsLoadSize );
    memcpy((uint32_t *)&Cla1ConstRunStart, (uint32_t *)&Cla1ConstLoadStart,
        (uint32_t)&Cla1ConstLoadSize );
#endif //defined(_FLASH)

    //
    // Select LS0 and LS1 RAM to be data RAM for the CLA and LS5 to be
    // programming space for the CLA as per linker cmd file used in this
    // example. This configuration should be updated as per the linker cmd file
    // used in the application.
    //
    MemCfgRegs.LSxMSEL.bit.MSEL_LS0 = 1; //LS0RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS0 = 0; // LS0RAM is configured as
                                             // data memory

    MemCfgRegs.LSxMSEL.bit.MSEL_LS1 = 1; //LS1RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS1 = 1; // LS1RAM is configured as
                                             // program memory

    MemCfgRegs.LSxMSEL.bit.MSEL_LS2 = 1; //LS2RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS2 = 1; // LS2RAM is configured as
                                             // program memory

    MemCfgRegs.LSxMSEL.bit.MSEL_LS3 = 1; //LS3RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS3 = 1; // LS3RAM is configured as
                                             // program memory

    MemCfgRegs.LSxMSEL.bit.MSEL_LS4 = 1; //LS4RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS4 = 1; // LS4RAM is configured as
                                             // program memory

    MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1; //LS5RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 1; // LS5RAM is configured as
                                             // program memory

    MemCfgRegs.LSxMSEL.bit.MSEL_LS6 = 1; //LS6RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS6 = 1; // LS6RAM is configured as
                                             // program memory

    MemCfgRegs.LSxMSEL.bit.MSEL_LS7 = 1; //LS7RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS7 = 1; // LS7RAM is configured as
                                             // program memory

    EDIS;
}

//
// CLA_initCpu1Cla - Initialize CLA tasks and end of task ISRs
//
void CLA_initCpu1Cla(void)
{
    //
    // Compute all CLA task vectors
    // On Type-1 CLAs the MVECT registers accept full 16-bit task addresses as
    // opposed to offsets used on older Type-0 CLAs
    //
    EALLOW;
    Cla1Regs.MVECT1 = (uint16_t)(&Cla1Task1);
    Cla1Regs.MVECT2 = (uint16_t)(&Cla1Task2);
    Cla1Regs.MVECT3 = (uint16_t)(&Cla1Task3);
    Cla1Regs.MVECT4 = (uint16_t)(&Cla1Task4);
    Cla1Regs.MVECT5 = (uint16_t)(&Cla1Task5);
    Cla1Regs.MVECT6 = (uint16_t)(&Cla1Task6);
    Cla1Regs.MVECT7 = (uint16_t)(&Cla1Task7);
    Cla1Regs.MVECT8 = (uint16_t)(&Cla1Task8);

    //
    // Enable IACK instruction to start a task on CLA in software
    // for all  8 CLA tasks
    //
    asm("   RPT #3 || NOP");
    Cla1Regs.MCTL.bit.IACKE = 1;
    Cla1Regs.MIER.all = 0x0081;

    //
    // Configure the vectors for the end-of-task interrupt for all
    // 8 tasks
    //
    PieVectTable.CLA1_1_INT = &cla1Isr1;
    PieVectTable.CLA1_2_INT = &cla1Isr2;
    PieVectTable.CLA1_3_INT = &cla1Isr3;
    PieVectTable.CLA1_4_INT = &cla1Isr4;
    PieVectTable.CLA1_5_INT = &cla1Isr5;
    PieVectTable.CLA1_6_INT = &cla1Isr6;
    PieVectTable.CLA1_7_INT = &cla1Isr7;
    PieVectTable.CLA1_8_INT = &cla1Isr8;

    //
    // Enable CLA interrupts at the group and subgroup levels
    //
    PieCtrlRegs.PIEIER11.all = 0xFFFF;
    IER |= (M_INT11 );
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM
    EDIS;
}

//
// cla1Isr1 - CLA 1 ISR 1
//
interrupt void cla1Isr1 ()
{
//     asm(" ESTOP0");
    Task1_Isr++;
     PieCtrlRegs.PIEACK.all = M_INT11;
     dlog1((ClaToCPU.ADC_CPU.datalog1+1)*10000);
     dlog2((ClaToCPU.ADC_CPU.datalog2+1)*10000);
     dlog3((ClaToCPU.ADC_CPU.datalog3+1)*10000);
}

//
// cla1Isr2 - CLA 1 ISR 2
//
interrupt void cla1Isr2 ()
{
//     asm(" ESTOP0");
     PieCtrlRegs.PIEACK.all = M_INT11;
}

//
// cla1Isr3 - CLA 1 ISR 3
//
interrupt void cla1Isr3 ()
{
//    asm(" ESTOP0");
}

//
// cla1Isr4 - CLA 1 ISR 4
//
interrupt void cla1Isr4 ()
{
//    asm(" ESTOP0");
}

//
// cla1Isr5 - CLA 1 ISR 5
//
interrupt void cla1Isr5 ()
{
//    asm(" ESTOP0");
}

//
// cla1Isr6 - CLA 1 ISR 6
//
interrupt void cla1Isr6 ()
{
//    asm(" ESTOP0");
}

//
// cla1Isr7 - CLA 1 ISR 7
//
interrupt void cla1Isr7 ()
{
//    asm(" ESTOP0");
}

//
// cla1Isr8 - CLA 1 ISR 8
//
interrupt void cla1Isr8 ()
{
    // asm(" ESTOP0");
    Task8_Isr++;
    PieCtrlRegs.PIEACK.all = M_INT11;
}

static inline void dlog1(Uint16 value)
{
    DataLog1[ndx1] = value;
    if(++ndx1 == DLOG_SIZE_1)
    {
        ndx1 = 0;
    }
}

static inline void dlog2(Uint16 value)
{
    DataLog2[ndx2] = value;
    if(++ndx2 == DLOG_SIZE_2)
    {
        ndx2 = 0;
    }
}
static inline void dlog3(Uint16 value)
{
    DataLog3[ndx3] = value;
    if(++ndx3 == DLOG_SIZE_3)
    {
      ndx3 = 0;
    }
}

//
// End of file
//
