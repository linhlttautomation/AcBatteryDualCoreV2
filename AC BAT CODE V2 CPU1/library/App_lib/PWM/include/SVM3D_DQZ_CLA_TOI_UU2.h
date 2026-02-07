/*
 * SVM3D_DQZ_CLA_TOI_UU2.h
 *
*  Created on: Jun 27, 2024
*  Author: Luu Linh K65
*  Toi uu SVM-3D Ver2
 */

#ifndef _SVM3D_DQZ_CLA_TOI_UU2_H_
#define _SVM3D_DQZ_CLA_TOI_UU2_H_

typedef struct {
    Uint16 Prism;
    Uint16 Tetra;
    float Ualpha, Ubeta, Ugamma, Udc;
    float da, db, dc, dn;
    float Ua, Ub, Uc;
    float d1, d2, d3, d0;
    float inver_Udc;
} SVM3D_DQZ_GEN_CLA_2;

#define CLAMP01(x) (((x) < 0.0f) ? 0.0f : (((x) > 0.98f) ? 0.98f : (x)))
#define SVM3D_DQZ_GEN_CLA_INT_2(v) do { memset(&(v), 0, sizeof(SVM3D_DQZ_GEN_CLA_2)); } while(0)

static inline void svm3d_inverse_clarke(volatile SVM3D_DQZ_GEN_CLA_2 *v) {
    float sqrt3_half = 0.8660254037844386f;
    v->Ua = v->Ualpha + v->Ugamma;
    v->Ub = -0.5f * v->Ualpha + sqrt3_half * v->Ubeta + v->Ugamma;
    v->Uc = -0.5f * v->Ualpha - sqrt3_half * v->Ubeta + v->Ugamma;
    v->inver_Udc = 0.5f / v->Udc;
}

static inline void svm3d_compute_prism(volatile SVM3D_DQZ_GEN_CLA_2 *v) {
    float sqrt3 = 1.7320508f;
    float absUa = fabsf(v->Ualpha);
    float absUb = fabsf(v->Ubeta);
    float ratio = absUb / sqrt3;

    int cond1 = (v->Ualpha * v->Ubeta >= 0);
    int cond2 = (v->Ubeta >= 0);

    if (cond1 && cond2)
        v->Prism = (absUa >= ratio) ? 1 : 2;
    else if (cond1 && !cond2)
        v->Prism = (absUa >= ratio) ? 4 : 5;
    else if (!cond1 && cond2)
        v->Prism = (absUa >= ratio) ? 3 : 2;
    else
        v->Prism = (absUa >= ratio) ? 6 : 5;
}

static inline void svm3d_compute_tetra(volatile SVM3D_DQZ_GEN_CLA_2 *v) {
    int sa = (v->Ua > 0.0f) ? 1 : 0;
    int sb = (v->Ub > 0.0f) ? 1 : 0;
    int sc = (v->Uc > 0.0f) ? 1 : 0;

    v->Tetra = sa * 4 + sb * 2 + sc;
    if (v->Tetra >= 8) v->Tetra = 7;
    static const Uint16 tetra_map[8] = {0, 1, 2, 0, 3, 1, 2, 3};
    v->Tetra = tetra_map[v->Tetra];
}

typedef struct {
    float m1[3];
    float m2[3];
    float m3[3];
    Uint16 output_pattern;
} SVM3D_TetraCoeff;

static const SVM3D_TetraCoeff tetra_coeffs[24] = {
    // Prism 1
    {{ 1, 0, 1}, {0.5f, -0.8660254f, -1}, {0, 1.7320508f, 0}, 0},
    {{ 0.5f, 0.8660254f, 1}, {1, 0, -1}, {-0.5f, 0.8660254f, 0}, 1},
    {{ 0.5f, 0.8660254f, 1}, {0, -1.7320508f, -1}, {-0.5f, 0.8660254f, 0}, 2},
    {{ 1, 0, 1}, {0, -1.7320508f, -1}, {0.5f, -0.8660254f, -1}, 3},

    // Prism 2
    {{ 0.5f, 0.8660254f, 1}, {-1, 0, -1}, {0.5f, -0.8660254f, 0}, 0},
    {{ -0.5f, 0.8660254f, 1}, {0.5f, 0.8660254f, -1}, {0.5f, -0.8660254f, 0}, 1},
    {{ -0.5f, 0.8660254f, 1}, {1, 0, -1}, {0.5f, 0.8660254f, 0}, 2},
    {{ 0.5f, 0.8660254f, 1}, {1, 0, -1}, {-1, 0, -1}, 3},

    // Prism 3
    {{ -0.5f, 0.8660254f, 1}, {-0.5f, -0.8660254f, -1}, {1, 0, 0}, 0},
    {{ -1, 0, 1}, {0.5f, -0.8660254f, -1}, {0.5f, 0.8660254f, 0}, 1},
    {{ -1, 0, 1}, {0.5f, -0.8660254f, -1}, {-0.5f, -0.8660254f, 0}, 2},
    {{ -0.5f, 0.8660254f, 1}, {-0.5f, -0.8660254f, -1}, {-1, 0, 0}, 3},

    // Prism 4
    {{ -1, 0, 1}, {-0.5f, 0.8660254f, -1}, {0.5f, -0.8660254f, 0}, 0},
    {{ -0.5f, -0.8660254f, 1}, {-1, 0, -1}, {0.5f, -0.8660254f, 0}, 1},
    {{ -0.5f, -0.8660254f, 1}, {0.5f, 0.8660254f, -1}, {-1, 0, 0}, 2},
    {{ -1, 0, 1}, {0.5f, 0.8660254f, -1}, {-0.5f, -0.8660254f, 0}, 3},

    // Prism 5
    {{ -0.5f, -0.8660254f, 1}, {1, 0, -1}, {-0.5f, 0.8660254f, 0}, 0},
    {{ 0.5f, -0.8660254f, 1}, {-0.5f, -0.8660254f, -1}, {-0.5f, 0.8660254f, 0}, 1},
    {{ 0.5f, -0.8660254f, 1}, {-1, 0, -1}, {0.5f, -0.8660254f, 0}, 2},
    {{ -0.5f, -0.8660254f, 1}, {-1, 0, -1}, {1, 0, 0}, 3},

    // Prism 6
    {{ 0.5f, -0.8660254f, 1}, {0.5f, 0.8660254f, -1}, {-1, 0, 0}, 0},
    {{ 1, 0, 1}, {-0.5f, 0.8660254f, -1}, {-0.5f, -0.8660254f, 0}, 1},
    {{ 1, 0, 1}, {-0.5f, 0.8660254f, -1}, {0.5f, 0.8660254f, 0}, 2},
    {{ 0.5f, -0.8660254f, 1}, {0.5f, 0.8660254f, -1}, {1, 0, 0}, 3}
};

static inline void svm3d_compute_duty(volatile SVM3D_DQZ_GEN_CLA_2 *v) {
    Uint16 sector = (v->Prism - 1) * 4 + v->Tetra;
    const SVM3D_TetraCoeff *coeff = &tetra_coeffs[sector];

    v->d1 = (coeff->m1[0]*v->Ualpha + coeff->m1[1]*v->Ubeta + coeff->m1[2]*v->Ugamma) * v->inver_Udc;
    v->d2 = (coeff->m2[0]*v->Ualpha + coeff->m2[1]*v->Ubeta + coeff->m2[2]*v->Ugamma) * v->inver_Udc;
    v->d3 = (coeff->m3[0]*v->Ualpha + coeff->m3[1]*v->Ubeta + coeff->m3[2]*v->Ugamma) * v->inver_Udc;
    v->d0 = 1.0f - v->d1 - v->d2 - v->d3;

    v->da = v->d1 + 0.5f * v->d0;
    v->db = v->d2 + 0.5f * v->d0;
    v->dc = v->d3 + 0.5f * v->d0;
    v->dn = 0.0f;

    v->da = CLAMP01(v->da);
    v->db = CLAMP01(v->db);
    v->dc = CLAMP01(v->dc);
}

static inline void SVM3D_DQZ_GEN_CLA_RUN(volatile SVM3D_DQZ_GEN_CLA_2 *v) {
    svm3d_inverse_clarke(v);
    svm3d_compute_prism(v);
    svm3d_compute_tetra(v);
    svm3d_compute_duty(v);
}

#endif /* _SVM3D_DQZ_CLA_TOI_UU2_H_ */
