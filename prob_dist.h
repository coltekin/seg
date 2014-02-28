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

#ifndef _PROB_DIST_H
#define _PROB_DIST_H 1

#include <math.h>

/* 
 * This structure is useful for probability distributions on real
 * numbers that can be parametrized with mean and variance.
 * The main motivation is on-line update. As a result we keep mean,
 * number of data points and sum of square of diffrence from mean.
 * See * http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 * for details of online algorithms for calcualting mean and variance
 */
struct prob_dist {
    double mean;
    double delta2;
    double N;       // integer could do, but this avoids type conversion on
};                  // every calculation

#define mean(pd) (pd->mean)
#define var(pd) (pd->delta2/pd->N)      // sample variance
#define sd2(pd) (pd->delta2/(pd->N-1))  // (estimated) population variance
#define sd(pd) (sqrt(var(pd)))          // sample sd
#define z_score(pd,x) ((x-mean(pd))/sd(pd))

struct prob_dist * prob_dist_new();
struct prob_dist * prob_dist_new_full(double mean, double var, double N);
void prob_dist_free(struct prob_dist *dist);
void prob_dist_update(struct prob_dist *dist, double x);
void prob_dist_remove(struct prob_dist *dist, double x);
void prob_dist_replace(struct prob_dist *dist, double old, double new);

double prob_dist_norm(struct prob_dist *dist, double val);


#endif // _PROB_DIST_H

