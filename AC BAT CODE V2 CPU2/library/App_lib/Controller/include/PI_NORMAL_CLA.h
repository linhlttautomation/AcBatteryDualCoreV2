/*
 * PI.h
 *
 *  Created on: 10 Mar 2017
 *      Author: dinhngock6
 */

#ifndef _PI_H_
#define _PI_H_

typedef struct
{
   float Ts;
   float Kp;
   float Ki;
   float IN_max;
   float OUT_max;
   float heso_P;
   float heso_I;
   float REF;
   float INPUT;
   float ERROR;
   float OUT_P;
   float OUT_I;
   float OUT;
   float LimitMax;
   float LimitMin;
}PI_NORMAL_CLA;

#define PI_NORMAL_INT(v)            \
        v.Ts = 0.0f;                       \
        v.Kp = 0.0f;                       \
        v.Ki = 0.0f;                       \
        v.IN_max = 0.0f;                   \
        v.OUT_max = 0.0f;                  \
        v.heso_P = 0.0f;                   \
        v.heso_I = 0.0f;                   \
        v.REF = 0.0f;                      \
        v.INPUT = 0.0f;                    \
        v.ERROR = 0.0f;                    \
        v.OUT_P = 0.0f;                    \
        v.OUT_I = 0.0f;                    \
        v.LimitMax = 0.0f;                    \
        v.LimitMin = 0.0f;                    \
        v.OUT = 0.0f;


#define PI_NORMAL_MACRO(v)                                      \
        v.heso_P   = v.Kp*v.IN_max/v.OUT_max;                   \
        v.heso_I   = v.Ki*v.Ts*v.IN_max/v.OUT_max;              \
        v.OUT_P    = v.heso_P*v.ERROR;                          \
        v.OUT_I    = v.OUT_I + v.heso_I*v.ERROR;                \
        v.OUT      = v.OUT_P + v.OUT_I;                         \
        if(v.OUT > v.LimitMax) v.OUT = v.LimitMax;             \
        if(v.OUT < v.LimitMin) v.OUT = v.LimitMin;              \
        if(v.OUT_I > v.LimitMax) v.OUT_I = v.LimitMax;             \
        if(v.OUT_I < v.LimitMin) v.OUT_I = v.LimitMin;
#define PI_NORMAL_V2_MACRO(v)                                   \
        v.heso_P   = v.Kp*v.IN_max/v.OUT_max;                   \
        v.heso_I   = v.Ki*v.Ts*v.IN_max/v.OUT_max;              \
        v.OUT_P    = v.heso_P*v.ERROR;                          \
        if(v.OUT >= 1.0f ||v.OUT <= -1.0f ) v.OUT_I = v.OUT_I;    \
        else v.OUT_I    = v.OUT_I + v.heso_I*v.ERROR;           \
        v.OUT      = v.OUT_P + v.OUT_I;                         \
        if(v.OUT > v.LimitMax) v.OUT = v.LimitMax;             \
        if(v.OUT < v.LimitMin) v.OUT = v.LimitMin;


#endif /* 2_LIBRUARY_INCLUDE_CONTROLLER_PI_H_ */
