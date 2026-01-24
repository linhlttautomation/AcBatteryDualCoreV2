/*
 * SVM3D_DQZ_CLA_TOI_UU2.h
 *
 *  Created on: 27/7/2025
 *  Author: LUU LINH K65
 *  Toi uu SVM-3D Ver2
*/

#ifndef _SVM3D_DQZ_CLA_TOI_UU2_H_
#define _SVM3D_DQZ_CLA_TOI_UU2_H_

#define SQRT3_F      1.732050807568877f
#define SQRT3_2_F    0.8660254037844386f
#define INV3_2_F     1.5f
#define HALF_F       0.5f
#define DUTY_MAX_F   0.98f
#define DUTY_MIN_F   0.0f

typedef struct  {
    Uint16 Prism;
    float  Ualpha;    // Input
    float  Ubeta;     // Input
    float  Ugamma;    // Input
    float  Udc;       // Input
    float  da;        // Output
    float  db;        // Output
    float  dc;        // Output
    float  dn;        // Output
    float  Ua;
    float  Ub;
    float  Uc;
    float  d1;
    float  d2;
    float  d3;
    float  d0;
    float  inver_Udc;
} SVM3D_DQZ_GEN_CLA_2;

#define SVM3D_DQZ_GEN_CLA_INT_2(v) \
    do {                           \
        (v).Ualpha = 0.0f;         \
        (v).Ugamma = 0.0f;         \
        (v).Ubeta  = 0.0f;         \
        (v).Udc    = 0.0f;         \
        (v).da     = 0.0f;         \
        (v).db     = 0.0f;         \
        (v).dc     = 0.0f;         \
        (v).dn     = 0.0f;         \
        (v).Ua     = 0.0f;         \
        (v).Ub     = 0.0f;         \
        (v).Uc     = 0.0f;         \
        (v).d1     = 0.0f;         \
        (v).d2     = 0.0f;         \
        (v).d3     = 0.0f;         \
        (v).d0     = 0.0f;         \
        (v).inver_Udc = 0.0f;      \
        (v).Prism  = 0;            \
    } while(0)

/* =========================
   Helper inline functions
   ========================= */

static inline void calc_uvw_1(SVM3D_DQZ_GEN_CLA_2 *v)
{
    v->Ua = v->Ualpha + v->Ugamma;
    v->Ub = -HALF_F * v->Ualpha + SQRT3_2_F * v->Ubeta + v->Ugamma;
    v->Uc = -HALF_F * v->Ualpha - SQRT3_2_F * v->Ubeta + v->Ugamma;
}

static inline void select_prism_1(SVM3D_DQZ_GEN_CLA_2 *v)
{
    float fabsUalpha = fabsf(v->Ualpha);
    float ratio = fabsf(v->Ubeta) / SQRT3_F;
    int condition1 = (v->Ualpha * v->Ubeta >= 0.0f);
    int condition2 = (v->Ubeta >= 0.0f);

    if (condition1 && condition2) {
        v->Prism = (fabsUalpha >= ratio) ? 1u : 2u;
    } else if (condition1 && !condition2) {
        v->Prism = (fabsUalpha >= ratio) ? 4u : 5u;
    } else if (!condition1 && condition2) {
        v->Prism = (fabsUalpha >= ratio) ? 3u : 2u;
    } else {
        v->Prism = (fabsUalpha >= ratio) ? 6u : 5u;
    }
}

/* Clamp duties exactly like original version */
static inline float clamp_duty_1(float d)
{
    if (d < DUTY_MIN_F) return DUTY_MIN_F;
    if (d > DUTY_MAX_F) return DUTY_MAX_F;
    return d;
}

static inline void SVM3D_DQZ_GEN_CLA_COMPUTE_2(SVM3D_DQZ_GEN_CLA_2 *v)
{
    if (v->Udc == 0.0f) {
        /* Avoid division by zero: set outputs to zero */
        v->da = v->db = v->dc = v->dn = 0.0f;
        v->d1 = v->d2 = v->d3 = v->d0 = 0.0f;
        return;
    }

    v->inver_Udc = HALF_F / v->Udc;
    calc_uvw_1(v);
    select_prism_1(v);

    /* Local copies for readability (same as original) */
    float Ua = v->Ua, Ub = v->Ub, Uc = v->Uc;
    float alpha = v->Ualpha, beta = v->Ubeta, gamma = v->Ugamma;
    float iudc = v->inver_Udc;

    /* Temporaries */
    float t1 = 0.0f, t2 = 0.0f, t3 = 0.0f;

    switch (v->Prism) {
        case 0:
            v->da = v->db = v->dc = v->dn = 0.0f;
            v->d1 = v->d2 = v->d3 = v->d0 = 0.0f;
            return;

        case 1:
            if ((Ua >= 0.0f) && (Ub >= 0.0f)) {
                if (Uc >= 0.0f) {
                    /* Prism 1 Tetra 3 */
                    t1 = (INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                    t2 = (SQRT3_F * beta) * iudc;
                    t3 = (-HALF_F * alpha - SQRT3_2_F * beta + gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t1 + t2 + t3;
                    v->db = 0.5f * v->d0 + t2 + t3;
                    v->dc = 0.5f * v->d0 + t3;
                    v->dn = 0.5f * v->d0;
                } else {
                    /* Prism 1 Tetra 2 */
                    t1 = (INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                    t2 = (-HALF_F * alpha + SQRT3_2_F * beta + gamma) * iudc;
                    t3 = (HALF_F * alpha + SQRT3_2_F * beta - gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t1 + t2 + t3;
                    v->db = 0.5f * v->d0 + t2 + t3;
                    v->dc = 0.5f * v->d0;
                    v->dn = 0.5f * v->d0 + t3;
                }
            } else if ((Ua >= 0.0f) && (Ub < 0.0f) && (Uc <= 0.0f)) {
                /* Prism 1 Tetra 1 */
                t1 = (alpha + gamma) * iudc;
                t2 = (HALF_F * alpha - SQRT3_2_F * beta - gamma) * iudc;
                t3 = (SQRT3_F * beta) * iudc;
                v->d0 = 1.0f - t1 - t2 - t3;
                v->da = 0.5f * v->d0 + t1 + t2 + t3;
                v->db = 0.5f * v->d0 + t3;
                v->dc = 0.5f * v->d0;
                v->dn = 0.5f * v->d0 + t2 + t3;
            } else if ((Ua < 0.0f) && (Ub <= 0.0f) && (Uc <= 0.0f)) {
                /* Prism 1 Tetra 4 */
                t1 = (INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                t2 = (SQRT3_F * beta) * iudc;
                t3 = (-alpha - gamma) * iudc;
                v->d0 = 1.0f - t1 - t2 - t3;
                v->da = 0.5f * v->d0 + t1 + t2;
                v->db = 0.5f * v->d0 + t2;
                v->dc = 0.5f * v->d0;
                v->dn = 0.5f * v->d0 + t1 + t2 + t3;
            }
            break;

        case 2:
            if ((Ua >= 0.0f) && (Ub >= 0.0f)) {
                if (Uc >= 0.0f) {
                    /* Prism 2 Tetra 3 */
                    t1 = (INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    t2 = (-INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    t3 = (-HALF_F * alpha - SQRT3_2_F * beta + gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t1 + t3;
                    v->db = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dc = 0.5f * v->d0 + t3;
                    v->dn = 0.5f * v->d0;
                } else {
                    /* Prism 2 Tetra 1 */
                    t1 = (alpha + gamma) * iudc;
                    t2 = (HALF_F * alpha + SQRT3_2_F * beta - gamma) * iudc;
                    t3 = (-INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t1 + t2;
                    v->db = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dc = 0.5f * v->d0;
                    v->dn = 0.5f * v->d0 + t2;
                }
            } else if ((Ua < 0.0f) && (Uc <= 0.0f)) {
                if (Ub >= 0.0f) {
                    /* Prism 2 Tetra 2 */
                    t1 = (INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    t2 = (-HALF_F * alpha + SQRT3_2_F * beta + gamma) * iudc;
                    t3 = (-alpha - gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t1;
                    v->db = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dc = 0.5f * v->d0;
                    v->dn = 0.5f * v->d0 + t1 + t3;
                } else {
                    /* Prism 2 Tetra 4 */
                    t1 = (INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    t2 = (-INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    t3 = (HALF_F * alpha - SQRT3_2_F * beta - gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t1;
                    v->db = 0.5f * v->d0 + t1 + t2;
                    v->dc = 0.5f * v->d0;
                    v->dn = 0.5f * v->d0 + t1 + t2 + t3;
                }
            }
            break;

        case 3:
            if ((Ub >= 0.0f) && (Uc >= 0.0f)) {
                if (Ua >= 0.0f) {
                    /* Prism 3 Tetra 3 */
                    t1 = (SQRT3_F * beta) * iudc;
                    t2 = (-INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                    t3 = (alpha + gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t3;
                    v->db = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dc = 0.5f * v->d0 + t2 + t3;
                    v->dn = 0.5f * v->d0;
                } else {
                    /* Prism 3 Tetra 2 */
                    t1 = (SQRT3_F * beta) * iudc;
                    t2 = (-HALF_F * alpha - SQRT3_2_F * beta + gamma) * iudc;
                    t3 = (-alpha - gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0;
                    v->db = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dc = 0.5f * v->d0 + t2 + t3;
                    v->dn = 0.5f * v->d0 + t3;
                }
            } else if ((Ua < 0.0f) && (Ub >= 0.0f) && (Uc < 0.0f)) {
                /* Prism 3 Tetra 1 */
                t1 = (-HALF_F * alpha + SQRT3_2_F * beta + gamma) * iudc;
                t2 = (HALF_F * alpha + SQRT3_2_F * beta - gamma) * iudc;
                t3 = (-INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                v->d0 = 1.0f - t1 - t2 - t3;
                v->da = 0.5f * v->d0;
                v->db = 0.5f * v->d0 + t1 + t2 + t3;
                v->dc = 0.5f * v->d0 + t3;
                v->dn = 0.5f * v->d0 + t2 + t3;
            } else if ((Ua < 0.0f) && (Ub < 0.0f) && (Uc <= 0.0f)) {
                /* Prism 3 Tetra 4 */
                t1 = (SQRT3_F * beta) * iudc;
                t2 = (-INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                t3 = (-HALF_F * alpha + SQRT3_2_F * beta + gamma) * iudc;
                v->d0 = 1.0f - t1 - t2 - t3;
                v->da = 0.5f * v->d0;
                v->db = 0.5f * v->d0 + t1 + t2;
                v->dc = 0.5f * v->d0 + t2;
                v->dn = 0.5f * v->d0 + t1 + t2 + t3;
            }
            break;

        case 4:
            if ((Ub >= 0.0f) && (Uc >= 0.0f)) {
                if (Ua >= 0.0f) {
                    /* Prism 4 Tetra 3 */
                    t1 = (-INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    t2 = (-SQRT3_F * beta) * iudc;
                    t3 = (alpha + gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t3;
                    v->db = 0.5f * v->d0 + t1 + t3;
                    v->dc = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dn = 0.5f * v->d0;
                } else {
                    /* Prism 4 Tetra 1 */
                    t1 = (-HALF_F * alpha + SQRT3_2_F * beta + gamma) * iudc;
                    t2 = (-alpha - gamma) * iudc;
                    t3 = (-SQRT3_F * beta) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0;
                    v->db = 0.5f * v->d0 + t1 + t2;
                    v->dc = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dn = 0.5f * v->d0 + t2;
                }
            } else if ((Ua < 0.0f) && (Ub < 0.0f)) {
                if (Uc >= 0.0f) {
                    /* Prism 4 Tetra 2 */
                    t1 = (-INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    t2 = (-HALF_F * alpha - SQRT3_2_F * beta + gamma) * iudc;
                    t3 = (HALF_F * alpha - SQRT3_2_F * beta - gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0;
                    v->db = 0.5f * v->d0 + t1;
                    v->dc = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dn = 0.5f * v->d0 + t1 + t3;
                } else {
                    /* Prism 4 Tetra 4 */
                    t1 = (-INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    t2 = (-SQRT3_F * beta) * iudc;
                    t3 = (HALF_F * alpha + SQRT3_2_F * beta - gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0;
                    v->db = 0.5f * v->d0 + t1;
                    v->dc = 0.5f * v->d0 + t1 + t2;
                    v->dn = 0.5f * v->d0 + t1 + t2 + t3;
                }
            }
            break;

        case 5:
            if ((Ua >= 0.0f) && (Uc >= 0.0f)) {
                if (Ub >= 0.0f) {
                    /* Prism 5 Tetra 3 */
                    t1 = (-INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                    t2 = (INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                    t3 = (-HALF_F * alpha + SQRT3_2_F * beta + gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t2 + t3;
                    v->db = 0.5f * v->d0 + t3;
                    v->dc = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dn = 0.5f * v->d0;
                } else {
                    /* Prism 5 Tetra 2 */
                    t1 = (-INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                    t2 = (alpha + gamma) * iudc;
                    t3 = (HALF_F * alpha - SQRT3_2_F * beta - gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t2 + t3;
                    v->db = 0.5f * v->d0;
                    v->dc = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dn = 0.5f * v->d0 + t3;
                }
            } else if ((Ua < 0.0f) && (Ub <= 0.0f)) {
                if (Uc >= 0.0f) {
                    /* Prism 5 Tetra 1 */
                    t1 = (-HALF_F * alpha - SQRT3_2_F * beta + gamma) * iudc;
                    t2 = (-alpha - gamma) * iudc;
                    t3 = (INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t3;
                    v->db = 0.5f * v->d0;
                    v->dc = 0.5f * v->d0 + t1 + t2 + t3;
                    v->dn = 0.5f * v->d0 + t2 + t3;
                } else {
                    /* Prism 5 Tetra 4 */
                    t1 = (-INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                    t2 = (INV3_2_F * alpha - SQRT3_2_F * beta) * iudc;
                    t3 = (HALF_F * alpha + SQRT3_2_F * beta - gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t2;
                    v->db = 0.5f * v->d0;
                    v->dc = 0.5f * v->d0 + t1 + t2;
                    v->dn = 0.5f * v->d0 + t1 + t2 + t3;
                }
            }
            break;

        case 6:
            if ((Ua >= 0.0f) && (Uc >= 0.0f)) {
                if (Ub >= 0.0f) {
                    /* Prism 6 Tetra 3 */
                    t1 = (-SQRT3_F * beta) * iudc;
                    t2 = (INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    t3 = (-HALF_F * alpha + SQRT3_2_F * beta + gamma) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t1 + t2 + t3;
                    v->db = 0.5f * v->d0 + t3;
                    v->dc = 0.5f * v->d0 + t1 + t3;
                    v->dn = 0.5f * v->d0;
                } else {
                    /* Prism 6 Tetra 1 */
                    t1 = (-HALF_F * alpha - SQRT3_2_F * beta + gamma) * iudc;
                    t2 = (HALF_F * alpha - SQRT3_2_F * beta - gamma) * iudc;
                    t3 = (INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                    v->d0 = 1.0f - t1 - t2 - t3;
                    v->da = 0.5f * v->d0 + t1 + t2 + t3;
                    v->db = 0.5f * v->d0;
                    v->dc = 0.5f * v->d0 + t1 + t2;
                    v->dn = 0.5f * v->d0 + t2;
                }
            } else if ((Ua >= 0.0f) && (Ub < 0.0f) && (Uc < 0.0f)) {
                /* Prism 6 Tetra 2 */
                t1 = (-SQRT3_F * beta) * iudc;
                t2 = (alpha + gamma) * iudc;
                t3 = (HALF_F * alpha + SQRT3_2_F * beta - gamma) * iudc;
                v->d0 = 1.0f - t1 - t2 - t3;
                v->da = 0.5f * v->d0 + t1 + t2 + t3;
                v->db = 0.5f * v->d0;
                v->dc = 0.5f * v->d0 + t1;
                v->dn = 0.5f * v->d0 + t1 + t3;
            } else if ((Ua < 0.0f) && (Ub <= 0.0f) && (Uc <= 0.0f)) {
                /* Prism 6 Tetra 4 */
                t1 = (-SQRT3_F * beta) * iudc;
                t2 = (INV3_2_F * alpha + SQRT3_2_F * beta) * iudc;
                t3 = (-alpha - gamma) * iudc;
                v->d0 = 1.0f - t1 - t2 - t3;
                v->da = 0.5f * v->d0 + t1 + t2;
                v->db = 0.5f * v->d0;
                v->dc = 0.5f * v->d0 + t1;
                v->dn = 0.5f * v->d0 + t1 + t2 + t3;
            }
            break;

        default:
            /* Unexpected prism -> zero outputs */
            v->da = v->db = v->dc = v->dn = 0.0f;
            v->d1 = v->d2 = v->d3 = v->d0 = 0.0f;
            return;
    }

    v->d1 = v->d1; v->d2 = v->d2; v->d3 = v->d3; /* No-op but preserves fields */

    if (v->da < 0.0f) v->da = 0.0f; else if (v->da > DUTY_MAX_F) v->da = DUTY_MAX_F;
    if (v->db < 0.0f) v->db = 0.0f; else if (v->db > DUTY_MAX_F) v->db = DUTY_MAX_F;
    if (v->dc < 0.0f) v->dc = 0.0f; else if (v->dc > DUTY_MAX_F) v->dc = DUTY_MAX_F;
    if (v->dn < 0.0f) v->dn = 0.0f; else if (v->dn > DUTY_MAX_F) v->dn = DUTY_MAX_F;
}

static inline void SVM3D_DQZ_GEN_CLA_RUN(SVM3D_DQZ_GEN_CLA_2 *v)
{
    SVM3D_DQZ_GEN_CLA_COMPUTE_2(v);
}

#endif /* _SVM3D_DQZ_CLA_TOI_UU2_H_ */
