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
#include "pred.h"
#include "mdata.h"
#include "measures.h"


int 
ub_init(struct mdlist *mdl, struct phonstats *ps, enum m_id ub_id, enum m_id ue_id)
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

    if (ub_id == M_SUB) {
        assert (ue_id == M_SUE);
        if (opt.sub_ngmin_given) lmin = rmin = opt.sub_ngmin_arg;
        if (opt.sub_ngmax_given) lmax = rmax = opt.sub_ngmax_arg;
    }

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
        struct mdata *md = mdata_new_full(ue_id, NULL, li, -1);
        md->ps = ps;
        mdlist_add(mdl, md);
    }
    for (ri = rmin; ub && ri <= rmax; ri++) {
        struct mdata *md = mdata_new_full(ub_id, NULL, -1, ri);
        md->ps = ps;
        mdlist_add(mdl, md);
    }

    return ub_votec;
}

static inline double 
_calc_ub_single(struct phonstats *ps, struct mdata *m, int pos, int len)
{
    char *ng_r;
    double ub;
    
    assert(pos <= len);
    assert(m->info->mid == M_PUB || m->info->mid == M_SUB || m->info->mid == M_LPB);

    assert (m->len_r > 0);
    if (pos + m->len_r > len) ng_r = strndup(m->s + pos, len - pos);
    else                      ng_r = strndup(m->s + pos, m->len_r); 

    ub = (pos == len) ? 0.5 : cond_p_r(ps, "<", ng_r);
    
    free(ng_r);

    return ub;
}

double 
calc_ub_single(struct phonstats *ps, struct mdata *m, int pos)
{
    int len = strlen(m->s);
    return  _calc_ub_single(ps, m, pos, len);
}

double *
calc_ub_list(struct phonstats *ps, struct mdata *m)
{
    int len = strlen(m->s);
    int j = 0;
    double *ubl = NULL;

    assert(ps->max_ng > m->len_r);

    ubl = malloc((len + 1) * sizeof (*ubl));

    for (j = 0; j <= len; j++) {
        ubl[j] = _calc_ub_single(ps, m, j, len);
    }
    return ubl;
}

double 
_calc_ue_single(struct phonstats *ps, struct mdata *m, int pos, int len)
{
    char *ng_l;
    double ub;
    
    assert(pos <= len);
    assert(m->info->mid == M_PUE || m->info->mid == M_SUE || m->info->mid == M_LPE);

    assert (m->len_l > 0);
    if (pos > m->len_l) ng_l = strndup(m->s + pos - m->len_l, m->len_l);
    else                ng_l = strndup(m->s , pos); 

    ub = (pos == 0) ? 0.5 : cond_p(ps, ng_l, ">");
    
    free(ng_l);

    return ub;
}

double 
calc_ue_single(struct phonstats *ps, struct mdata *m, int pos)
{
    int len = strlen(m->s);
    return  _calc_ue_single(ps, m, pos, len);
}

double *
calc_ue_list(struct phonstats *ps, struct mdata *m)
{
    int len = strlen(m->s);
    int j = 0;
    double *uel = NULL;

    assert(ps->max_ng > m->len_l);

    uel = malloc((len + 1) * sizeof (*uel));

    for (j = 0; j <= len; j++) {
        uel[j] = _calc_ue_single(ps, m, j, len);
    }
    return uel;
}
#if 0

// some old trials, not used anymore.

static inline double
avg_cond_p(char *l)
{
    double avg = 0.0;
    int i ;

    assert(phonstats_freq_ng(ps, l) > 0);

    // TODO: check for overflow
    for (i = 0; i < ps->n_typ[0]; i++) {
        char *r = ps->ngstr[0][i];
        avg +=  phonstats_P(ps, r, SMOOTH_NONE)
              * cond_p(ps, l, r);
    }

    return avg;
}

static inline double
avg_cond_p_r(char *r)
{
    double avg = 0.0;
    int i ;
    assert(phonstats_freq_ng(ps, r) > 0);

    // TODO: check for overflow
    for (i = 0; i < ps->n_typ[0]; i++) {
        char *l = ps->ngstr[0][i];
        avg +=  phonstats_P(ps, l, SMOOTH_NONE)
              * cond_p_r(ps, l, r);
    }

    return avg;
}

/* probability of a boundary given we observed ng to the left
 */
static inline double
p_bndry_l(char *ng)
{
    int len = strlen(ng);
    char ng_ub[len + 2];
    sprintf(ng_ub, "%s>", ng);

    return  (double) phonstats_freq_ng(ps, ng_ub)
          / (double) phonstats_freq_ng(ps, ng);
}

/* probability of a boundary given we observed ng to the right
 */
static inline double
p_bndry_r(char *ng)
{
    int len = strlen(ng);
    char ng_ub[len + 2];
    sprintf(ng_ub, "<%s", ng);

    return  (double) phonstats_freq_ng(ps, ng_ub)
          / (double) phonstats_freq_ng(ps, ng);
}

/* probability of (left) ng, given there is a boundary following it
 */
static inline double
bndry_p_l(char *ng)
{
    int len = strlen(ng);
    char ng_ub[len + 2];
    strcpy(ng_ub, ng);
    strcat(ng_ub, ">");

    return  (double) phonstats_freq_ng(ps, ng_ub)
          / (double) ps->n_updt;
}

/* probability of (righ) ng, given there is a boundary before it
 */
static inline double
bndry_p_r(char *ng)
{
    int len = strlen(ng);
    char ng_ub[len + 2];
    strcpy(ng_ub, "<");
    strcat(ng_ub, ng);

    return  (double) phonstats_freq_ng(ps, ng_ub)
          / (double) ps->n_updt;
}

/* probability of (left) ng, given it does not occur just before a boundary
 */
static inline double
n_bndry_p_l(char *ng)
{
    int len = strlen(ng);
    char ng_ub[len + 2];
    strcpy(ng_ub, ng);
    strcat(ng_ub, ">");

    return  (double) (phonstats_freq_ng(ps, ng) - phonstats_freq_ng(ps, ng_ub))
          / (double) ps->n_tok[len];
}

/* probability of (right) ng, given it does not occur just after a boundary
 */
static inline double
n_bndry_p_r(char *ng)
{
    int len = strlen(ng);
    char ng_ub[len + 2];
    strcpy(ng_ub, "<");
    strcat(ng_ub, ng);
    return  (double) (phonstats_freq_ng(ps, ng) - phonstats_freq_ng(ps, ng_ub))
          / (double) ps->n_tok[len];
}

static inline double
bndry_mi_l(char *ng)
{
    int len = strlen(ng);
    char ng_ub[len + 2];
    double p_xy = 0.0, p_x = 0.0, p_y = 0.0;

    sprintf(ng_ub, "%s>", ng);

    p_xy = (double) phonstats_freq_ng(ps, ng_ub) 
          /(double) ps->n_tok[len + 1];
    p_x = (double) phonstats_freq_ng(ps, ng)
         /(double) ps->n_tok[len];
    p_y = (double) ps->n_updt
         /(double) ps->n_tok[1];

    return log2(p_xy) - log2(p_x) - log2(p_y);
}

static inline double
bndry_mi_r(char *ng)
{
    int len = strlen(ng);
    char ng_ub[len + 2];
    double p_xy = 0.0, p_x = 0.0, p_y = 0.0;

    sprintf(ng_ub, "<%s", ng);

    p_xy = (double) phonstats_freq_ng(ps, ng_ub) 
          /(double) ps->n_tok[len + 1];
    p_x = (double) ps->n_updt
         /(double) ps->n_tok[1];
    p_y = (double) phonstats_freq_ng(ps, ng)
         /(double) ps->n_tok[len];

    return log2(p_xy) - log2(p_x) - log2(p_y);
}
#endif
