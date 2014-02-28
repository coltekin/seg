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

#include <string.h>
#include <assert.h>
#include <malloc.h>
#include "ub.h"
#include "mdata.h"
#include "measures.h"


int 
stress_init(struct mdlist *mdl, struct phonstats *ps, enum m_id ub_id, enum m_id ue_id)
{
    int nvotes;
    switch (opt.stress_arg) {
        case stress_arg_transition:
            nvotes = 0;
        break;
        case stress_arg_cheat:
            nvotes = 0;
        break;
        case stress_arg_sylcheat:
            nvotes = 0;
        break;
        case stress_arg_ub:
        default:
             nvotes = stress_init(mdl, ps, M_SUB, M_SUE);
        break;
    }

    return nvotes;
}

static inline double 
_calc_stress_single(struct phonstats *ps, struct mdata *m, int pos, int len)
{
    return 0.0;
}
    

double 
calc_stress_single(struct phonstats *ps, struct mdata *m, int pos)
{
    return 0.0;
}

double *
calc_stress_list(struct phonstats *ps, struct mdata *m)
{
    return NULL;
}

