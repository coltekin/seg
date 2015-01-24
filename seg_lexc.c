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
#include <string.h>
#include <stdlib.h>
#include "seglist.h"
#include "lexc.h"
#include "mvote.h"
#include "mlist.h"
#include "io.h"
#include "pred.h"
#include "mdata.h"
#include "seg_lexc.h"


static struct phonstats *ps = NULL;     // statistics over the corpus
static struct phonstats *lps = NULL;    // statistics over the lexicon
static cg_lexicon *L;

static struct mdata *md = NULL;
static int nvotes = 0;

static short seg_pred = 0;  // Just for conveniently checking if
static short seg_ub = 0;    // particular measure is in use 
static short seg_lex = 0;   // or not.



void 
segment_lexc_init(struct input *in)
{
    int mdalloc = BUFSIZ / sizeof(*md);
    int i = 0, li = 0, ri = 0;
    int maxng = 0;


    md = malloc(BUFSIZ);

// UB initialization...
    int ub = 0, ue = 0;
    int  ub_votec = 0;
    int  lmin = 0, lmax = 0, rmin = 0, rmax = 0; 

    seg_ub = 1;

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

    while (nvotes + ub_votec > mdalloc) {
        md = realloc(md, BUFSIZ);   //FIXME: this looks like a bug: the size is constant
        mdalloc += BUFSIZ / sizeof(*md);
    }

    for (li = lmin; ue && li <= lmax; li++) {
        md[i].info = &m_info[M_PUE];
//        md[i].s = md[i].l = md[i].r = NULL;
        md[i].s = NULL;
        md[i].len_l = li;
        md[i].len_r = -1;
        md[i].w_l = md[i].w_r = 1;
        ++i;
    }
    for (ri = rmin; ub && ri <= rmax; ri++) {
        md[i].info = &m_info[M_PUB];
//        md[i].s = md[i].l = md[i].r = NULL;
        md[i].s = NULL;
        md[i].len_l = -1;
        md[i].len_r = ri;
        md[i].w_l = md[i].w_r = 1;
        ++i;
    }
    maxng = (lmax > maxng) ? lmax : maxng;
    maxng = (rmax > maxng) ? rmax : maxng;
    nvotes += ub_votec;

// pred initialization
    int  pred_votec = 0;
    int mcount = (opt.pred_m_given) ? opt.pred_m_given : 1;
    int pi;

    seg_pred = 1;

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

    while (nvotes + pred_votec > mdalloc) {
        md = realloc(md, BUFSIZ);
        mdalloc += BUFSIZ / sizeof(*md);
    }

    for (pi = 0; pi < mcount; pi++) {
        enum m_id m = opt.pred_m_arg[pi];
        assert (m & (M_PFMASK | M_PRMASK));
        for (li = lmin; li <= lmax; li++) {
            for (ri = rmin; ri <= rmax; ri++) {
                md[i].info = &m_info[m];
                md[i].s = NULL;
                if (opt.pred_swaplr_flag && 
                       (md[i].info->mmask & M_PRMASK)) {
                    md[i].len_l = ri;
                    md[i].len_r = li;
                } else {
                    md[i].len_l = li;
                    md[i].len_r = ri;
                }
                md[i].w_l = md[i].w_r = 1;
                ++i;
            }
        }
    }

    maxng = (lmax > maxng) ? lmax : maxng;
    maxng = (rmax > maxng) ? rmax : maxng;
    nvotes += pred_votec;

// lex....
    int j;
    seg_lex = 1;

    if(opt.inlex_given){
        L = cg_lexicon_load(opt.inlex_arg);
    } else {
        L = cg_lexicon_new();
    }

    lps = phonstats_new_st(opt.lex_nglen_arg, NULL);
    if (opt.prior_data_given && opt.lex_useprior_flag) {
        phonstats_update_from_file(lps, opt.prior_data_arg);
    }

    for (j = 0; j < opt.lex_mult_arg; j++) {
        nvotes += 2;
        while (nvotes + 2 > mdalloc) {
            md = realloc(md, BUFSIZ);
            mdalloc += BUFSIZ / sizeof(*md);
        }
        md[i].info = &m_info[M_LFB];
        md[i].s = NULL;
        md[i].len_l = -1;
        md[i].len_r = -1;
        md[i].w_l = md[i].w_r = 1;
        md[i].L = L;
        md[i].c = NULL;
        ++i;
        md[i].info = &m_info[M_LFE];
        md[i].s = NULL;
        md[i].len_l = -1;
        md[i].len_r = -1;
        md[i].w_l = md[i].w_r = 1;
        md[i].L = L;
        md[i].c = NULL;
        ++i;
    }

    if (maxng < opt.lex_nglen_arg) 
        maxng = opt.lex_nglen_arg;

    maxng = 1 + 2 * maxng;
    ps = phonstats_new_st(maxng, NULL);

    mv_init();

    if (opt.prior_data_given) {
        phonstats_update_from_file(ps, opt.prior_data_arg);
    }

/*
    for (mi = 0; mi < nvotes; mi++) {
        printf ("%s:%d:%d\n", md[mi].info->sname, md[mi].len_l, md[mi].len_r);
    }
*/
}

unsigned short *
seg_nonlex (char *u)
{
    int len = strlen(u);
    int j = 0;
    int i = 1;
    unsigned short *seg = malloc ((len + 1) * sizeof (*seg));
    double votes[len];
    struct mlist *ml = mlist_new(nvotes);

    seg[0] = 0;
    
    ml->s = u;
    ml->slen = len;
    for (j = 0; j < nvotes; j++) {
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

    mlist_free(ml, 0);

    return seg;

}

struct seglist * 
segment_lexc(struct input *in, int idx)
{
    char *u = in->u[idx].s;
    int len = strlen(u);
    struct seglist *segl = seglist_new();
    unsigned short seg[len + 1];
    unsigned short *lexseg;
    
    int i;

    phonstats_update(ps, u);

    lexseg = lexc_best_seg(L, ps, lps, u);
    

    seg[0] = 0;

    i = 0;
    while (i <= lexseg[0]) {
        int start = (i == 0) ? 0 : lexseg[i];
        int seg_len = (i == lexseg[0]) ? len - start
                                       : lexseg[i + 1] - start;
        char w[seg_len + 1];
        sprintf(w, "%.*s", seg_len, u + start); 
        if (cg_lexicon_lookup(L, w)) {
            if (i != lexseg[0]) {
                ++seg[0];
                seg[seg[0]] = start + seg_len;
            }
        } else {
            unsigned short *sseg = seg_nonlex (w);
            int j = 0;
            while (j <= sseg[0]) {
                int sstart = (j == 0) ? 0 : sseg[j];
                int sseg_len = (j == sseg[0]) ? seg_len - sstart
                                              : sseg[j + 1] - sstart;
                if (i != lexseg[0] || j != sseg[0]) {
                    ++seg[0];
                    seg[seg[0]] = start + sstart + sseg_len;
                }
                j++;
            }
        }
        ++i;
    }

    if (lexseg) free(lexseg);
    seglist_add(segl, seg);

    segment_lexc_update(u, segl);
    return segl;
}

static inline double
freq_score(char *ng) {
    int len = strlen(ng);
    if (len < ps->max_ng) {
        return phonstats_freq_z(ps, ng);
    } else {
        char head[ps->max_ng + 1];
        char tail[ps->max_ng + 1];
        strncpy(head, ng, ps->max_ng); head[ps->max_ng] = '\0';
        strcpy(tail, ng + len - ps->max_ng);
        return (phonstats_freq_z(ps, head) + phonstats_freq_z(ps, tail)) / 2.0;
    }
}

static inline double
ent_score(char *ng) {
    int len = strlen(ng);
    if (len < ps->max_ng) {
        return (cond_entropy(ps, ng, 1)  + cond_entropy_r(ps, ng, 1)) / 2.0;
    } else {
        char head[ps->max_ng + 1];
        char tail[ps->max_ng + 1];
        strncpy(head, ng, ps->max_ng); head[ps->max_ng] = '\0';
        strcpy(tail, ng + len - ps->max_ng);
        return (cond_entropy(ps, tail, 1) + cond_entropy_r(ps, head, 1)) / 2.0;
    }
}

void 
segment_lexc_update(char *s, struct seglist *segl)
{
    char **words = seg_to_strlist(s, segl->segs[0]);
    int i;

//printf("%s\n", s);
    for (i = 0; i <= segl->segs[0][0]; i++) {
//        printf("\t%s: ", words[i]);
        if (cg_lexicon_lookup(L, words[i]) ) {
//            printf("lex\n");
            cg_lexicon_add(L, words[i], "x", NULL);
        } else {
//            printf("nonlex f:%f, e:%f ", freq_score(words[i]), ent_score(words[i]));
            if (freq_score(words[i]) > opt.lex_minfreq_arg 
                && ent_score(words[i]) > opt.lex_minent_arg){
                cg_lexicon_add(L, words[i], "x", NULL);
                phonstats_update(lps, words[i]);
//                printf("ok\n");
            } 
//            else  printf("nok\n");
        }
//        printf("\n");
    }
    free_strlist(words);
}

void 
segment_lexc_cleanup()
{
    cg_lexicon_write(stdout, L);
    return;
}
