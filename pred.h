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

#ifndef _PRED_H
#define _PRED_H 1

#include "phonstats.h"
#include "measures.h"
#include "mdata.h"

double joint_p(struct phonstats *ps, char *x, char *y);
double cond_p(struct phonstats *ps, char *x, char *y);
double pmi(struct phonstats *ps, char *x, char *y);
double cond_entropy(struct phonstats *ps, char *x, int y_len);
size_t sv(struct phonstats *ps, char *x, int y_len);

double cond_p_r(struct phonstats *ps, char *x, char *y);
double cond_entropy_r(struct phonstats *ps, char *x, int y_len);
size_t sv_r(struct phonstats *ps, char *x, int y_len);

double npmi(struct phonstats *ps, char *x, char *y);

double calc_pred_single(struct phonstats *ps, struct mdata *m, int len);
double *calc_pred_list(struct phonstats *ps, struct mdata *m);

void add_bow_eow(char *dest, const char *src);
inline void char_swap(char *s, int len, int pos);
inline void char_unswap(char *s, int len, int pos);
double pred_calc(struct phonstats *ps, enum m_id m, char *x, char *y,
                              int x_len, int y_len);

int pred_init(struct mdlist *mdl, struct phonstats *ps);


#endif // _PRED_H
