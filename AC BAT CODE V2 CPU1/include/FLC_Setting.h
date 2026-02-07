/*
 * FLC_Setting.h
 *
 *  Created on: 27/7/2025
 *  Author: LUU LINH K65
 */

#ifndef _FLC_SETTING_H_
#define _FLC_SETTING_H_

// Define Select Level
#define LEVEL1        1           // Vong ho doc lap 3P4N
#define LEVEL2        2           // Vong ho doc lap IFB
#define LEVEL3        3           // Vong dong doc lap 3P4N
#define LEVEL4        4           // Vong ap doc lap 3P4N
#define LEVEL5        5           // Vong dong IFB
#define LEVEL6        6           // Vong ap IFB
#define LEVEL7        7           // Vong dong noi luoi 3P4N
#define LEVEL8        8           // Vong ap noi luoi 3P4N

// ---------------------------------------------------
#define BUILDLEVEL    LEVEL6
// ---------------------------------------------------

#define SINGLE_PHASE_MODE       1
#define THREE_PHASE_MODE        3

// ---------------------------------------------------
#define SET_MODE_RUN          SINGLE_PHASE_MODE
// ---------------------------------------------------

#define NONE                               0
#define READ_VOLTAGE_AC_BEFORE_LPF         1
#define READ_VOLTAGE_AC_AFTER_LPF          2
#define READ_VOLTAGE_DC                    3
#define READ_CURRENT                       4

// ---------------------------------------------------
#define SET_MODE_SWITCH_READ               0 // YES; 0 // NO
#define SET_MODE_READ                      READ_VOLTAGE_AC_AFTER_LPF
// ---------------------------------------------------

typedef enum {
    READ_VOLTAGE_AC_AFTER_LPF_e = 0,
    READ_VOLTAGE_AC_BEFORE_LPF_e = 1,
    READ_VOLTAGE_DC_e = 2,
    READ_CURRENT_e = 3
} eReadMode;

extern volatile eReadMode eSET_MODE_READ;

// ---------------------------------------------------
#define TUNNING_ADC                 2 // 1: Manual; 2: No
// ---------------------------------------------------

#define ALLOW_IPC_CPU               0
#define ALLOW_CALIB_PI              0
#define ALLOW_TIMER                 1
#define ALLLOW_DAC                  0
#define ALLOW_EPWM_INT              1
#define ALLOW_ADC_INT               1
#define ALLOW_FLC_AUTO_START_UP     0

// ---------------------------------------------------
#define MODE_MODULATION_SVM2D           2
#define MODE_MODULATION_SVM3D           3
#define MODE_MODULATION_SVM3D_2         4

// ---------------------------------------------------
#define MODE_MODULATION             MODE_MODULATION_SVM3D_2
// ---------------------------------------------------

#if(BUILDLEVEL == LEVEL4)

    #define VAC_LOAD_64_FLC_30             30
    #define VAC_LOAD_64_FLC_60             60
    #define VAC_LOAD_64_FLC_100            100
    #define VAC_LOAD_64_FLC_130            130

// ---------------------------------------------------
#define SET_MODE_CONTROL                   VAC_LOAD_64_FLC_130
// ---------------------------------------------------

#endif

#if (BUILDLEVEL == LEVEL5)

#define KP_CURR_LOOP_PR        2.953f ;//2.953;
#define KI_CURR_LOOP_PR        3000.0f; //6159.173;

#define KP_CURR_LOOP_PI         3.4f
#define KI_CURR_LOOP_PI         800.0f
#endif

#if (BUILDLEVEL == LEVEL6)
#define KP_CURR_LOOP_PR        1.0f; //2.953;
#define KI_CURR_LOOP_PR        1000.0f; //6159.173;

#define KP_voltage_LOOP_PR        0.05f;
#define KI_voltage_LOOP_PR        200.0f;
#endif

// Define constant math
#define PI 3.14159265358979f
#define can2 1.414213562f
#define can3 1.732050808f
#define can6 2.449489743f

// Define read ADC FLC
#define UDC_HCPL        AdcbResultRegs.ADCRESULT1 //B1

#define VaG_HCPL        AdcdResultRegs.ADCRESULT2 //D2
#define VbG_HCPL        AdcdResultRegs.ADCRESULT4 //D4

#define VcG_HCPL        AdcdResultRegs.ADCRESULT3 //D3

#define IA_INV_LEM      AdcdResultRegs.ADCRESULT0 //D0
#define IB_INV_LEM      AdcdResultRegs.ADCRESULT1 //D1
#define IC_INV_LEM      AdcbResultRegs.ADCRESULT0 //B2
#define IZ_INV_LEM      AdcbResultRegs.ADCRESULT2 //B0

#define T_Us             0.002f     // Time sample voltage Us
#define Ti               0.00002f   // Time sample current
#define T                0.00002f   // Time sample
#define T_Udc            0.0002f    // Time sample voltage Udc

#if (BUILDLEVEL == LEVEL3)

    #define KP_CURR_LOOP            3.0f
    #define KI_CURR_LOOP            1500.0f

    #define KP_CURR_LOOP_Z          3.0f
    #define KI_CURR_LOOP_Z          1500.0f

#endif

#if(BUILDLEVEL == LEVEL4)

    #if(SET_MODE_CONTROL == VAC_LOAD_64_FLC_30 && ALLOW_CALIB_PI == 0) //ok
        #define KP_CURR_LOOP_1            1.0f/2
        #define KI_CURR_LOOP_1            200.0f*2
    #endif

    #if(SET_MODE_CONTROL == VAC_LOAD_64_FLC_60 && ALLOW_CALIB_PI == 0) //ok
        #define KP_CURR_LOOP_1            1.0f/2
        #define KI_CURR_LOOP_1            200.0f*2
    #endif

    #if(SET_MODE_CONTROL == VAC_LOAD_64_FLC_100 && ALLOW_CALIB_PI == 0) //ok
        #define KP_CURR_LOOP_1            1.0f/2
        #define KI_CURR_LOOP_1            200.0f*2
    #endif

    #if(SET_MODE_CONTROL == VAC_LOAD_64_FLC_130 && ALLOW_CALIB_PI == 0) //ok
        #define KP_CURR_LOOP_1            1.0f/2
        #define KI_CURR_LOOP_1            200.0f*2
    #endif

    #define KP_VOLT_US_LOOP           0.00001f
    #define KI_VOLT_US_LOOP           0.1f

    #define STEP_RAMP_VAC             0.0005f/10.0f

#endif

#if (BUILDLEVEL == LEVEL7)

    //#define KP_CURR_LOOP            1.15       // L filter 1.7mH: 17.0   --- L Filter 115uH: 1.15
    //#define KI_CURR_LOOP            800.0      // L filter 1.7mH: 800.0  --- L Filter 115uH: 800.0
    //
    #define KP_CURR_LOOP_Z          3.0       // L filter 1.7mH: 68.0   --- L Filter 115uH: 4.60
    #define KI_CURR_LOOP_Z          1500.0     // L filter 1.7mH: 3200.0 --- L Filter 115uH: 3200.0

    #define KP_CURR_LOOP            3.0       // L filter 1.7mH: 17.0   --- L Filter 115uH: 1.15
    #define KI_CURR_LOOP            1500.0      // L filter 1.7mH: 800.0  --- L Filter 115uH: 800.0

    //#define KP_CURR_LOOP_Z          4.60       // L filter 1.7mH: 68.0   --- L Filter 115uH: 4.60
    //#define KI_CURR_LOOP_Z          3200.0     // L filter 1.7mH: 3200.0 --- L Filter 115uH: 3200.0

    #define KP_PLL                  4.0
    #define KI_PLL                  100.0

#endif

#if (BUILDLEVEL == LEVEL8)

    #define KP_CURR_LOOP            3.0
    #define KI_CURR_LOOP            1500.0

    #define KP_CURR_LOOP_Z          3.0
    #define KI_CURR_LOOP_Z          1500.0

    #define KP_VOLT_UDC_LOOP        0.1
    #define KI_VOLT_UDC_LOOP        5.0

#endif

// Define the base quantities for PU system conversion
#define NORMAL_FREQ     50.0f
#define BASE_FREQ       150.0f
#define Udc_max         800.0f
#define Us_max          400.0f
#define Is_max          10.0f
#define Wref            (2.0f*PI*NORMAL_FREQ)
#define Wmax            (2.0f*PI*BASE_FREQ)

//SW FLC Permission
#define SW_PROTECT_UDC_UPPER            1
#define SW_PROTECT_UDC_UNDER            1

// CMPSS FLC Permission
#define CMPSS_PROTECT_UDC_UPPER         0

#define CMPSS_PROTECT_VaG_UPPER         1 // Protect ok
#define CMPSS_PROTECT_VaG_LOWER         1 // Protect ok

#define CMPSS_PROTECT_VbG_UPPER         0
#define CMPSS_PROTECT_VbG_LOWER         0

#define CMPSS_PROTECT_VcG_UPPER         0
#define CMPSS_PROTECT_VcG_LOWER         0

#define CMPSS_PROTECT_Ia_inv_UPPER      1 // Protect ok
#define CMPSS_PROTECT_Ia_inv_LOWER      1 // Protect ok

#define CMPSS_PROTECT_Ib_inv_UPPER      0
#define CMPSS_PROTECT_Ib_inv_LOWER      0

#define CMPSS_PROTECT_Ic_inv_UPPER      1 // Protect ok
#define CMPSS_PROTECT_Ic_inv_LOWER      1 // Protect ok

// CMPSS FLC Setting
extern volatile float CMPSS_Vg_Rms_Protection;
extern volatile float CMPSS_Ig_Rms_Protecion;

#endif /* _FLC_SETTING_H_ */
