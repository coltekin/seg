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

#include <assert.h>
#include "pub.h"
#include "measures.h"

int 
pub_init(struct mdlist *mdl, int *ngmax)
{
    int ub = 0, ue = 0;
    int ub_votec = 0;
    int lmin = 0, rmin = 0, lmax = 0, rmax = 0; 
    int li, ri;

    lmin = opt.ub_lmin_arg;
    lmax = opt.ub_lmax_arg;
    rmin = opt.ub_rmin_arg;
    rmax = opt.ub_rmax_arg;

    if (opt.ub_ngmax_given) {
        assert (!opt.ub_lmax_given && !opt.ub_rmax_given);
        rmax = lmax = opt.ub_ngmax_arg;
    }
    if (opt.ub_ngmin_given) {
        assert (!opt.ub_lmin_given && !opt.ub_rmin_given);
        rmin = lmin = opt.ub_ngmin_arg;
    }
    if (opt.ub_nglen_given) {
        assert (!opt.ub_lmin_given && !opt.ub_rmin_given);
        assert (!opt.ub_lmax_given && !opt.ub_rmax_given);
        assert (!opt.ub_ngmin_given && !opt.ub_ngmax_given);
        rmin = lmin = rmax = lmax = opt.ub_nglen_arg;
    }
    
    assert(lmax >= lmin && rmax >= rmin);


    if (opt.ub_type_arg == ub_type_arg_both) {
        ub_votec = 2 + (rmax - rmin + lmax - lmin);
        ub = ue = 1;
    } else {
        if (opt.ub_type_arg == ub_type_arg_ubegin) {
            ub_votec = 1 + (rmax - rmin);
            ub = 1;
        } else if (opt.ub_type_arg == ub_type_arg_uend) {
            ub_votec = 1 + (lmax - lmin);
            ue = 1;
        }
    }

    for (li = lmin; ue && li <= lmax; li++) {
        struct mdata *md = mdata_new_full(M_PUE, NULL, li, -1);
        mdlist_add(mdl, md);
    }
    for (ri = rmin; ub && ri <= rmax; ri++) {
        struct mdata *md = mdata_new_full(M_PUB, NULL, -1, ri);
        mdlist_add(mdl, md);
    }
    if (lmax > *ngmax) *ngmax = lmax;
    if (rmax > *ngmax) *ngmax = rmax;

    return ub_votec;
}
