/*
 * MPC_DAB.h
 *
 *  Created on: Oct 11, 2024
 *      Author: levuo
 */

#ifndef MPC_H
#define MPC_H
typedef struct {
// tham so
    float Ro;
    float Lt ;
    float Rt ;
    float Co ;
    float fs ;
    float fc ;
    float T_s;
    float Lo ;
    float N ;
    float pi_val ;
    float Vi;
    float Vout ;
    float Kmpc[1][3];
    float limitMax;
    float limitMin;
    float phi_max;
} CNTL_MPC_CONST;
typedef struct {
    // matrix
    float Iref;
    float x_k[2][1]; // x_k[0] la i_o, x_k[1] la V_c
    float w_k ;
    float u_k ;
    float u;
    float x_k3[2][1];
    float x_k4[2][1];
    float w_k2;
    float Ad[2][2];
    float Ad1[2][1];
    float Ad2[2][1];
    float Bd[2][1];
    float Bd1[2][1];
    float Bd2[2][1];
    float Ed1[2][1];
    float Ed[2][1];
    float Ed2[2][1];
    float Cd[1][2];
    float Az[3][3];
    float Az1[3][1];
    float Bz[3][1];
    float Bz1[3][1];

    //double x_k[2];
    //double w_k;
    //double u_k;
    float Vc;
    float V_c;
    float phi;
    float vc_ref;
    float Io;
    float x_o[2][1];
    float x_o1[2][1];
    float u_o;
    float ek[2][1];
    float z_k[3][1];
    float delta_u;
    float x_k1[2][1];
    float e_k1[2][1];
    float w_k1;
    float z_k1[3][1];
    float z_k2[3][1];
    float delta_u1;
    float discriminant;
    float d;
    float sqrt_val;
    float test[2][2];
    float tt[2][2];
    float n;
    float m;
   //  float i;
   //  float j;
   //  float k;
} CNTL_MPC_CLA_VARS;
void MPC_FUNC(CNTL_MPC_CONST *k, CNTL_MPC_CLA_VARS *v);
#define MPC_CONST(k)         \
        k.Ro = 0;               \
        k.Lt = 0 ;               \
        k.Rt = 0 ;          \
        k.Co = 0 ;           \
        k.fs = 0 ;           \
        k.fc = 0;              \
        k.T_s = 0;          \
        k.Lo = 0 ;           \
        k.N  = 0;             \
        k.pi_val = 0 ;         \
        k.Vout = 0;           \
        k.T_s = 0;       \
        k.limitMax = 0;   \
        k.limitMin = 0;  \
        k.Vi = 0;    \
        memset(k.Kmpc, 0, sizeof(k.Kmpc));\
        k.phi_max = 0;
#define MPC_CURREN_INT(v)            \
        v.Iref = 0;                   \
        memset(v.x_k, 0, sizeof(v.x_k));      \
        v.w_k = 0;                \
        v.u_k =0;                        \
        memset(v.x_k3, 0, sizeof(v.x_k3));      \
        memset(v.x_k4, 0, sizeof(v.x_k4));      \
        v.w_k2 = 0;          \
        memset(v.Ad, 0, sizeof(v.Ad));      \
        memset(v.Ad1,0, sizeof(v.Ad1));     \
        memset(v.Ad2,0, sizeof(v.Ad2));     \
        memset(v.Bd, 0, sizeof(v.Bd));      \
        memset(v.Bd1,0, sizeof(v.Bd1));     \
        memset(v.Bd2,0, sizeof(v.Bd2));     \
        memset(v.Ed, 0, sizeof(v.Ed));      \
        memset(v.Ed1,0, sizeof(v.Ed1));     \
        memset(v.Ed2,0, sizeof(v.Ed2));     \
        memset(v.Cd, 0, sizeof(v.Cd));      \
        memset(v.Az, 0, sizeof(v.Az));      \
        memset(v.Az1,0, sizeof(v.Az1));     \
        memset(v.Bz, 0, sizeof(v.Bz));      \
        memset(v.Bz1,0, sizeof(v.Bz1));     \
        v.Vc = 0;                       \
        v.V_c = 0;                        \
        v.vc_ref = 0;                   \
        v.Io = 0;                  \
        memset(v.x_o, 0, sizeof(v.x_o));      \
        memset(v.x_o1, 0, sizeof(v.x_o1));      \
        v.u_o = 0;                   \
        memset(v.ek,0, sizeof(v.ek));     \
        memset(v.z_k,0, sizeof(v.z_k));     \
        v.delta_u = 0;                    \
        memset(v.x_k1,0, sizeof(v.x_k1));     \
        memset(v.e_k1,0, sizeof(v.e_k1));     \
        v.w_k1 = 0;                    \
        memset(v.z_k1,0, sizeof(v.z_k1));     \
        memset(v.z_k2,0, sizeof(v.z_k2));     \
        v.delta_u1 = 0;      \
        v.discriminant = 0;  \
        v.sqrt_val = 0;          \
        v.d = 0;               \
        v.u = 0;                 \
        memset(v.test,0, sizeof(v.test));     \
        memset(v.tt,0, sizeof(v.tt));     \
        v.n = 0;\
        v.m = 0;\
        v.phi = 0;
//*********** Macro Definition ***********//
/*#define MPC_CURRENT_matrix(v, k)            \
            v.Ad[0][0] = -(k.Ro/k.Lo)*k.T_s + 1.0;   \
            v.Ad[0][1] = (1.0/k.Lo)*k.T_s ;        \
            v.Ad[1][0] = -(1.0/k.Co) * k.T_s;        \
            v.Ad[1][1] = 1.0;                 \
            v.Bd[0][0] = 0;         \
            v.Bd[1][0] = (Vi/(2.0*k.N*k.Lt*k.Co*k.fs)) * k.T_s;          \
            v.Ed[0][0] = -(1.0/k.Lo) * k.T_s;            \
            v.Ed[1][0] = 0;          \
            v.Cd[0][0] = 1.0;           \
            v.Cd[0][1] = 0;              \
            v.Az[0][0] = v.Ad[0][0];     \
            v.Az[0][1] = v.Ad[0][1];     \
            v.Az[0][2] = 0;       \
            v.Az[1][0] = v.Ad[1][0];     \
            v.Az[1][1] = v.Ad[1][1]      \
            v.Az[1][2] = 0;         \
            v.Az[2][0] = 1.0;         \
            v.Az[2][1] = 0;         \
            v.Az[2][2] = 1.0;         \
            v.Bz[0][0] = v.Bd[0][0];      \
            v.Bz[1][0] = v.Bd[1][0];      \
            v.Bz[2][0] = 0;*/
#define CNTL_MPC_CLA_MACRO(k, v)                                                      \
        v.n= v.m;\
       v.test[0][0] = v.tt[0][0]+v.tt[0][0]; \
        v.test[0][1] = v.tt[0][1]; \
        v.test[1][0] = v.tt[1][0]; \
        v.test[1][1] = v.tt[1][1]; \
        v.Ad[0][0] = -(k.Ro/k.Lo)*k.T_s + 1.0;   \
        v.Ad[0][1] = (1.0/k.Lo)*k.T_s ;        \
        v.Ad[1][0] = -(1.0/k.Co) * k.T_s;        \
        v.Ad[1][1] = 1.0;                 \
        v.Bd[0][0] = 0.0;         \
        v.Bd[1][0] = (k.Vi/(2.0*k.N*k.Lt*k.Co*k.fs)) * k.T_s;          \
        v.Ed[0][0] = -(1.0/k.Lo) * k.T_s;            \
        v.Ed[1][0] = 0.0;          \
        v.Cd[0][0] = 1.0;           \
        v.Cd[0][1] = 0.0;              \
        v.Az[0][0] = v.Ad[0][0];     \
        v.Az[0][1] = v.Ad[0][1];     \
        v.Az[0][2] = 0.0;       \
        v.Az[1][0] = v.Ad[1][0];     \
        v.Az[1][1] = v.Ad[1][1];      \
        v.Az[1][2] = 0.0;         \
        v.Az[2][0] = 1.0;         \
        v.Az[2][1] = 0.0;         \
        v.Az[2][2] = 1.0;         \
        v.Bz[0][0] = v.Bd[0][0];      \
        v.Bz[1][0] = v.Bd[1][0];      \
        v.Bz[2][0] = 0.0;               \
        v.Vc = v.V_c*1000.0;   \
    v.x_k[0][0] = v.Io;                                                          \
    v.x_k[1][0] = v.Vc;                                                          \
    /*v.w_k = 0.0;                                                                */ \
    v.vc_ref = k.Vout + k.Ro * v.Iref;                                         \
    v.x_o[0][0] = v.Iref;                                                         \
    v.x_o[1][0] = v.vc_ref;                                                       \
    v.u_o = (2.0 * k.N * k.Lt * k.fs / k.Vi) * v.Iref*(1.0+0.0) ;             \
    v.Ad1[0][0]=v.Ad[0][0]*v.x_o[0][0] + v.Ad[0][1]*v.x_o[1][0];   \
    v.Ad1[1][0]=v.Ad[1][0]*v.x_o[0][0] + v.Ad[1][1]*v.x_o[1][0];    \
    v.Bd1[0][0]=v.Bd[0][0]*v.u_o;             \
    v.Bd1[1][0]=v.Bd[1][0]*v.u_o;            \
    v.Ed1[0][0] = v.Ed[0][0]*k.Vout;                      \
    v.Ed1[1][0] = v.Ed[1][0]*k.Vout;                      \
    v.x_o1[0][0] = v.Ad1[0][0]+v.Ed1[0][0]+v.Bd1[0][0];      \
    v.x_o1[1][0] = v.Ad1[1][0]+v.Ed1[1][0]+v.Bd1[1][0];      \
    v.ek[0][0] = v.x_k[0][0]-v.x_o1[0][0];                    \
    v.ek[1][0] = v.x_k[1][0]-v.x_o1[1][0];                    \
    v.w_k1 = 3.0*v.w_k + v.Cd[0][0] * v.ek[0][0] + v.Cd[1][0] * v.ek[0][1];                \
    v.z_k[0][0] = v.ek[0][0];                                                        \
    v.z_k[1][0] = v.ek[1][0];                                                        \
    v.z_k[2][0] = v.w_k1;                                                         \
    v.delta_u = k.Kmpc[0][0] * v.z_k[0][0] + k.Kmpc[0][1] * v.z_k[1][0] + k.Kmpc[0][2] * v.z_k[2][0]; \
    v.u_k = v.u_o + v.delta_u;                                                 \
    v.Ad2[0][0] = v.Ad[0][0]*v.x_k[0][0]+v.Ad[0][1]*v.x_k[1][0];                \
    v.Ad2[1][0] = v.Ad[1][0]*v.x_k[0][0]+v.Ad[1][1]*v.x_k[1][0];                \
    v.Bd2[0][0] = v.Bd[0][0]*v.u_k;                                            \
    v.Bd2[0][1] = v.Bd[0][1]*v.u_k;                                            \
    v.Ed2[0][0] = v.Ed[0][0]*v.Vc;                      \
    v.Ed2[1][0] = v.Ed[1][0]*v.Vc;                      \
    v.x_k1[0][0] = v.Ad2[0][0]+v.Ed2[0][0]+v.Bd2[0][0];      \
    v.x_k1[1][0] = v.Ad2[1][0]+v.Ed2[1][0]+v.Bd2[1][0];     \
    v.e_k1[0][0] = v.x_k1[0][0]-v.x_o1[0][0];                                       \
    v.e_k1[1][0] = v.x_k1[1][0]-v.x_o1[1][0];                                       \
    v.z_k1[0][0] = v.e_k1[0][0];                                                      \
    v.z_k1[1][0] = v.e_k1[1][0];                                                      \
    v.w_k2 =  3.0*v.w_k1 + v.Cd[0][0] * v.e_k1[0][0] + v.Cd[0][1] * v.e_k1[1][0];           \
    v.z_k[2][0] = v.w_k2;                                                         \
    v.Az1[0][0] = v.Az[0][0]*v.z_k1[0][0]+v.Az[0][1]*v.z_k1[1][0]+ v.Az[0][2]*v.z_k1[2][0];      \
    v.Az1[1][0] = v.Az[1][0]*v.z_k1[0][0]+v.Az[1][1]*v.z_k1[1][0]+ v.Az[1][2]*v.z_k1[2][0];      \
    v.Az1[2][0] = v.Az[2][0]*v.z_k1[0][0]+v.Az[2][1]*v.z_k1[1][0]+ v.Az[2][2]*v.z_k1[2][0];      \
    v.Bz1[0][0] = v.Bz[0][0]*v.delta_u;                            \
    v.Bz1[1][0] = v.Bz[1][0]*v.delta_u;                            \
    v.Bz1[2][0] = 0.0;   \
    v.z_k2[0][0] = v.Az1[0][0] + v.Bz1[0][0];                             \
    v.z_k2[1][0] = v.Az1[1][0] + v.Bz1[1][0];                             \
    v.z_k2[2][0] = v.Az1[2][0] + v.Bz1[2][0];                             \
    v.delta_u1 = k.Kmpc[0][0] * v.z_k2[0][0] + k.Kmpc[0][1] * v.z_k2[1][0] + k.Kmpc[0][2] * v.z_k2[2][0]; \
    v.u = v.u_o + v.delta_u1;                                                                                                                           \
    v.discriminant = 1.0 - 4.0 * v.u;                                            \
    if (v.discriminant >= 0.0) {                                                    \
            v.sqrt_val = __sqrt(v.discriminant);                                     \
            v.d = (1.0 - v.sqrt_val) / 2.0;                                        \
        } else {                                                                   \
            v.sqrt_val = 0.5;                \
            v.d = (1.0 - v.sqrt_val) / 2.0;                                        \
        }        \
/*   if (v.discriminant < 0) {                                                  \
        v.sqrt_val = sqrt(-v.discriminant);                                    \
        if (v.u >= 0) {                                                      \
            v.d = (1 - v.sqrt_val * I) / 2.0;                                    \
        } else {                                                               \
            v.d = (-1 + v.sqrt_val * I) / 2.0;                                   \
        }                                                                      \
    } else {                                                                   \
        v.sqrt_val = sqrt(v.discriminant);                                     \
        if (v.u >= 0) {                                                      \
            v.d = (1 - v.sqrt_val) / 2.0;                                        \
        } else {                                                               \
            v.d = (-1 + v.sqrt_val) / 2.0;                                       \
        }                                                                      \
    }  */                                                                        \
    v.phi = v.d;                                                        \
    if (v.phi > k.limitMax) v.phi = k.limitMax;                                   \
    if (v.phi < k.limitMin) v.phi = k.limitMin;
    //if (creal(v.d) < k.limitMin) v.phi = k.limitMin;

#endif /* 2_LIBRUARY_APP_LIB_CONTROLLER_INCLUDE_MPC_DAB_H_ */
