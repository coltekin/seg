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

#ifndef _UB_H
#define _UB_H 1

#include "phonstats.h"
#include "measures.h"
#include "mdata.h"

int ub_init(struct mdlist *mdl, struct phonstats *ps, enum m_id ub_id, enum m_id ue_id); 
double calc_ub_single(struct phonstats *ps, struct mdata *m, int pos);
double *calc_ub_list(struct phonstats *ps, struct mdata *m);

double calc_ue_single(struct phonstats *ps, struct mdata *m, int pos);
double *calc_ue_list(struct phonstats *ps, struct mdata *m);

#endif // _UB_H
