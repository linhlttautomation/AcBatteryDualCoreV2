/*
 * CPU2_Main.c
 *
 *  Created on: Aug 1, 2025
 *  Author    : TUNG BUI K65
 */

//
// Included Files
//
#include "F28x_Project.h"
#include "sdfm_ex1_filter_cla_sync_shared_cpu2.h"
#include "f2838x_sdfm_drivers.h"
#include "f2838x_struct.h"
#include "TPC_Variables.h"
#include "f2838x_examples.h"
#include "TPC_Setting.h"
#include "F2837xD_CMPSS_defines.h"
#include "Pwm_CFDAB.h"
#include "Pwm_VFDAB.h"
#include "ADC_CFDAB.h"

typedef struct {
        unsigned int PeriodMax;     // Parameter: PWM Half-Period in CPU clock cycles (Q0)
        float MfuncA1;        // Input: EPWM1 A&B Duty cycle ratio (Q15)
        float MfuncA2;        // Input: EPWM2 A&B Duty cycle ratio (Q15)
        float MfuncB1;        // Input: EPWM3 A&B Duty cycle ratio (Q15)
        float MfuncB2;        // Input: EPWM1 A&B Duty cycle ratio (Q15)
        float MfuncC1;        // Input: EPWM2 A&B Duty cycle ratio (Q15)
        float MfuncC2;        // Input: EPWM3 A&B Duty cycle ratio (Q15)
        } PWMGEN ;

typedef PWMGEN *PWMGEN_handle;

float Datalog1[200],Datalog2[200];
float Vout_Display;
float Vin_Display;
float Vc_Display;

#define F2837X_FC_PWM_GEN    { 10000,  \
                              0.0, \
                              0.0, \
                              0.0, \
                              0.0, \
                              0.0, \
                              0.0, \
                             }

#define PWMGEN_DEFAULTS     F2837X_FC_PWM_GEN

#define WAITSTEP                  asm(" RPT #255 || NOP")

//
// Global variables
//
#pragma DATA_SECTION(Protect_Chanel_TPC,"RAMGS1");
volatile PROTECT_CHANEL_TPC  Protect_Chanel_TPC;

#pragma DATA_SECTION(Setting_bat,"RAMGS1");
volatile SETTING_BAT  Setting_bat;

#pragma DATA_SECTION(Task8_Isr,"RAMGS0");
volatile Uint16  Task8_Isr = 0;

#pragma DATA_SECTION(StartFlag,"RAMGS0");
volatile Uint16  StartFlag = 0;

#pragma DATA_SECTION(START,"RAMGS0");
volatile Uint16  START = 0;

#pragma DATA_SECTION(Task1_Isr,"RAMGS0");
volatile Uint16  Task1_Isr = 0;

#pragma DATA_SECTION(Task2_Isr,"RAMGS0");
volatile Uint16  Task2_Isr = 0;

#pragma DATA_SECTION(ChannelAdc,"RAMGS0");
volatile int  ChannelAdc = 0;

// CMPSS parameters for Over Current/Voltage Protection TPC

#pragma DATA_SECTION(clkPrescale, "RAMGS0");
volatile Uint16 clkPrescale = 2;

#pragma DATA_SECTION(sampwin, "RAMGS0");
volatile Uint16 sampwin = 30;

#pragma DATA_SECTION(thresh, "RAMGS0");
volatile Uint16 thresh = 18;

#pragma DATA_SECTION(LEM_curIlvHi, "RAMGS0");
volatile Uint16 LEM_curIlvHi = LEM_2(45);

#pragma DATA_SECTION(LEM_curIlvLo, "RAMGS0");
volatile Uint16 LEM_curIlvLo = LEML_2(-45);

#pragma DATA_SECTION(LEM_curIhvHi, "RAMGS0");
volatile Uint16 LEM_curIhvHi = LEM_2(10);

#pragma DATA_SECTION(LEM_curIhvLo, "RAMGS0");
volatile Uint16 LEM_curIhvLo = LEML_2(-10);

#pragma DATA_SECTION(MEA_voltUbatHi, "RAMGS0");
volatile Uint16 MEA_voltUbatHi = MEAUBAT(120);

#pragma DATA_SECTION(MEA_voltUbatLo, "RAMGS0");
volatile Uint16 MEA_voltUbatLo = 0;

#pragma DATA_SECTION(MEA_voltUcHi, "RAMGS0");
volatile Uint16 MEA_voltUcHi = MEAUC(340);

#pragma DATA_SECTION(MEA_voltUcLo, "RAMGS0");
volatile Uint16 MEA_voltUcLo = 0;

#pragma DATA_SECTION(MEA_voltUdcHi, "RAMGS0");
volatile Uint16 MEA_voltUdcHi = MEAUDC(480);

#pragma DATA_SECTION(MEA_voltUdcLo, "RAMGS0");
volatile Uint16 MEA_voltUdcLo = 0;

//
// Macro definitions
//
#define MAX_SAMPLES               1024
#define SDFM_PIN_MUX_OPTION1      1
#define SDFM_PIN_MUX_OPTION2      2
#define SDFM_PIN_MUX_OPTION3      3


//
// Global variables
//
Uint16 gPeripheralNumber;

//
// Function prototypes
//
void Cla_initMemoryMap(void);
void CLA_initCpu2Cla(void);
void CMPSS_Protection(void);

// Khai bao cac bien share CPU --> CLA
extern volatile CPU_TO_CLA CpuToCLA;

// Khai bao cac bien share CPU --> CLA
extern volatile CLA_TO_CPU ClaToCPU;

void DelayMs(unsigned long ms)
{
    unsigned long count = 0;
    for(count = 0; count < ms ; count++)
    {
        DELAY_US(1000);
    }
}

void DelayS(unsigned long s)
{
    unsigned long count = 0;
    for(count = 0; count < s ; count++)
    {
        DelayMs(1000);
    }
}


//
// Main
//
int main(void)
{
//    Uint16  HLT, LLT;
    InitSysCtrl();

    EALLOW;

    CpuSysRegs.PCLKCR2.bit.EPWM7 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM8 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM9 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM10 = 1;

    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    CpuSysRegs.PCLKCR13.bit.ADC_C = 1;

    CpuSysRegs.PCLKCR14.bit.CMPSS1 = 1;
    CpuSysRegs.PCLKCR14.bit.CMPSS2 = 1;
    CpuSysRegs.PCLKCR14.bit.CMPSS4 = 1;
    CpuSysRegs.PCLKCR14.bit.CMPSS5 = 1;
    CpuSysRegs.PCLKCR14.bit.CMPSS6 = 1;

    EDIS;

    Cla1ForceTask8();
    Init_ADC_A();
    Init_ADC_C();


    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;


 #if(TPC_MODE_RUN == CFDAB_MODE)
    PWM_CFDAB(2000,60);
 #endif

#if(TPC_MODE_RUN == VFDAB_MODE)

   //PWM_VFDAB(1000,60); //50k
   PWM_VFDAB(500,50); //100k
   //PWM_VFDAB(625,40); //80k
   //PWM_VFDAB(715,40); //70k
   //PWM_VFDAB(834,40); //60k
#endif

    CMPSS_Protection();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    DINT;

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

    EINT;


    //
    // Configure the CLA memory spaces
    //
    Cla_initMemoryMap();

    //
    // Configure the CLA task vectors for CPU2
    //
    CLA_initCpu2Cla();

    Cla1ForceTask8andWait();
    WAITSTEP;

    EALLOW;

//    //
//    // Trigger Source for TASK1 of CLA1 = SDFM1. Uncomment if SDFM1 is
//    // connected to CPU2.
//    //
    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK1=CLA_TRIG_ADCCINT1;

    //
    // Trigger Source for TASK2 of CLA1 = SDFM2
    //
   // DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK2=CLA_TRIG_ADCAINT2;

    //
    // Lock CLA1TASKSRCSEL1 register
    //
    DmaClaSrcSelRegs.CLA1TASKSRCSELLOCK.bit.CLA1TASKSRCSEL1=1;
    EDIS;

#if(TPC_MODE_RUN == CFDAB_MODE)

    // khoi tao luong dat Cpu cho CLA
    CpuToCLA.EnableADC = 0;
    DelayMs(1000);

    CpuToCLA.EnableADC = 1;
    CpuToCLA.EnableFlag = 0;
    CpuToCLA.UdcTesting = 600.0/Udc_max;

    CpuToCLA.DutyTesting = 0.64;
    CpuToCLA.DutyStart = 0.001;

    CpuToCLA.PhiTesting = 0.084;
    CpuToCLA.PhiStart = 0.001;

    DelayMs(1000);
    //------------------------------------------------------------------------------
    // khoi tao tham so ban dau cho CFDAB
    Setting_bat.Power  = CFDAB_Power;
    Setting_bat.Voltage = CFDAB_Voltage;
    Setting_bat.ChargeCurrentMax = CFDAB_MaxCharge_Current;
    Setting_bat.DisChargeCurrentMax = CFDAB_MaxDischarge_Current;

    Setting_bat.UdcRef = CFDAB_UdcRef;
    Setting_bat.VcRef  = CFDAB_VcRef;
    Setting_bat.UbatRef = CFDAB_UbatRef;
    Setting_bat.IbatRef = CFDAB_IbatRef;

    Setting_bat.UdcMax = CFDAB_Udc_Max;
    Setting_bat.UdcMin = CFDAB_Udc_Min;
    Setting_bat.VcMax = CFDAB_Vc_Max;
    Setting_bat.VcMin = CFDAB_Vc_Min;
    Setting_bat.UbatMax = CFDAB_Ubat_Max;
    Setting_bat.UbatMin = CFDAB_Ubat_Min;
    Setting_bat.IbatMax = CFDAB_Ibat_Max;

    DelayMs(1000);

#endif

#if(TPC_MODE_RUN == VFDAB_MODE)

    /*****************************************************************************/
    // khoi tao luong dat Cpu cho CLA
    CpuToCLA.EnableADC = 0;
    DelayMs(1000);

    CpuToCLA.EnableADC = 1;
    CpuToCLA.EnableFlag = 0;
    CpuToCLA.UdcTesting = 600.0/Udc_max;

    //CpuToCLA.DutyTesting_Discharge = 0.998;//0.999
    CpuToCLA.DutyTesting_Discharge = 0.998;
    CpuToCLA.DutyTesting_Charge = 0.5;
    CpuToCLA.DutyStart = 0.001;

    CpuToCLA.PhiTesting = 0.084;
    CpuToCLA.PhiStart = 0.001;
    CpuToCLA.cmpc = 500;
    //CpuToCLA.Io_Ref = VFDAB_IoutRef;
    //------------------------------------------------------------------------------
    // khoi tao tham so ban dau cho VFDAB
    Setting_bat.Power  = VFDAB_Power;
    Setting_bat.Voltage = VFDAB_Voltage;
    Setting_bat.ChargeCurrentMax = VFDAB_MaxCharge_Current;
    Setting_bat.DisChargeCurrentMax = VFDAB_MaxDischarge_Current;

    Setting_bat.UdcRef = VFDAB_UdcRef;
    Setting_bat.IoutRef = VFDAB_IoutRef;
    Setting_bat.UdcThreshold = VFDAB_UdcThreshold;
    Setting_bat.VcRef  = VFDAB_VcRef;
    Setting_bat.UbatRef = VFDAB_UbatRef;
    Setting_bat.IbatRef = VFDAB_IbatRef;

    Setting_bat.UdcMax = VFDAB_Udc_Max;
    Setting_bat.UdcMin = VFDAB_Udc_Min;
    Setting_bat.VcMax = VFDAB_Vc_Max;
    Setting_bat.VcMin = VFDAB_Vc_Min;
    Setting_bat.UbatMax = VFDAB_Ubat_Max;
    Setting_bat.UbatMin = VFDAB_Ubat_Min;
    Setting_bat.IbatMax = VFDAB_Ibat_Max;
    Setting_bat.IoutMax = VFDAB_Iout_Max;
    Setting_bat.cmpc = 500;
    // START = 1;
    //DelayMs(100);
#endif

    //
    // Select SDFM2
    //
    //gPeripheralNumber = SDFM2;

    //
    // Input Control Module
    //

    //
    //Configure Input Control Mode: Modulator Clock rate = Modulator data rate
    //
//    Sdfm_configureInputCtrl(gPeripheralNumber, FILTER1, MODE_0);
//    Sdfm_configureInputCtrl(gPeripheralNumber, FILTER2, MODE_0);
//    Sdfm_configureInputCtrl(gPeripheralNumber, FILTER3, MODE_0);
//    Sdfm_configureInputCtrl(gPeripheralNumber, FILTER4, MODE_0);

    //
    // Comparator Module
    //
//    HLT = 0x7FFF;    //Over value threshold settings
//    LLT = 0x0000;    //Under value threshold settings

    //
    // Configure Comparator module's comparator filter type and comparator's OSR
    // value, higher threshold, lower threshold
    //
//    Sdfm_configureComparator(gPeripheralNumber, FILTER1, SINC3, OSR_32,
//                             HLT, LLT);
//    Sdfm_configureComparator(gPeripheralNumber, FILTER2, SINC3, OSR_32,
//                             HLT, LLT);
//    Sdfm_configureComparator(gPeripheralNumber, FILTER3, SINC3, OSR_32,
//                             HLT, LLT);
//    Sdfm_configureComparator(gPeripheralNumber, FILTER4, SINC3, OSR_32,
//                             HLT, LLT);

    //
    // Data filter Module
    //

    //
    // Configure Data filter modules filter type, OSR value and
    // enable / disable data filter
    //
//    Sdfm_configureData_filter(gPeripheralNumber, FILTER1, FILTER_ENABLE, SINC3,
//                              OSR_256, DATA_16_BIT, SHIFT_10_BITS);
//    Sdfm_configureData_filter(gPeripheralNumber, FILTER2, FILTER_ENABLE, SINC3,
//                              OSR_256, DATA_16_BIT, SHIFT_10_BITS);
//    Sdfm_configureData_filter(gPeripheralNumber, FILTER3, FILTER_ENABLE, SINC3,
//                              OSR_256, DATA_16_BIT, SHIFT_10_BITS);
//    Sdfm_configureData_filter(gPeripheralNumber, FILTER4, FILTER_ENABLE, SINC3,
//                              OSR_256, DATA_16_BIT, SHIFT_10_BITS);

    //
    // Enable Master filter bit: Unless this bit is set none of the filter modules
    // can be enabled.
    // All the filter modules are synchronized when master filter bit is enabled
    // after individual filter modules are enabled.
    //
//    Sdfm_enableMFE(gPeripheralNumber);

    //
    // PWM11.CMPC, PWM11.CMPD signals can synchronize SDFM1 filters and
    // PWM12.CMPC and PWM12.CMPD signals can synchronize SDFM2 filters. This
    // option is not being used in this example for SDFM1.
    //
//    Sdfm_configureExternalreset(gPeripheralNumber,FILTER_1_EXT_RESET_DISABLE,
//                                FILTER_2_EXT_RESET_DISABLE,
//                                FILTER_3_EXT_RESET_DISABLE,
//                                FILTER_4_EXT_RESET_DISABLE);

    //
    // Enable interrupts
    //

    //
    // Following SDFM interrupts can be enabled / disabled using this function.
    //  Enable / disable comparator high threshold
    //  Enable / disable comparator low threshold
    //  Enable / disable modulator clock failure
    //  Enable / disable filter acknowledge
    //
//    Sdfm_configureInterrupt(gPeripheralNumber, FILTER1, IEH_DISABLE,
//                            IEL_DISABLE, MFIE_ENABLE, AE_ENABLE);
//    Sdfm_configureInterrupt(gPeripheralNumber, FILTER2, IEH_DISABLE,
//                            IEL_DISABLE, MFIE_ENABLE, AE_ENABLE);
//    Sdfm_configureInterrupt(gPeripheralNumber, FILTER3, IEH_DISABLE,
//                            IEL_DISABLE, MFIE_ENABLE, AE_ENABLE);
//    Sdfm_configureInterrupt(gPeripheralNumber, FILTER4, IEH_DISABLE,
//                            IEL_DISABLE, MFIE_ENABLE, AE_ENABLE);

    //
    // Enable master interrupt so that any of the filter interrupts can trigger
    // SDFM interrupt to CPU
    //
//    Sdfm_enableMIE(gPeripheralNumber);

    //
    // Wait for an interrupt
    //
    while(1)
    {
        if(START == 1)
        {
#if(TPC_MODE_RUN == CFDAB_MODE)
            // LEVEL1
            #if(BUILDLEVEL == LEVEL1)
                CpuToCLA.EnableFlag = 1;
            #endif
            // LEVEL5
            #if(BUILDLEVEL == LEVEL5)
            // if(Setting_bat.UdcRef > BAT_UDC_REF) Setting_bat.UdcRef = BAT_UDC_REF;
            if(Setting_bat.UdcRef > 401) Setting_bat.UdcRef = 400;
            // if(Setting_bat.VcRef > BAT_VC_REF) Setting_bat.VcRef = BAT_VC_REF;
            if(Setting_bat.VcRef > 268) Setting_bat.VcRef = 267;

            CpuToCLA.UdcRef = Setting_bat.UdcRef/Udc_max;
            CpuToCLA.VcRef = Setting_bat.VcRef/Uc_max;
            CpuToCLA.EnableFlag = 1;
            #endif
#endif

#if(TPC_MODE_RUN == VFDAB_MODE)
#if(BUILDLEVEL == LEVEL1)

CpuToCLA.EnableFlag = 1;
#endif
// LEVEL2
//            #if(BUILDLEVEL == LEVEL2)
//
//            CpuToCLA.EnableFlag = 1;
//            #endif
// LEVEL2
#if(BUILDLEVEL == LEVEL2)
if(Setting_bat.UdcRef > 350) Setting_bat.UdcRef = 350;
if(Setting_bat.UdcThreshold > 350) Setting_bat.UdcThreshold = 350;
       //    if(Setting_bat.IoutRef > BAT_IOUT_REF_MAX) Setting_bat.IoutRef = BAT_IOUT_REF_MAX;
CpuToCLA.UdcRef = Setting_bat.UdcRef/Udc_max;
CpuToCLA.UdcThreshold = Setting_bat.UdcThreshold/Udc_max;
// CpuToCLA.Io_Ref = Setting_bat.IoutRef/Iout_max;
CpuToCLA.EnableFlag = 1;
CpuToCLA.cmpc = Setting_bat.cmpc;
#endif
 // LEVEL3
#if(BUILDLEVEL == LEVEL3)
if(Setting_bat.UdcRef > 350) Setting_bat.UdcRef = 350;
if(Setting_bat.UdcThreshold > 350) Setting_bat.UdcThreshold = 350;
CpuToCLA.UdcRef = Setting_bat.UdcRef/Udc_max;
CpuToCLA.UdcThreshold = Setting_bat.UdcThreshold/Udc_max;
CpuToCLA.EnableFlag = 1;
CpuToCLA.cmpc = Setting_bat.cmpc;
#endif

// LEVEL4
#if(BUILDLEVEL == LEVEL4)
if(Setting_bat.VcRef >50) Setting_bat.VcRef = 50;
CpuToCLA.VcRef = Setting_bat.VcRef/Uc_max;
CpuToCLA.EnableFlag = 1;
#endif

// LEVEL5
#if(BUILDLEVEL == LEVEL5)
//            if(Setting_bat.UdcRef > BAT_UDC_REF) Setting_bat.UdcRef = BAT_UDC_REF;
if(Setting_bat.UdcRef > 250) Setting_bat.UdcRef = 250;
//            if(Setting_bat.VcRef > BAT_VC_REF) Setting_bat.VcRef = BAT_VC_REF;
if(Setting_bat.VcRef > 110) Setting_bat.VcRef = 110;

CpuToCLA.UdcRef = Setting_bat.UdcRef/Udc_max;
CpuToCLA.VcRef = Setting_bat.VcRef/Uc_max;
CpuToCLA.EnableFlag = 1;
#endif

// LEVEL6
#if(BUILDLEVEL == LEVEL6)
if(Setting_bat.IbatRef > BAT_IBAT_REF_MAX) Setting_bat.IbatRef = BAT_IBAT_REF_MAX;
CpuToCLA.IbatRef = Setting_bat.IbatRef/Ibat_max;
CpuToCLA.EnableFlag = 1;
#endif

// LEVEL7
#if(BUILDLEVEL == LEVEL7)
if(Setting_bat.UbatRef > 60) Setting_bat.UbatRef = 60;
CpuToCLA.UbatRef = Setting_bat.UbatRef/Ubat_max;
CpuToCLA.EnableFlag = 1;
#endif
// LEVEL8
#if(BUILDLEVEL == LEVEL8)
if(Setting_bat.UdcRef > 350) Setting_bat.UdcRef = 350;
if(Setting_bat.UdcThreshold > 350) Setting_bat.UdcThreshold = 350;
//    if(Setting_bat.IoutRef > BAT_IOUT_REF_MAX) Setting_bat.IoutRef = BAT_IOUT_REF_MAX;
CpuToCLA.UdcRef = Setting_bat.UdcRef/Udc_max;
CpuToCLA.UdcThreshold = Setting_bat.UdcThreshold/Udc_max;
// CpuToCLA.Io_Ref = Setting_bat.IoutRef/Iout_max;
CpuToCLA.EnableFlag = 1;
CpuToCLA.cmpc = Setting_bat.cmpc;
#endif
// LEVEL 9
#if(BUILDLEVEL == LEVEL9)
if(Setting_bat.IoutRef > VFDAB_Iout_Max) Setting_bat.IbatRef = VFDAB_Iout_Max;
// CpuToCLA.Io_Ref = Setting_bat.IoutRef/Iout_max;
CpuToCLA.Io_Ref = Setting_bat.IoutRef;
CpuToCLA.EnableFlag = 1;
#endif
#endif
        }

        else
        {
            CpuToCLA.EnableFlag = 0;
        }

        // Hien thi kenh bao ve
        if(Cmpss5Regs.COMPSTS.bit.COMPHLATCH == 1)
        {
            Protect_Chanel_TPC.Ilv = 1;
        }
        if(Cmpss5Regs.COMPSTS.bit.COMPLLATCH == 1)
        {
            Protect_Chanel_TPC.Ilv = 1;
        }
        if(Cmpss2Regs.COMPSTS.bit.COMPHLATCH == 1)
        {
            Protect_Chanel_TPC.Ubat_Upper = 1;
        }
        if(Cmpss1Regs.COMPSTS.bit.COMPHLATCH == 1)
        {
            Protect_Chanel_TPC.Uclamp_Upper = 1;
        }
        if(Cmpss4Regs.COMPSTS.bit.COMPHLATCH == 1)
        {
            Protect_Chanel_TPC.Udc_TPC_Upper = 1;
        }
    }
}

//
// Cla_initMemoryMap - Initialize CLA memory
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
// CLA_initCpu2Cla - Initialize CLA task vectors and end of task interrupts
//
void CLA_initCpu2Cla(void)
{
    //
    // Compute all CLA task vectors. The VECT register has 16 bits so no
    // need to subtract an offset
    //
    EALLOW;
    Cla1Regs.MVECT1 = (Uint16)(&Cla1Task1);
    Cla1Regs.MVECT2 = (Uint16)(&Cla1Task2);
    Cla1Regs.MVECT3 = (Uint16)(&Cla1Task3);
    Cla1Regs.MVECT4 = (Uint16)(&Cla1Task4);
    Cla1Regs.MVECT5 = (Uint16)(&Cla1Task5);
    Cla1Regs.MVECT6 = (Uint16)(&Cla1Task6);
    Cla1Regs.MVECT7 = (Uint16)(&Cla1Task7);
    Cla1Regs.MVECT8 = (Uint16)(&Cla1Task8);

    //
    // Enable IACK instruction to start a task on CLA
    // and for all  the CLA tasks
    //
    asm("   RPT #3 || NOP");
    Cla1Regs.MCTL.bit.IACKE = 1;
    Cla1Regs.MIER.all = 0x0083;

    //
    // Enable CLA interrupts at the group and subgroup levels
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
// cla1Isr1 - CLA1 ISR 1
//
interrupt void cla1Isr1 ()
{
     //asm(" ESTOP0");
     PieCtrlRegs.PIEACK.all = M_INT11;
}

//
// cla1Isr2 - CLA1 ISR 2
//
interrupt void cla1Isr2 ()
{
     //asm(" ESTOP0");
     PieCtrlRegs.PIEACK.all = M_INT11;
}

//
// cla1Isr3 - CLA1 ISR 3
//
interrupt void cla1Isr3 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr4 - CLA1 ISR 4
//
interrupt void cla1Isr4 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr5 - CLA1 ISR 5
//
interrupt void cla1Isr5 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr6 - CLA1 ISR 6
//
interrupt void cla1Isr6 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr7 - CLA1 ISR 7
//
interrupt void cla1Isr7 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr8 - CLA1 ISR 8
//
interrupt void cla1Isr8 ()
{
    // asm(" ESTOP0");
    PieCtrlRegs.PIEACK.all = M_INT11;
}

void cmpssConfig(volatile struct CMPSS_REGS *v, int16 Hi, int16 Lo)
{

    // Set up COMPCTL register
    EALLOW;
    v->COMPCTL.bit.COMPDACE    = 1;             // Enable CMPSS
    v->COMPCTL.bit.COMPLSOURCE = NEGIN_DAC;     // NEG signal from DAC for COMP-L
    v->COMPCTL.bit.COMPHSOURCE = NEGIN_DAC;     // NEG signal from DAC for COMP-H
    v->COMPCTL.bit.COMPHINV    = 0;             // COMP-H output is NOT inverted
    v->COMPCTL.bit.COMPLINV    = 1;             // COMP-L output is inverted
    v->COMPCTL.bit.ASYNCHEN    = 0;             // Disable aynch COMP-H ouput
    v->COMPCTL.bit.ASYNCLEN    = 0;             // Disable aynch COMP-L ouput
    v->COMPCTL.bit.CTRIPHSEL    = CTRIP_FILTER; // Dig filter output ==> CTRIPH
    v->COMPCTL.bit.CTRIPOUTHSEL = CTRIP_FILTER; // Dig filter output ==> CTRIPOUTH
    v->COMPCTL.bit.CTRIPLSEL    = CTRIP_FILTER; // Dig filter output ==> CTRIPL
    v->COMPCTL.bit.CTRIPOUTLSEL = CTRIP_FILTER; // Dig filter output ==> CTRIPOUTL

    // Set up COMPHYSCTL register
    v->COMPHYSCTL.bit.COMPHYS   = 2; // COMP hysteresis set to 2x typical value

    // set up COMPDACCTL register
    v->COMPDACCTL.bit.SELREF    = REFERENCE_VDDA_CMPSS; // VDDA is REF for CMPSS DACs
    v->COMPDACCTL.bit.SWLOADSEL = 0; // DAC updated on sysclock
    v->COMPDACCTL.bit.DACSOURCE = 0; // Ramp bypassed

    // Load DACs - High and Low
    v->DACHVALS.bit.DACVAL = Hi;     // Set DAC-H to allowed MAX +ve current
    v->DACLVALS.bit.DACVAL = Lo;     // Set DAC-L to allowed MAX -ve current

    // digital filter settings - HIGH side
    v->CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale; // set time between samples, max : 1023
    v->CTRIPHFILCTL.bit.SAMPWIN        = sampwin;     // # of samples in window, max : 31
    v->CTRIPHFILCTL.bit.THRESH         = thresh;      // recommended : thresh > sampwin/2
    v->CTRIPHFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value

    // digital filter settings - LOW side
    v->CTRIPLFILCLKCTL.bit.CLKPRESCALE = clkPrescale; // set time between samples, max : 1023
    v->CTRIPLFILCTL.bit.SAMPWIN        = sampwin;     // # of samples in window, max : 31
    v->CTRIPLFILCTL.bit.THRESH         = thresh;      // recommended : thresh > sampwin/2
    v->CTRIPLFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value

    // Clear the status register for latched comparator events
    v->COMPSTSCLR.bit.HLATCHCLR = 1;
    v->COMPSTSCLR.bit.LLATCHCLR = 1;


    EDIS;
    return;
}

void cmpssConfig1(volatile struct CMPSS_REGS *v, int16 Hi, int16 Lo)
{

    // Set up COMPCTL register
    EALLOW;
    v->COMPCTL.bit.COMPDACE    = 1;             // Enable CMPSS
    v->COMPCTL.bit.COMPLSOURCE = NEGIN_DAC;     // NEG signal from DAC for COMP-L
    v->COMPCTL.bit.COMPHSOURCE = NEGIN_DAC;     // NEG signal from DAC for COMP-H
    v->COMPCTL.bit.COMPHINV    = 0;             // COMP-H output is NOT inverted
    v->COMPCTL.bit.COMPLINV    = 1;             // COMP-L output is inverted
    v->COMPCTL.bit.ASYNCHEN    = 0;             // Disable aynch COMP-H ouput
    v->COMPCTL.bit.ASYNCLEN    = 0;             // Disable aynch COMP-L ouput
    v->COMPCTL.bit.CTRIPHSEL    = CTRIP_FILTER; // Dig filter output ==> CTRIPH
    v->COMPCTL.bit.CTRIPOUTHSEL = CTRIP_FILTER; // Dig filter output ==> CTRIPOUTH
    v->COMPCTL.bit.CTRIPLSEL    = CTRIP_FILTER; // Dig filter output ==> CTRIPL
    v->COMPCTL.bit.CTRIPOUTLSEL = CTRIP_FILTER; // Dig filter output ==> CTRIPOUTL

    // Set up COMPHYSCTL register
    v->COMPHYSCTL.bit.COMPHYS   = 2; // COMP hysteresis set to 2x typical value

    // set up COMPDACCTL register
    v->COMPDACCTL.bit.SELREF    = REFERENCE_VDDA_CMPSS; // VDDA is REF for CMPSS DACs
    v->COMPDACCTL.bit.SWLOADSEL = 0; // DAC updated on sysclock
    v->COMPDACCTL.bit.DACSOURCE = 0; // Ramp bypassed

    // Load DACs - High and Low
    v->DACHVALS.bit.DACVAL = Hi;     // Set DAC-H to allowed MAX +ve current
    v->DACLVALS.bit.DACVAL = Lo;     // Set DAC-L to allowed MAX -ve current

    // digital filter settings - HIGH side
    v->CTRIPHFILCLKCTL.bit.CLKPRESCALE = 5; // set time between samples, max : 1023
    v->CTRIPHFILCTL.bit.SAMPWIN        = sampwin;     // # of samples in window, max : 31
    v->CTRIPHFILCTL.bit.THRESH         = thresh;      // recommended : thresh > sampwin/2
    v->CTRIPHFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value

    // digital filter settings - LOW side
    v->CTRIPLFILCLKCTL.bit.CLKPRESCALE = 5; // set time between samples, max : 1023
    v->CTRIPLFILCTL.bit.SAMPWIN        = sampwin;     // # of samples in window, max : 31
    v->CTRIPLFILCTL.bit.THRESH         = thresh;      // recommended : thresh > sampwin/2
    v->CTRIPLFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value

    // Clear the status register for latched comparator events
    v->COMPSTSCLR.bit.HLATCHCLR = 1;
    v->COMPSTSCLR.bit.LLATCHCLR = 1;


    EDIS;
    return;
}
//
void CMPSS_Protection(void)
{
    #if(CMPSS_PROTECT_Ubat_UPPER == 1)
    cmpssConfig(&Cmpss2Regs,MEA_voltUbatHi, MEA_voltUbatLo);  //Enable CMPSS6 - BAT VOLTAGE - 6P
    #endif
    #if(CMPSS_PROTECT_Ihv == 1)
    cmpssConfig1(&Cmpss5Regs, LEM_curIhvHi, LEM_curIhvLo);  //Enable CMPS5 - LEM CURRENT  for ADCINC4
    #endif
    #if(CMPSS_PROTECT_Uclamp_UPPER == 1)
    cmpssConfig(&Cmpss1Regs,MEA_voltUcHi,MEA_voltUcLo);  //Enable CMPSS7 - Vclamp -7P _ADCC5
    #endif
    #if(CMPSS_PROTECT_Udc_UPPER == 1)
    cmpssConfig(&Cmpss4Regs,MEA_voltUdcHi,MEA_voltUdcLo);  //Enable CMPSS8 - Vdc - 8P _ADCB2
    #endif

    EALLOW;

    EPwm7Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3; //Trip 4 is the input to the DCAHCOMPSEL
    EPwm7Regs.TZDCSEL.bit.DCAEVT1       = TZ_DCAH_HI;
    EPwm7Regs.DCACTL.bit.EVT1SRCSEL     = DC_EVT1;
    EPwm7Regs.DCACTL.bit.EVT1FRCSYNCSEL = DC_EVT_ASYNC;
    EPwm7Regs.TZSEL.bit.DCAEVT1         = 1;           // 1/0 - Enable/Disable One Shot Mode

    EPwm7Regs.DCTRIPSEL.bit.DCBHCOMPSEL = 3; //Trip 4 is the input to the DCBHCOMPSEL
    EPwm7Regs.TZDCSEL.bit.DCBEVT1       = TZ_DCBH_HI;
    EPwm7Regs.DCBCTL.bit.EVT1SRCSEL     = DC_EVT1;
    EPwm7Regs.DCBCTL.bit.EVT1FRCSYNCSEL = DC_EVT_ASYNC;
    EPwm7Regs.TZSEL.bit.DCBEVT1         = 1;           // 1/0 - Enable/Disable One Shot Mode

    EPwm8Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3; //Trip 4 is the input to the DCAHCOMPSEL
    EPwm8Regs.TZDCSEL.bit.DCAEVT1       = TZ_DCAH_HI;
    EPwm8Regs.DCACTL.bit.EVT1SRCSEL     = DC_EVT1;
    EPwm8Regs.DCACTL.bit.EVT1FRCSYNCSEL = DC_EVT_ASYNC;
    EPwm8Regs.TZSEL.bit.DCAEVT1         = 1;

    EPwm8Regs.DCTRIPSEL.bit.DCBHCOMPSEL = 3; //Trip 4 is the input to the DCBHCOMPSEL
    EPwm8Regs.TZDCSEL.bit.DCBEVT1       = TZ_DCBH_HI;
    EPwm8Regs.DCBCTL.bit.EVT1SRCSEL     = DC_EVT1;
    EPwm8Regs.DCBCTL.bit.EVT1FRCSYNCSEL = DC_EVT_ASYNC;
    EPwm8Regs.TZSEL.bit.DCBEVT1         = 1;           // 1/0 - Enable/Disable One Shot Mode

    EPwm9Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3; //Trip 4 is the input to the DCAHCOMPSEL
    EPwm9Regs.TZDCSEL.bit.DCAEVT1       = TZ_DCAH_HI;
    EPwm9Regs.DCACTL.bit.EVT1SRCSEL     = DC_EVT1;
    EPwm9Regs.DCACTL.bit.EVT1FRCSYNCSEL = DC_EVT_ASYNC;
    EPwm9Regs.TZSEL.bit.DCAEVT1         = 1;

    EPwm9Regs.DCTRIPSEL.bit.DCBHCOMPSEL = 3; //Trip 4 is the input to the DCBHCOMPSEL
    EPwm9Regs.TZDCSEL.bit.DCBEVT1       = TZ_DCBH_HI;
    EPwm9Regs.DCBCTL.bit.EVT1SRCSEL     = DC_EVT1;
    EPwm9Regs.DCBCTL.bit.EVT1FRCSYNCSEL = DC_EVT_ASYNC;
    EPwm9Regs.TZSEL.bit.DCBEVT1         = 1;           // 1/0 - Enable/Disable One Shot Mode

    EPwm10Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3; //Trip 4 is the input to the DCAHCOMPSEL
    EPwm10Regs.TZDCSEL.bit.DCAEVT1       = TZ_DCAH_HI;
    EPwm10Regs.DCACTL.bit.EVT1SRCSEL     = DC_EVT1;
    EPwm10Regs.DCACTL.bit.EVT1FRCSYNCSEL = DC_EVT_ASYNC;
    EPwm10Regs.TZSEL.bit.DCAEVT1         = 1;

    EPwm10Regs.DCTRIPSEL.bit.DCBHCOMPSEL = 3; //Trip 4 is the input to the DCBHCOMPSEL
    EPwm10Regs.TZDCSEL.bit.DCBEVT1       = TZ_DCBH_HI;
    EPwm10Regs.DCBCTL.bit.EVT1SRCSEL     = DC_EVT1;
    EPwm10Regs.DCBCTL.bit.EVT1FRCSYNCSEL = DC_EVT_ASYNC;
    EPwm10Regs.TZSEL.bit.DCBEVT1         = 1;           // 1/0 - Enable/Disable One Shot Mode

    // What do we want the DCAEVT1 events to do?
    // TZA events can force EPWMxA
    // TZB events can force EPWMxB

    EPwm7Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm7Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    EPwm8Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm8Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    EPwm9Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm9Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    EPwm10Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm10Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low


    // Clear any spurious OV trip
    EPwm7Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm8Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm9Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm10Regs.TZCLR.bit.DCAEVT1 = 1;

    // Clear any spurious OV trip
    EPwm7Regs.TZCLR.bit.DCBEVT1 = 1;
    EPwm8Regs.TZCLR.bit.DCBEVT1 = 1;
    EPwm9Regs.TZCLR.bit.DCBEVT1 = 1;
    EPwm10Regs.TZCLR.bit.DCBEVT1 = 1;

    EDIS;

}


//
// End of file
//
