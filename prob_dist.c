/*  
    Copyright 2010-2014 Çağrı Çöltekin <c.coltekin@rug.nl>

    This file is part of seg, an application for word segmentation.

    seg is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program as `gpl.txt'. If not, see 
    <http://www.gnu.org/licenses/>.
*/

/*
 * Some basic functions for handling probability distributions.
 * See prob_dist.h for details .
 */

#include <malloc.h>
#include <float.h>
#include <assert.h>
#include "prob_dist.h"

struct prob_dist * 
prob_dist_new()
{
    struct prob_dist *ret = malloc (sizeof *ret);
    ret->mean = 0.0;
    ret->delta2 = 0.0;
    ret->N = DBL_EPSILON;
//   ret->N = 0.0;
    return ret;
}

struct prob_dist * 
prob_dist_new_full(double mean, double var, double N)
{
    struct prob_dist *ret = malloc (sizeof *ret);
    ret->mean = mean;
    ret->delta2 = var;
    ret->N = DBL_EPSILON;
//   ret->N = 0.0;
    return ret;
}

void 
prob_dist_free(struct prob_dist *dist)
{
    free(dist);
}

void 
prob_dist_update(struct prob_dist *dist, double x)
{
    double delta;
    dist->N += 1.0;
    delta = x - dist->mean;
    dist->mean += delta / dist->N;
    dist->delta2 += delta * (x - dist->mean);
}

void 
prob_dist_remove(struct prob_dist *dist, double x)
{
    double delta = x - dist->mean;
    dist->mean = (dist->N * dist->mean - x) / (dist->N - 1);
    dist->delta2 -= delta * (x - dist->mean);
    dist->N -= 1.0;
}

// TODO: this can be done more efficiently, and possibly 
//       with better numerical stability
void 
prob_dist_replace(struct prob_dist *dist, double old, double new)
{
    prob_dist_remove(dist, old);
    prob_dist_update(dist, new);
}

double
prob_dist_norm(struct prob_dist *dist, double val)
{
    assert(dist != NULL);
    return (val - mean(dist)) / sd(dist);
}

#ifdef _PROB_DIST_TEST_

main()
{
    struct prob_dist *d = prob_dist_new();
    double x[] = {10,15,23,12,3};
    int i;

    for (i = 0; i < 5; i++) {
        prob_dist_update(d, x[i]);
        printf("%d: m = %f, sd = %f\n", i, mean(d), sqrt(sd2(d)));
    }
}
#endif
