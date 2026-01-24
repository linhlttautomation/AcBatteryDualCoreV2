/*
 * TPC_Variables.h
 *
 *  Created on: Jul 31, 2025
 *      Author: TUNG BUI K65
 */

#ifndef _TPC_VARIABLES_H_
#define _TPC_VARIABLES_H_


#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    unsigned int ID;
    unsigned int WORD_ONE;
    unsigned int WORD_SEC;
    unsigned int WORD_THREE;
    unsigned int WORD_FOUR;
}MsgDataCan;

typedef struct
{
    float Power;
    float Voltage;
    float ChargeCurrentMax;
    float DisChargeCurrentMax;

    float  UdcRef;
    float  VcRef;
    float  UbatRef;
    float  IbatRef;

    float  UdcMax;
    float  UdcMin;
    float  VcMax;
    float  VcMin;
    float  UbatMax;
    float  UbatMin;
    float  IbatMax;

    float  IoutRef;
    float  cmpc;
    float  IoutMax;
    float  UdcThreshold;
}SETTING_BAT;

// CLA ---> CPU

typedef struct
{
    float Udc_CFDAB;
    float Udc;
    float Ubat;
    float Vc;
    float Ilv;
    float Ihv;
    float I_out;
    float datalog1;
    float datalog2;
    float Ibat;
}ADC_VALUE;


typedef struct
{
    float theta1;
    float theta2;
    float duty;
}MEASUREMENT_VAULE;

typedef struct
{
    ADC_VALUE ADC_CPU;
    MEASUREMENT_VAULE MEASUARE_CPU;
    int32 flagRun;
    int32 flagErr;
    int32 count;
}CLA_TO_CPU;

//----------------------------------------------------------------------------------------

typedef struct
{
    float UdcTesting;
    float UbatTesting;
    float UcTesting;

    float DutyTesting;
    float DutyStart;

    float DutyTesting_Discharge;
    float DutyTesting_Charge;

    float PhiTesting;
    float PhiStart;

    float UbatRef;
    float IbatRef;
    float UdcRef;
    float VcRef;

    float Theta1;
    float Theta2;

    short EnableFlag;
    short EnableADC;
    float cmpc;
    float UdcThreshold;
}CPU_TO_CLA;

typedef struct
{
    Uint16 Udc_TPC_Upper;
    Uint16 Uclamp_Upper;
    Uint16 Ubat_Upper;
    Uint16 Ilv;
    Uint16 Ihv;

}PROTECT_CHANEL_TPC;


#endif /* _TPC_VARIABLES_H_ */
