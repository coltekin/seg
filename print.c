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
 *
 *
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <glib.h>
#include "seglist.h"
#include "strutils.h"
#include "predictability.h"
#include "prob_dist.h"
#include "print.h"

#define SEP ','
/* print_pred() - print predictability scores for given input
 *
 */
void
print_pred(FILE *fp, struct input *in) 
{
    int x_len = opt.pred_xlen_arg, 
        y_len = opt.pred_ylen_arg;
    struct phonstats *ps = phonstats_new(x_len + y_len, NULL);;
    int i, j, m;
    int first = 1;
    unsigned mmask = 0;
    double *plist[PM_MAX] = {NULL};
    static struct prob_dist *mdist[PM_MAX] = {NULL};


    assert(opt.pred_m_given); 

    for (m = 0; m < ((opt.pred_m_given) ? opt.pred_m_given : 1) ; ++m) { 
        mmask |= p_info[opt.pred_m_arg[m]].mmask; 
        if(opt.pred_norm_given) {
            mdist[opt.pred_m_arg[m]] = prob_dist_new();
        }
    }

    if (opt.prior_data_given) {
        for (i = 0; i < in->size; i++) {
            phonstats_update(ps, in->u[i].s);
        }
        if(opt.pred_norm_given) {
            for (i = 0; i < in->size; i++) {
                for (m = 0; m < PM_MAX; m++) {
                    if(mmask & p_info[m].mmask) {
                        plist[m] = pred_list(ps, in->u[i].s, m, x_len, y_len); 
                        for (j = 1; j < strlen(in->u[i].s); j++) {
                            prob_dist_update(mdist[m], plist[m][j]);
                        }
                        free(plist[m]);
                    }
                }
            }
        }
    }

    if (opt.print_header_flag) {
        if (opt.print_ph_flag) { 
            fprintf(fp, "phoneme");
            first = 0;
        } else if (opt.print_phng_flag) {
            fprintf(fp, "phoneme-ng");
            first = 0;
        }
        for (m = 0; m < PM_MAX; m++) {
            if(p_info[m].mmask & mmask) {
                if (!first) fprintf(fp, "%c", SEP);
                first = 0;
                fprintf(fp, "%s_%d_%d", p_info[m].sname, x_len, y_len);
            }
        }
        if (opt.print_lb_flag) { 
            if (!first) fprintf(fp, "%c", SEP);
            first = 0;
//            fprintf(fp, "wboundary");
            fprintf(fp, "lbegin,");
            fprintf(fp, "lboundary");
        } 
        if(opt.print_ub_flag) {
            if (!first) fprintf(fp, "%c", SEP);
            first = 0;
//            fprintf(fp, "uboundary");
            fprintf(fp, "ubegin,");
            fprintf(fp, "uboundary");
        }
        fprintf(fp, "\n");
    }

    for (i = 0; i < in->size; i++) {
        char *s = in->u[i].s;
        int len = strlen(s);
        int k = 1;
        int boundary = 0;
        int wbegin = 0;

        if (!opt.prior_data_given) {
            phonstats_update(ps, in->u[i].s);
        }

        if (opt.progress_given) {
            if((i %  opt.progress_arg) == 0) {
                fprintf(stderr,"%*d/%zu\r", 6, i, in->size);
            }
        }
        for (m = 0; m < PM_MAX; m++) {
            if(p_info[m].mmask & mmask) {
                plist[m] = pred_list(ps, s, m, x_len, y_len); 
            }
        }
        for (j = 0; j <= len; j++) {
            int offset = j - x_len;
            int winsz = x_len + y_len;
            first = 1;
            if ((in->u[i].seg != NULL) &&  
                (in->u[i].seg[0] != 0) &&
                (k <= in->u[i].seg[0]) && 
                (j == in->u[i].seg[k])) {
                    k++;
                    boundary = 1;
            }
            if (opt.print_ph_flag) { 
                fprintf(fp, "%c", (j == 0) ? '<' : s[j-1]);
                first = 0;
            } else if (opt.print_phng_flag) { 
                if(offset < 0) {
                    int tmp = offset;
                    while (tmp < 0) {
                        fprintf(fp, "<");
                        tmp++;
                    }
                    fprintf(fp, "%.*s", winsz + offset, s);
                } else {
                    fprintf(fp, "%.*s", winsz, s + offset);
                }
                int tmp = j + y_len - len;
                while (tmp > 0) {
                    fprintf(fp, ">");
                    --tmp;
                }
                first = 0;
            }
            for (m = 0; m < PM_MAX; m++) {
                if (p_info[m].mmask & mmask) {
                    if (!first) fprintf(fp, "%c", SEP);
                    first = 0;
                    fprintf(fp, "%f", (opt.pred_norm_given) ?
                                       prob_dist_norm(mdist[m], plist[m][j]):
                                       plist[m][j]);
                }
            }
            if (opt.print_lb_flag) { 
                if (!first) fprintf(fp, "%c", SEP);
                first = 0;
                fprintf(fp, "%c%c", (j == x_len || wbegin == 1) 
                                  ? 'T' : 'F', SEP);
                if (j == len || boundary == 1) {
                    fprintf(fp, "T");
                    wbegin = 1;
                } else {
                    fprintf(fp, "F");
                    wbegin = 0;
                }
            } 
            boundary = 0;
            if(opt.print_ub_flag) {
                if (!first) fprintf(fp, "%c", SEP);
                first = 0;
                fprintf(fp, "%c%c", (j == x_len) ? 'T' : 'F', SEP);
                fprintf(fp, "%c", (j == len) ? 'T' : 'F');
            }
            fprintf(fp, "\n");
        }
        for (m = 0; m < PM_MAX; m++) {
            if(plist[m]) {
                free(plist[m]);
                plist[m] = NULL;
            }
        }
    }
    if (opt.progress_given) {
        fprintf(stderr, "\n");
    }

    phonstats_free(ps);
}


void
print_pred_list(char *s, double *mlist) 
{
    int len = strlen(s);
    int i;

    printf("< %f ", mlist[0]);
    for (i = 0; i < len ; i++) {
        printf("%c %f ", s[i], mlist[i+1]);
    }
    printf(">\n");
}

struct ptp_stat {
    size_t count;
    size_t wbound;
    size_t wbegin;
    size_t wend;
    size_t ubegin;
    size_t uend;
};

void
print_ptp(FILE *fp, struct input *in) 
{
    GHashTable  *ptp;
    int i;
    unsigned short j, k;
    int nglen = opt.ub_nglen_arg;
    struct phonstats *ps = phonstats_new(nglen + 1, NULL);

    ptp = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
    if (opt.prior_data_given) {
        for (i = 0; i < in->size; i++) {
            phonstats_update(ps, in->u[i].s);
        }
    }

    for (i = 0; i < in->size; i++) {
        char *s = in->u[i].s;
        int len = strlen(s);

        if (!opt.prior_data_given) {
            phonstats_update(ps, in->u[i].s);
        }
        for (j = 0; j < len - 1; j++) {
            char *pp = str_span(s, j, nglen);
            struct ptp_stat *tmp;
            unsigned short wbound = 0, wbegin = 0, wend = 0, 
                           ubegin = 0, uend = 0;

            if (j == 0) ubegin = wbegin = 1;
            if (j == len - nglen) uend = wend = 1;

            if (seg_check(in->u[i].seg, j)) wbegin = 1;
            if (seg_check(in->u[i].seg, j + nglen)) wend = 1;
            for (k=1; k < nglen; k++) {
                if (seg_check(in->u[i].seg, j + k)) {
                    wbound = 1;
                    break;
                }
            }

            tmp = g_hash_table_lookup(ptp, pp);
            if (tmp != NULL) {
                free(pp);
            } else {
                tmp = calloc(1, sizeof (*tmp));
                g_hash_table_insert (ptp, pp, tmp);
            }
            tmp->count += 1;
            tmp->wbound += wbound;
            tmp->wbegin += wbegin;
            tmp->ubegin += ubegin;
            tmp->wend += wend;
            tmp->uend += uend;
        }
    }

    GHashTableIter iter;
    g_hash_table_iter_init (&iter, ptp);
    gpointer key, val;
    fprintf(fp, "bound,nbound,wbegin,wend,ubegin,uend,all");

    if(opt.print_ptp_arg[0] != print_ptp_arg_none) {
        for (i = 0; i < opt.print_ptp_given ; ++i) {
            int m = opt.print_ptp_arg[i];
            fprintf(fp, ",%s_ub,ub_%s,%s_ue,ue_%s", 
                        p_info[m].sname, p_info[m].sname, 
                        p_info[m].sname, p_info[m].sname);
        }
    }
    fprintf(fp, "\n");

    while (g_hash_table_iter_next (&iter, &key, &val)) {
        char *pp = (char *) key;
        struct ptp_stat *counts = (struct ptp_stat *) val;
        fprintf(fp, "%s,%zu,%zu,%zu,%zu,%zu,%zu,%zu", pp,
                    counts->wbound, counts->count - counts->wbound,
                    counts->wbegin, counts->wend,
                    counts->ubegin, counts->uend,
                    counts->count);
        if(opt.print_ptp_arg[0] != print_ptp_arg_none) {
            for (i = 0; i < opt.print_ptp_given ; ++i) {
                int m = opt.print_ptp_arg[i];
                float m_ub = 0.0, m_ue = 0.0, ub_m = 0.0, ue_m = 0.0;
                switch (m) {
                    case PM_MI:
                        m_ub = ub_m = npmi(ps, "<", pp);
                        m_ue = ue_m = npmi(ps, pp, ">");
                     break;
                     case PM_TP:
                        m_ub = cond_p(ps, "<", pp);     // P(pp|<)
                        ub_m = cond_p_r(ps, "<", pp);   // P(<|pp)
                        m_ue = cond_p_r(ps, pp, ">");   // P(pp|>)
                        ue_m = cond_p(ps, pp, ">");     // P(>|pp)
                     break;
                     case PM_JP:
                        m_ub = ub_m = joint_p(ps, "<", pp);
                        m_ue = ue_m = joint_p(ps, pp, ">");
                     break;
                     default:
                        fprintf(stderr, "method %d (%s) is not implemented yet\n", 
                                m, p_info[m].sname);
                        exit (-1);
                }
                fprintf(fp, ",%f,%f,%f,%f", m_ub, ub_m, m_ue, ue_m);     
            }
        }
        fprintf(fp, "\n");
    }

    phonstats_free(ps);
    g_hash_table_destroy(ptp);
}

void
print_wfreq(FILE *fp, struct input *in)
{
    GHashTable  *lexhash;
    int i, j;
    int nglen = opt.print_wfreq_arg;
    struct phonstats *ps = phonstats_new(nglen, NULL);

    lexhash = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);

    for (i = 0; i < in->size; i++) {
        char *s = in->u[i].s;
        unsigned short *seg = in->u[i].seg;
        char **segstr = seg_to_strlist(s, seg);
        char **tmp = segstr;
        while (*tmp){
            if (g_hash_table_lookup(lexhash, *tmp)) {
                free(*tmp);
            } else {
                g_hash_table_insert (lexhash, *tmp, *tmp);
            }
            ++tmp;
        }
        free(segstr);

        phonstats_update(ps, s);
    }


    if (opt.print_wfreq_sum_flag) {
        printf("; max_ng: %zu\n", ps->max_ng);
        printf("; n_updt: %zu\n", ps->n_updt);
        printf("; n_tok: ");
        for (i = 0; i < ps->max_ng; i++) 
            printf(" %zu", ps->n_tok[i]);
        printf("\n; n_typ: ");
        for (i = 0; i < ps->max_ng; i++) 
            printf(" %zu", ps->n_typ[i]);
        printf("\n");
    }

    if (opt.print_header_flag) {
       printf("ng,len,freq,is_word\n");
    }

    for (i = 0; i < ps->max_ng; i++) {
        for (j = 0; j < ps->n_typ[i]; j++) {
            char *ng = ps->ngstr[i][j];
            printf("%s,%d,%zu,%c\n", ng, i, phonstats_freq_ng(ps, ng), 
                   g_hash_table_lookup(lexhash, ng) ? 'T' : 'F');
        }
    }

    phonstats_free(ps);
    g_hash_table_destroy(lexhash);

}


