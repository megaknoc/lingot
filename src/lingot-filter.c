/*
 * lingot, a musical instrument tuner.
 *
 * Copyright (C) 2004-2020  Iban Cereijo.
 * Copyright (C) 2004-2008  Jairo Chapela.

 *
 * This file is part of lingot.
 *
 * lingot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * lingot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with lingot; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <memory.h>
#include <math.h>

#include "lingot-complex.h"
#include "lingot-filter.h"

#define max(a,b) (((a)<(b))?(b):(a))

// given each polynomial order and coefs, with optional initial status.
void lingot_filter_new(lingot_filter_t* filter,
                       unsigned int Na, unsigned int Nb,
                       const LINGOT_FLT* a, const LINGOT_FLT* b) {
    unsigned int i;
    filter->N = max(Na, Nb);

    filter->a = malloc((filter->N + 1) * sizeof(LINGOT_FLT));
    filter->b = malloc((filter->N + 1) * sizeof(LINGOT_FLT));
    filter->s = malloc((filter->N + 1) * sizeof(LINGOT_FLT));

    for (i = 0; i <= filter->N; i++) {
        filter->a[i] = 0.0;
        filter->b[i] = 0.0;
        filter->s[i] = 0.0;
    }

    memcpy(filter->a, a, (Na + 1) * sizeof(LINGOT_FLT));
    memcpy(filter->b, b, (Nb + 1) * sizeof(LINGOT_FLT));

    for (i = 0; i <= filter->N; i++) {
        filter->a[i] /= a[0]; // polynomial normalization.
        filter->b[i] /= a[0];
    }
}

void lingot_filter_reset(lingot_filter_t* filter) {
    unsigned int i;
    for (i = 0; i <= filter->N; i++) {
        filter->s[i] = 0.0;
    }
}

void lingot_filter_destroy(lingot_filter_t* filter) {
    free(filter->a);
    free(filter->b);
    free(filter->s);
    filter->a = NULL;
    filter->b = NULL;
    filter->s = NULL;
}

// Digital Filter Implementation II, in & out can overlap.
void lingot_filter_filter(lingot_filter_t* filter, unsigned int n,
                          const LINGOT_FLT* in, LINGOT_FLT* out) {
    LINGOT_FLT w, y;
    unsigned int i;
    int j;

    for (i = 0; i < n; i++) {

        w = in[i];
        y = 0.0;

        for (j = (int) (filter->N - 1); j >= 0; j--) {
            w -= filter->a[j + 1] * filter->s[j];
            y += filter->b[j + 1] * filter->s[j];
            filter->s[j + 1] = filter->s[j];
        }

        y += w * filter->b[0];
        filter->s[0] = w;

        out[i] = y;
    }
}

// single sample filtering
LINGOT_FLT lingot_filter_filter_sample(lingot_filter_t* filter, LINGOT_FLT in) {
    LINGOT_FLT result;

    lingot_filter_filter(filter, 1, &in, &result);
    return result;
}

// vector prod
void lingot_filter_vector_product(unsigned int n, lingot_complex_t* vector, lingot_complex_t result) {
    unsigned int i;
    lingot_complex_t aux1;

    result[0] = 1.0;
    result[1] = 0.0;

    for (i = 0; i < n; i++) {
        aux1[0] = -vector[i][0];
        aux1[1] = -vector[i][1];
        lingot_complex_mul_by(result, aux1);
    }

}

// Chebyshev filters
void lingot_filter_cheby_design(lingot_filter_t* filter, unsigned int n, LINGOT_FLT Rp, LINGOT_FLT wc) {
    unsigned int i; // loops
    unsigned int k;
    unsigned int p;

    LINGOT_FLT a[n + 1];
    LINGOT_FLT b[n + 1];

    LINGOT_FLT new_a[n + 1];
    LINGOT_FLT new_b[n + 1];

    // locate poles
    lingot_complex_t pole[n];

    for (i = 0; i < n; i++) {
        pole[i][0] = 0.0;
        pole[i][1] = 0.0;
    }

    LINGOT_FLT T = 2.0;
    // 2Hz
    LINGOT_FLT W = 2.0 / T * tan(M_PI * wc / T);

    LINGOT_FLT epsilon = sqrt(pow(10.0, 0.1 * Rp) - 1);
    LINGOT_FLT v0 = asinh(1 / epsilon) / n;

    LINGOT_FLT sv0 = sinh(v0);
    LINGOT_FLT cv0 = cosh(v0);

    LINGOT_FLT t;

    for (i = -(n - 1), k = 0; k < n; i += 2, k++) {
        t = M_PI * i / (2.0 * n);
        pole[k][0] = -sv0 * cos(t);
        pole[k][1] = cv0 * sin(t);
    }

    lingot_complex_t gain;
    lingot_filter_vector_product(n, pole, gain);

    if ((n & 1) == 0) { // even
        LINGOT_FLT f = pow(10.0, -0.05 * Rp);
        gain[0] *= f;
        gain[1] *= f;
    }

    LINGOT_FLT f = pow(W, n);
    gain[0] *= f;
    gain[1] *= f;

    for (i = 0; i < n; i++) {
        pole[i][0] *= W;
        pole[i][1] *= W;
    }

    // bilinear transform
    lingot_complex_t sp[n];

    for (i = 0; i < n; i++) {
        sp[i][0] = (2.0 - pole[i][0] * T) / T;
        sp[i][1] = (0.0 - pole[i][1] * T) / T;
    }

    lingot_complex_t tmp1;
    lingot_complex_t aux2;

    lingot_filter_vector_product(n, sp, tmp1);

    lingot_complex_div_by(gain, tmp1);

    for (i = 0; i < n; i++) {
        tmp1[0] = (2.0 + pole[i][0] * T);
        tmp1[1] = (0.0 + pole[i][1] * T);
        aux2[0] = (2.0 - pole[i][0] * T);
        aux2[1] = (0.0 - pole[i][1] * T);
        lingot_complex_div(tmp1, aux2, pole[i]);
    }

    // compute filter coefficients from pole/zero values
    a[0] = 1.0;
    b[0] = 1.0;
    new_a[0] = 1.0;
    new_b[0] = 1.0;

    for (i = 1; i <= n; i++) {
        a[i] = 0.0;
        b[i] = 0.0;
        new_a[i] = 0.0;
        new_b[i] = 0.0;
    }

    if (n & 1) {  // odd
        // first subfilter is first order
        a[1] = -pole[n / 2][0];
        b[1] = 1.0;
    }

    // iterate over the conjugate pairs
    for (p = 0; p < n / 2; p++) {
        LINGOT_FLT b1 = 2.0;
        LINGOT_FLT b2 = 1.0;

        LINGOT_FLT a1 = -2.0 * pole[p][0];
        LINGOT_FLT a2 = pole[p][0] * pole[p][0] + pole[p][1] * pole[p][1];

        // 2nd order subfilter per each pair
        new_a[1] = a[1] + a1 * a[0];
        new_b[1] = b[1] + b1 * b[0];

        // poly multiplication
        for (i = 2; i <= n; i++) {
            new_a[i] = a[i] + a1 * a[i - 1] + a2 * a[i - 2];
            new_b[i] = b[i] + b1 * b[i - 1] + b2 * b[i - 2];
        }
        for (i = 1; i <= n; i++) {
            a[i] = new_a[i];
            b[i] = new_b[i];
        }
    }

    gain[0] = fabs(gain[0]);
    for (i = 0; i <= n; i++) {
        b[i] *= gain[0];
    }

    lingot_filter_new(filter, n, n, a, b);
}
