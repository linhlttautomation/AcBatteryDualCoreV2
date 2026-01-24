/*
 * TPC_Setting.h
 *
 *  Created on: Jul 31, 2025
 *      Author: TUNG BUI K65
 */

#ifndef _TPC_SETTING_H_
#define _TPC_SETTING_H_


// Define Level
#define LEVEL1          1           // Vong ho discharge
#define LEVEL3          3           // vong kin discharge DAB
#define LEVEL5          5           // Vong kin discharge CFDAB

#define BUILDLEVEL      LEVEL1

// Define Mode_Run
#define CFDAB_MODE      1
#define VFDAB_MODE      2

#define TPC_MODE_RUN    VFDAB_MODE


#define     NOT_ALLOW_SWITCH_BDK      0
#define     ALLOW_SWITCH_BDK          1

#define     SWITCH_BDK_MODE    ALLOW_SWITCH_BDK
//
#define PV_INVERTER_ADC 1                                           // 0 HIL , 1 = Exp
#define ADC_PU_SCALE_FACTOR_16BIT        0.0000152587890625f         //1/2^12=0.000244140625
#define ADC_PU_SCALE_FACTOR_12BIT        0.000244140625f             //1/2^12=0.000244140625
#define ADC_PU_PPB_SCALE_FACTOR          0.000488281250f             //1/2^11
#define SD_PU_SCALE_FACTOR               0.000030517578125f
#define Scale                            1.0f

#define PI 3.14159265358979f
#define DUTY_MAX        1.0

#define Udc_max         800.0f
#define Ubat_max        200.0f
#define Ibat_max        50.0f
#define Uc_max          600.0f

#define T                   0.00001                                 // time sample
//#define MEAUBAT(A) (A/Ubat_max*4096.0+43)

#if (PV_INVERTER_ADC == 0)
//
#endif
//
#if (PV_INVERTER_ADC == 1)

#define UDC_HCPL    AdcaResultRegs.ADCRESULT14
#define UBAT_HCPL   AdcaResultRegs.ADCRESULT4
#define VC_HCPL     AdcaResultRegs.ADCRESULT2
#define IHV_LEM     AdccResultRegs.ADCRESULT2
#define ILV_LEM     AdccResultRegs.ADCRESULT4
#endif

// LEM    1.0pu current ==> 50.0A -> 2048 counts
#define LEM_2(A)     2024.0f-132.0f+(2048.0f*A/Ibat_max)
#define LEML_2(A)    2024.0f+(2048.0f*A/Ibat_max/1.1f)-165.0f
#define MEAUBAT(A)   (A/Ubat_max*4096.0f)
#define MEAUDC(A)    (4096.0f*A/Udc_max-5.0f)/1.093
#define MEAUC(A)     (A/Uc_max*4096.0f+7.0f)/1.088

#if(TPC_MODE_RUN == CFDAB_MODE)
// Discharge Close_Loop
#if (BUILDLEVEL == LEVEL5)

// PRECIOUS PARAMETERS

//#define KP_VOLT_UDC_LOOP        0.0002
//#define KI_VOLT_UDC_LOOP        0.05
//#define KP_VOLT_VC_LOOP         0.000001
//#define KI_VOLT_VC_LOOP         0.00003
//#define KD_VOLT_VC_LOOP         0

//12V_Vbat

//#define KP_VOLT_UDC_LOOP        0.0002
//#define KI_VOLT_UDC_LOOP        0.05
//#define KP_VOLT_VC_LOOP         0.00001
//#define KI_VOLT_VC_LOOP         0.00005
//#define KD_VOLT_VC_LOOP         0

//14V_Vbat

//#define KP_VOLT_UDC_LOOP        0.00002
//#define KI_VOLT_UDC_LOOP        0.05
//#define KP_VOLT_VC_LOOP         0.000025
//#define KI_VOLT_VC_LOOP         0.00003
//#define KD_VOLT_VC_LOOP         0

////29V_Vbat_ source

//#define KP_VOLT_UDC_LOOP        0.00002
//#define KI_VOLT_UDC_LOOP        0.05
//#define KP_VOLT_VC_LOOP         0.000025
//#define KI_VOLT_VC_LOOP         0.00003
//#define KD_VOLT_VC_LOOP         0

//120V_Vbat

//#define KP_VOLT_UDC_LOOP        0.00002*2
//#define KI_VOLT_UDC_LOOP        0.05/6
//#define KP_VOLT_VC_LOOP         0.000025
//#define KI_VOLT_VC_LOOP         0.00003
//#define KD_VOLT_VC_LOOP         0

//#define KP_VOLT_UDC_LOOP          0.00002*2 //source
//#define KI_VOLT_UDC_LOOP          0.05/6 //source
//
//#define KP_VOLT_VC_LOOP           0.000025
//#define KI_VOLT_VC_LOOP           0.00003
//#define KD_VOLT_VC_LOOP           0

#define KP_VOLT_UDC_LOOP          0.00002f*4.5f //source
#define KI_VOLT_UDC_LOOP          0.05f/6.0f //source

#define KP_VOLT_VC_LOOP           0.0000265f
#define KI_VOLT_VC_LOOP           0.0000285f
#define KD_VOLT_VC_LOOP           0


#define KP_VOLT_UDC_LOOP_LOAD          0.00002f*5.0f  //source
#define KI_VOLT_UDC_LOOP_LOAD          0.05f/12.0f    //source

#define KP_VOLT_VC_LOOP_LOAD           0.0000265f
#define KI_VOLT_VC_LOOP_LOAD           0.0000285f
#define KD_VOLT_VC_LOOP_LOAD           0

//5V_Vbat

//#define KP_VOLT_UDC_LOOP        0.0002
//#define KI_VOLT_UDC_LOOP        0.0002
//#define KP_VOLT_VC_LOOP         0.000001
//#define KI_VOLT_VC_LOOP         0.00003
//#define KD_VOLT_VC_LOOP         0


//#define KP_VOLT_UDC_LOOP        0.00001   //_12_18   //_5v
//#define KP_VOLT_UDC_LOOP        0.0008 //10V

//#define KI_VOLT_UDC_LOOP        0.2860
//#define KI_VOLT_UDC_LOOP        0.01

//#define KI_VOLT_UDC_LOOP        0.0001  //_5v
//#define KI_VOLT_UDC_LOOP        0.008 //10V

//#define KP_VOLT_VC_LOOP         0.00001
//#define KP_VOLT_VC_LOOP         0.000005 _//10V

//#define KI_VOLT_VC_LOOP         0.000015

#endif

// SETTING CFDAB

#define CFDAB_Power                    2500
#define CFDAB_Voltage                  400
#define CFDAB_MaxCharge_Current        10
#define CFDAB_MaxDischarge_Current     10

#define CFDAB_UdcRef                    42
#define CFDAB_VcRef                     28

#define CFDAB_UbatRef                   5
#define CFDAB_IbatRef                   5.2

#define CFDAB_Udc_Max                   460
#define CFDAB_Udc_Min                   350

#define CFDAB_Vc_Max                    460
#define CFDAB_Vc_Min                    0

#define CFDAB_Ubat_Max                  160
#define CFDAB_Ubat_Min                  0

#define CFDAB_Ibat_Max                  10

// CMPSS TPC Permission
#define CMPSS_PROTECT_Ubat_UPPER        1

#define CMPSS_PROTECT_Ihv_UPPER         0

#define CMPSS_PROTECT_Ilv               0

#define CMPSS_PROTECT_Uclamp_UPPER      1

#define CMPSS_PROTECT_Udc_UPPER         1
#endif
#if(TPC_MODE_RUN == VFDAB_MODE)
//#define T                   0.00002            // time sample
//#define T                   0.00001            // time sample
//#define Isr_Frequency       50000
#define Isr_Frequency       100000
#define Frequency_Clock_PWM 200000000

#define L_t                  0.000013
#define R_t                  0.0001
#define C_o                  0.00005
#define R_o                  0.001
#define L_o                  0.00033
#define N_1                   2.25
#define V_i                  66.667
#define V_out                  150.0
#define F_s                    50000.0

//--------------------------------------------------------
#if (BUILDLEVEL == LEVEL3) // Discharge

// 150Vdc ok
//#define KP_VOLT_UDC_LOOP     0.0006     //0.0001
//#define KI_VOLT_UDC_LOOP     0.008      //0.001
//#define KP_VOLT_UDC_LOOP     0.0001     //0.0001 vong ho ko tai 1kW
//#define KI_VOLT_UDC_LOOP     0.001      //0.001

/* che do khong tai -> dong tai
#define KP_VOLT_UDC_LOOP     0.0001*2.0     //*6.0
#define KI_VOLT_UDC_LOOP     0.001      //0.02/9.0

#define KP_VOLT_UDC_LOOP_1    0.0001*16.0
#define KI_VOLT_UDC_LOOP_1    0.001*7.0
*/
//--------------- co tai tu dau
/*
#define KP_VOLT_UDC_LOOP     0.0001*4.0     //*9
#define KI_VOLT_UDC_LOOP     0.001*1.5      //4

#define KP_VOLT_UDC_LOOP_1    0.0001*12.0
#define KI_VOLT_UDC_LOOP_1    0.001*6.0
*/
#define KP_VOLT_UDC_LOOP     0.0001f*2.0f     //*9
#define KI_VOLT_UDC_LOOP     0.001f*1.0f      //4

#define KP_VOLT_UDC_LOOP_1    0.0001f*12.0f
#define KI_VOLT_UDC_LOOP_1    0.001f*6.0f
// 4:18PM
//#define KP_VOLT_UDC_LOOP     0.00001
//#define KI_VOLT_UDC_LOOP     0.001

//#define KP_VOLT_UDC_LOOP     0.00001
//#define KI_VOLT_UDC_LOOP     0.1

#endif

#if (BUILDLEVEL == LEVEL2) // Discharge double

// 150Vdc ok
#define KP_VOLT_UDC_LOOP_V_1     0.2     //0.0001
#define KI_VOLT_UDC_LOOP_V_1     0.08      //0.001   //not load

#define KP_VOLT_UDC_LOOP_V     0.8     //0.0001
#define KI_VOLT_UDC_LOOP_V     0.08      //0.001

#define KP_LOOP_I_1    0.004         //not load
#define KI_LOOP_I_1    0.001

#define KP_LOOP_I    0.01
#define KI_LOOP_I    0.04
// 4:18PM
//#define KP_VOLT_UDC_LOOP     0.00001
//#define KI_VOLT_UDC_LOOP     0.001

//#define KP_VOLT_UDC_LOOP     0.00001
//#define KI_VOLT_UDC_LOOP     0.1

#endif

#if (BUILDLEVEL == LEVEL4)  // Charge
#define KP_VOLT_UBAT_LOOP   0.000002
#define KI_VOLT_UBAT_LOOP   0.3
#endif

#if (BUILDLEVEL == LEVEL5)

#define KP_VOLT_UDC_LOOP      0.000625
//#define KP_VOLT_UDC_LOOP      0.001

//#define KI_VOLT_UDC_LOOP     0.2860
#define KI_VOLT_UDC_LOOP      0.2


#endif

#if (BUILDLEVEL == LEVEL7)

#define KP_VOLT_UBAT_LOOP     0.1941
#define KI_VOLT_UBAT_LOOP     1.0

#endif
#if (BUILDLEVEL == LEVEL8) // Discharge

// 150Vdc ok
//#define KP_VOLT_UDC_LOOP     1.0003       //old  //0.0001
//#define KI_VOLT_UDC_LOOP     3.005          // 0.001

#define KP_VOLT_UDC_LOOP_1     0.06      //(date12/2:0.05)   //0.0001
#define KI_VOLT_UDC_LOOP_1     0.4        //(date12/2:0.4) // 0.001

#define KP_VOLT_UDC_LOOP     0.9         //0.0001
#define KI_VOLT_UDC_LOOP     2.1         // 0.001

//#define KP_VOLT_UDC_LOOP_1    0.000005*4
//#define KI_VOLT_UDC_LOOP_1    0.02/15

// 4:18PM
//#define KP_VOLT_UDC_LOOP     0.00001
//#define KI_VOLT_UDC_LOOP     0.001

//#define KP_VOLT_UDC_LOOP     0.00001
//#define KI_VOLT_UDC_LOOP     0.1

#endif

#define VFDAB_Power                    2500
#define VFDAB_Voltage                  400
#define VFDAB_MaxCharge_Current        10
#define VFDAB_MaxDischarge_Current     10

#define VFDAB_UdcRef                   400                          // Set Udc_ref: Level 3
#define VFDAB_UdcThreshold             396                          // Set : Level 3
#define VFDAB_IoutRef                  0.3

#define VFDAB_VcRef                    0                            // Set Ubat_ref here: Level 4

#define VFDAB_UbatRef                  10
#define VFDAB_IbatRef                  5

#define VFDAB_Udc_Max                  460
#define VFDAB_Udc_Min                  350

#define VFDAB_Vc_Max                   460
#define VFDAB_Vc_Min                   0

#define VFDAB_Ubat_Max                 160
#define VFDAB_Ubat_Min                 0

#define VFDAB_Ibat_Max                 10
#define VFDAB_Iout_Max                 6
#define BAT_IBAT_REF_MAX               1.5

#define BAT_IOUT_REF_MAX            2
//look up the signal makes the system be protected

#define ERROR_NONE                  0
#define ERROR_I_BAT_HIGH            1
#define ERROR_VDC_HIGH              2
#define ERROR_VC_HIGH               3
#define ERROR_VBAT_HIGH             4

#define ERROR_VC_LOW                5
#define ERROR_VBAT_LOW              6
#define ERROR_VDC_LOW               7

#define ERROR_CONTROLLER            8

// CMPSS TPC Permission
#define CMPSS_PROTECT_Ubat_UPPER        1

#define CMPSS_PROTECT_Ihv_UPPER         1

#define CMPSS_PROTECT_Ilv               0

#define CMPSS_PROTECT_Uclamp_UPPER      1

#define CMPSS_PROTECT_Udc_UPPER         1
#endif

#endif /* _TPC_SETTING_H_ */
