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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <gsl/gsl_randist.h>
#include "seg.h"
#include "seg_ub.h"
#include "phonstats.h"
#include "predictability.h"
#include "mvote.h"
#include "mlist.h"
#include "measures.h"
#include "mdata.h"

static struct phonstats *ps = NULL;
static int  lmin = 0; 
static int  lmax = 0; 
static int  rmin = 0; 
static int  rmax = 0; 
static int  votec = 0; 
static struct mdata *md = NULL;


void 
segment_ub_init(struct input *in)
{
    int ub = 0, ue = 0;
    int i = 0, li = 0, ri = 0;
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

    if (opt.ub_type_arg == ub_type_arg_both) {
        votec = 2 + (rmax - rmin + lmax - lmin);
        ub = ue = 1;
    } else {
        if (opt.ub_type_arg == ub_type_arg_ubegin) {
            votec = 1 + (rmax - rmin);
            ub = 1;
        } else if (opt.ub_type_arg == ub_type_arg_uend) {
            votec = 1 + (lmax - lmin);
            ue = 1;
        }
    }

    md = malloc (votec * sizeof (*md));

    for (li = lmin; ue && li <= lmax; li++) {
        md[i].info = &m_info[M_PUE]; 
        md[i].s = NULL; 
//        md[i].l = NULL; 
//        md[i].r = NULL; 
        md[i].len_l = li; 
        md[i].len_r = -1;
        md[i].w_l = md[i].w_r = 1;
        ++i;
    }
    for (ri = rmin; ub && ri <= rmax; ri++) {
        md[i].info = &m_info[M_PUB]; 
        md[i].s = NULL; 
//        md[i].l = NULL; 
//        md[i].r = NULL; 
        md[i].len_l = -1; 
        md[i].len_r = ri;
        md[i].w_l = md[i].w_r = 1;
        ++i;
    }

    assert (i == votec);

    mv_init();

    ps = phonstats_new(1 + ((rmax > lmax) ? rmax : lmax), NULL);

    if (opt.prior_data_given) {
        phonstats_update_from_file(ps, opt.prior_data_arg);
    }
}

struct seglist * 
segment_ub(struct input *in, int idx)
{
    char *u = in->u[idx].s;
    struct seglist *segl = seglist_new();
    int len = strlen(u);
    int j = 0;
    int i = 1;
    unsigned short seg[len + 1];
    double votes[len];
    struct mlist *ml = mlist_new(votec);

    seg[0] = 0;
    
    phonstats_update(ps, u);

    ml->s = u;
    ml->slen = len;
    for (j = 0; j < votec; j++) {
        md[j].s = u;
        mlist_add_old(ml, &md[j], ps);
    }

    mv_getvotes(votes, ml);

    i = 1;
    for (j = 1; j < len; j++) {
        if (votes[j] > 0) 
        {
            seg[i] = j;
            ++i; ++seg[0];
        } else {
        }
    }

    seglist_add(segl, seg);
    return segl;
}

void 
segment_ub_update(char *s, struct seglist *segl)
{
    return;
}

void 
segment_ub_cleanup()
{
/*
    int i;
    for (i = 0; i < votec; i++) {
        printf("%s(%d/%d): %f %f\n", md[i].info->sname, md[i].len_l, md[i].len_r, md[i].w_l, md[i].w_r);
    }
*/
    free(md);
}
