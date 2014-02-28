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
 * This file includes routines to calculate various predictability measures.
 *
 * All functions calculate respective measures for a given string xy, 
 * where x, and y are substrings. 
 *
 * The functions accept a complete string (s), the position offset where 
 * x ends and y begins (pos), length of x (x_len) and length of y (y_len).
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "strutils.h"
#include "pred.h"

/* P() - probability estimate of a string, for now, this only 
 *       an alias to phonstats_rfreq_ng()
 */
static inline double 
P(struct phonstats *ps, char *ng)
{
//printf("P - %f\n",  phonstats_rfreq_ng(ps, ng));
    return phonstats_P(ps, ng, 0);
}

void
add_bow_eow(char *dest, const char *src)
{
    const char *ch_src = src;
    char *ch_dst = dest;

    *ch_dst = BOW_CH;
    ch_dst++;
    while(*ch_src) {
        *ch_dst = *ch_src;
        ++ch_dst;
        ++ch_src;
    }
    *ch_dst = EOW_CH;
    ++ch_dst;
    *ch_dst = '\0';
}



/* joint_p() - calculates joint probability of x and y
 *
 */
double
joint_p(struct phonstats *ps, char *x, char *y)
{
    char xy[strlen(x) + strlen(y) + 1];
    strcpy(xy, x);
    strcat(xy, y);
    return P(ps, xy);
}

double
joint_p_str(struct phonstats *ps, char *s, int pos, int x_len, int y_len)
{
    int slen = strlen(s);
    char tmps[slen + 3];
    char *ng_xy;
    double p;

    assert((x_len + y_len) <= ps->max_ng);
    assert(x_len <= (pos + 1));

    add_bow_eow(tmps, s);

    ng_xy = str_span(tmps, pos + 1 - x_len, y_len + x_len);
    p = P(ps, ng_xy);
    free(ng_xy);
    return p;
}

/* cond_p() - calculates P(y|x)
 *
 */
double
cond_p(struct phonstats *ps, char *x, char *y)
{
    char xy[strlen(x) + strlen(y) + 1];
    strcpy(xy, x);
    strcat(xy, y);
//printf("tp.. %s-%s: %f\n", x, y,(double) phonstats_freq_ng(ps, xy) / (double) phonstats_freq_ng(ps, x) );
//    return P(ps, xy) / P(ps, x);
    return (double) phonstats_freq_ng(ps, xy) / 
            (double) phonstats_freq_ng(ps, x); 
}

/* cond_p_r() - calculates P(x|y)
    --> p(l|r)
 *
 */
double
cond_p_r(struct phonstats *ps, char *x, char *y)
{
    char xy[strlen(x) + strlen(y) + 1];
    strcpy(xy, x);
    strcat(xy, y);
//    return P(ps, xy) / P(ps, x);
    return (double) phonstats_freq_ng(ps, xy) / 
            (double) phonstats_freq_ng(ps, y); 
}

double
cond_p_str (struct phonstats *ps, char *s, int pos, int x_len, int y_len)
{
    int slen = strlen(s);
    char tmps[slen + 3];
    char *ng_x, *ng_y;
    double p;

    assert((x_len + y_len) <= ps->max_ng);
// TODO: (maybe) falling back to shorter n-grams may be a better idea
    assert(x_len <= (pos + 1));

    add_bow_eow(tmps, s);

    ng_x = str_span(tmps, pos + 1 - x_len, x_len);
    ng_y = str_span(tmps, pos + 1, y_len);

    p = cond_p(ps, ng_x, ng_y);
    free(ng_y); free(ng_x);

    return p;
}

double
pmi(struct phonstats *ps, char *x, char *y)
{
    double p_xy = joint_p(ps, x, y),
           p_x = P(ps, x),
           p_y = P(ps, y);

    return log2(p_xy / (p_x * p_y));
}

/* this version normalizes pmi by dividing it to -log(p(x,y))
 * which gives better bounded results: if two variables are perfectly 
 * associated the result is 1, if they are independent, it is 0, and if
 * they are not associated (p(x,y) is 0), then it is -1
 */
double
npmi(struct phonstats *ps, char *x, char *y)
{
    double p_xy = joint_p(ps, x, y),
           p_x = P(ps, x),
           p_y = P(ps, y);

//printf("npmi(%s, %s): p_xy = %f, p_x = %f p_y = %f : %f\n", x, y, p_xy, p_x, p_y, p_xy / (p_x * p_y));
    if (p_xy == 0.0) return -1.0;
    return (log2(p_xy) - log2(p_x) - log2(p_y))/ -log2(p_xy);
}

double
pmi_str(struct phonstats *ps, char *s, int pos, int x_len, int y_len)
{
    double p_xy = joint_p_str(ps, s, pos, x_len, y_len),
           p_x = joint_p_str(ps, s, pos, x_len, 0),
           p_y = joint_p_str(ps, s, pos, 0, y_len);

    return log2(p_xy / (p_x * p_y));
}

double
cond_entropy(struct phonstats *ps, char *x, int y_len)
{
    int x_len = strlen(x);
    char xy[x_len + y_len + 1];
    char *y = xy + x_len;
    double ent = 0.0;
    size_t f_xy, f_x;
    int i;

    f_x = phonstats_freq_ng(ps, x);

    strcpy(xy, x);
    for (i = 0; i < ps->n_typ[y_len - 1]; i++) {
        strcpy(y, ps->ngstr[y_len - 1][i]);
        f_xy = phonstats_freq_ng(ps, xy);
        if(f_xy != 0) {
            double tmp = (double)f_xy/(double)f_x;
            ent -=  tmp * log2(tmp);
        }
    }

    return ent;
}

double
cond_entropy_r(struct phonstats *ps, char *y, int x_len)
{
    int  y_len = strlen(y);
    char xy[x_len + y_len + 1];
    char *yp = xy + x_len ;
    double f_xy, f_y;
    double ent = 0.0;
    int i;

    f_y = phonstats_freq_ng(ps, y);

    // TODO: avoid copying y all the time 
    for (i = 0; i < ps->n_typ[x_len - 1]; i++) {
        strcpy(xy, ps->ngstr[x_len - 1][i]);
        strcpy(yp, y);
        f_xy = phonstats_freq_ng(ps, xy);
        if(f_xy != 0) {
            double tmp = (double)f_xy/(double)f_y;
            ent -=  tmp * log2(tmp);
        }
    }

    return ent;
}

double
cond_entropy_str(struct phonstats *ps, char *s, int pos, int x_len, int y_len)
{
    int  slen = strlen(s);
    char tmps[slen + 3];
    char *x;
    double ent = 0.0;

    add_bow_eow(tmps, s);

    x = str_span(tmps, pos + 1 - x_len, x_len);
    ent = cond_entropy(ps, x, y_len);
    free(x);

    return ent;
}

size_t
sv(struct phonstats *ps, char *x, int y_len)
{
    int  x_len = strlen(x);
    char xy[x_len + y_len + 1];
    char *y = xy + x_len ;
    size_t sv = 0;
    int i;

    strcpy(xy, x);
    for (i = 0; i < ps->n_typ[y_len - 1]; i++) {
        strcpy(y, ps->ngstr[y_len - 1][i]);
        sv += (phonstats_freq_ng(ps, xy) > 0);
    }

    return sv;
}

size_t
sv_r(struct phonstats *ps, char *y, int x_len)
{
    int  y_len = strlen(y);
    char xy[x_len + y_len + 1];
    char *yp = xy + x_len ;
    size_t pv = 0;
    int i;

    assert(y != NULL);
    assert(x_len > 0);

    // TODO: avoid copying y all the time 
    for (i = 0; i < ps->n_typ[x_len - 1]; i++) {
        strcpy(xy, ps->ngstr[x_len - 1][i]);
        strcpy(yp, y);
        pv += (phonstats_freq_ng(ps, xy) > 0);
    }

    return pv;
}

size_t
pv(struct phonstats *ps, int x_len, char *y)
{
    int  y_len = strlen(y);
    char xy[x_len + y_len + 1];
    size_t pv = 0;
    int i;

    strcpy(xy + x_len, y);
    for (i = 0; i < ps->n_typ[x_len - 1]; i++) {
        strncpy(xy, ps->ngstr[x_len - 1][i], x_len);
        pv += (phonstats_freq_ng(ps, xy) > 0);
    }

    return pv;
}


inline void
char_swap(char *s, int len, int pos)
{
    s[len] = s[pos]; // save the char overwriting '\0'
    s[pos] = '\0';
}

inline void
char_unswap(char *s, int len, int pos)
{
    assert(s[pos] == '\0');
    s[pos] = s[len]; // save the char overwriting '\0'
    s[len] = '\0';
}

double 
pred_calc(struct phonstats *ps,
          enum m_id m,
          char *x, char *y,
          int x_len, int y_len)
{
    switch(m) {
        case M_JP: {
            assert(x != NULL && y != NULL);
            return joint_p(ps, x, y);
        } break;
        case M_TP: {
            assert(x != NULL && y != NULL);
            return  cond_p(ps, x, y);
        } break;
        case M_MI: {
            assert(x != NULL && y != NULL);
            return  pmi(ps, x, y);
        } break;
        case M_H: {
            assert(x != NULL);
            return cond_entropy(ps, x, y_len);
        } break;
        case M_SV: {
            assert(x != NULL);
            return (double) sv(ps, x, y_len);
        } break;
        case M_RTP: {
            assert(x != NULL && y != NULL);
            return cond_p_r(ps, x, y);
        } break;
        case M_RH: {
            assert(y != NULL);
            return cond_entropy_r(ps, y, x_len);
        } break;
        case M_RSV: {
            assert(y != NULL);
            return (double) sv_r(ps, y, x_len);
        } break;
        default : {
            fprintf(stderr, "pred_calc(): unknown measure %d\n", m);
            exit(-1);
        }
    };
    
}

static inline char *
ng_l (struct mdata *m, int pos, int len)
{
    assert(m->len_l > 0);
    assert(pos >= 0 && pos <= len);

//printf("ng_l: s=%s -- pos=%d len=%d\n", m->s, pos, len);
    int ngstart = pos - m->len_l;
    int nglen = (m->len_l + ngstart < m->len_l) ?     
                 m->len_l + ngstart : m->len_l;
    char  *s = malloc(nglen + 2);

    if (ngstart < 0) {
        sprintf(s, "%c%.*s", BOW_CH, nglen, m->s);
    } else {
        sprintf(s, "%.*s", nglen, m->s + ngstart);
    }
//printf("\t?=%d ngstart=%d nglen=%d -->%s\n", (m->len_l + ngstart < m->len_l), ngstart, nglen, s);

    return s;
}

static inline char *
ng_r (struct mdata *m, int pos, int len)
{
    assert(m->len_r > 0);
    assert(pos >= 0 && pos <= len);
//printf("ng_r: s=%s -- pos=%d len=%d\n", m->s, pos, len);
    int nglen = (pos + m->len_r < len) ?
                 m->len_r : len - pos;
    char *s = malloc(nglen + 2);
    if (nglen < m->len_r) {
        sprintf(s, "%.*s%c", nglen, m->s + pos,  EOW_CH);
    } else {
        sprintf(s, "%.*s", nglen, m->s + pos);
    }
//printf("\t?=%d nglen=%d -->%s\n", (pos + m->len_r < len), nglen, s);
    return s;
}

double
_calc_pred_single(struct phonstats *ps, struct mdata *m, int pos, int len)
{
    char *l = NULL;
    char *r = NULL;
    double pm = 0.0;

    assert(m->info->mmask & (M_PFMASK | M_PRMASK));

    switch(m->info->mid) {
        case M_JP: {
            l = ng_l(m, pos, len);
            r = ng_r(m, pos, len);
            pm =  joint_p(ps, l, r);
        } break;
        case M_TP: {
            l = ng_l(m, pos, len);
            r = ng_r(m, pos, len);
//printf("%d-%d: %s-%s\n", m->len_l, m->len_r, l, r);
            pm =  cond_p(ps, l, r);
// printf("tp ... %s-%s: %f\n", l, r, pm);
        } break;
        case M_MI: {
            l = ng_l(m, pos, len);
            r = ng_r(m, pos, len);
            pm =  pmi(ps, l, r);
        } break;
        case M_H: {
            assert(m->len_r != 0);
            l = ng_l(m, pos, len);
            pm =  cond_entropy(ps, l, m->len_r);
        } break;
        case M_SV: {
            assert(m->len_r != 0);
            l = ng_l(m, pos, len);
            pm =  (double) sv(ps, l, m->len_r);
        } break;
        case M_RTP: {
            l = ng_l(m, pos, len);
            r = ng_r(m, pos, len);
            pm = cond_p_r(ps, l, r);
        } break;
        case M_RH: {
            assert(m->len_l != 0);
            r = ng_r(m, pos, len);
            pm = cond_entropy_r(ps, r, m->len_l);
        } break;
        case M_RSV: {
            assert(m->len_l != 0);
            r = ng_r(m, pos, len);
            pm = (double) sv_r(ps, r, m->len_l);
        } break;
        default : {
            fprintf(stderr, "pred_calc(): unknown measure %d\n", m->info->mid);
            exit(-1);
        }
    };
    if (l != NULL) free(l);
    if (r != NULL) free(r);
    return pm;
}

double
calc_pred_single(struct phonstats *ps, struct mdata *m, int pos)
{
    int len = strlen(m->s);
    return _calc_pred_single(ps, m, pos, len);
}

double *
calc_pred_list(struct phonstats *ps, struct mdata *m)
{
    int len = strlen(m->s);
    int j = 0;
    double *plist = NULL;

    assert(ps->max_ng > m->len_l);

    plist = malloc((len + 1) * sizeof (*plist));

    for (j = 0; j <= len; j++) {
        plist[j] = _calc_pred_single(ps, m, j, len);
//printf("%s .. plist[%d] = %f\n", m->info->sname, j, plist[j]);
    }
    return plist;
}


int
pred_init(struct mdlist *mdl, struct phonstats *ps)
{
    int  pred_votec = 0;
    int  lmin = 0, rmin = 0, lmax = 0, rmax = 0;
    int mcount = (opt.pred_m_given) ? opt.pred_m_given : 1;
    int pi;
    int li, ri;

    lmin = opt.pred_xmin_arg;
    rmin = opt.pred_ymin_arg;
    lmax = opt.pred_xmax_arg;
    rmax = opt.pred_ymax_arg;

    if(opt.pred_xlen_given) {
        lmin = lmax = opt.pred_xlen_arg;
    }
    if(opt.pred_ylen_given) {
        rmin = rmax = opt.pred_ylen_arg;
    }

    assert(lmax >= lmin && rmax >= rmin);

    pred_votec = (lmax - lmin + 1) * (rmax - rmin + 1)
                * mcount;

    for (pi = 0; pi < mcount; pi++) {
        enum m_id m = opt.pred_m_arg[pi];
        assert (m & (M_PFMASK | M_PRMASK));
        for (li = lmin; li <= lmax; li++) {
            for (ri = rmin; ri <= rmax; ri++) {
                struct mdata *md = mdata_new();
                md->ps = ps;
                md->info = &m_info[m];
                md->s = NULL;
                if (opt.pred_swaplr_flag && 
                       (md->info->mmask & M_PRMASK)) {
                    md->len_l = ri;
                    md->len_r = li;
                } else {
                    md->len_l = li;
                    md->len_r = ri;
                }
                mdlist_add(mdl, md);
            }
        }
    }

    return pred_votec;
}
