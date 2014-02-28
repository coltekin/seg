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
#include "ctxlex.h"
#include "seg_combine.h"
#include "seg.h"
#include "ub.h"
#include "pred.h"
#include "mvote.h"
#include "mlist.h"
#include "mdata.h"
#include "lex.h"

static struct phonstats *ps_u = NULL; // phoneme stats over utterances
static struct phonstats *ps_b = NULL; // phoneme stats over utterance boundaries
static struct phonstats *ps_l = NULL; // phoneme stats over lexicon
static struct phonstats *ss_u = NULL; // stress stats over utterances
static struct phonstats *ss_b = NULL; // stress stats over utterance boundaries
static struct phonstats *ss_l = NULL; // stress stats over lexicon
static struct mdlist *mdl = NULL;
static int nvotes = 0;

static short seg_lex = 0;   // or not.

static struct cg_lexicon *lex = NULL;
static struct ctxlex *lex_b = NULL;


#define max_of(x,y) ((x > y) ? x : y)

static inline int 
get_maxng()
{
    int max = 0;

    max = max_of(opt.pred_xlen_arg, max);
    max = max_of(opt.pred_ylen_arg, max);
    max = max_of(opt.pred_xmax_arg, max);
    max = max_of(opt.pred_ymax_arg, max);
    max = max_of(opt.ub_nglen_arg, max);
    max = max_of(opt.ub_ngmax_arg, max);
    max = max_of(opt.sub_ngmax_arg, max);
    max = max_of(opt.ub_lmax_arg, max);
    max = max_of(opt.ub_rmax_arg, max);
    max = max_of(opt.lex_nglen_arg, max);
    return max;
}

void 
segment_combine_init(struct input *in)
{
    int mi = 0;
    int maxng = 1 + 2 * get_maxng();
    int pred_src = (opt.pred_source_given) ? opt.pred_source_arg
                                           : opt.cue_source_arg,
        phon_src = (opt.phon_source_given) ? opt.phon_source_arg
                                           : opt.cue_source_arg,
        stress_src = (opt.stress_source_given) ? opt.stress_source_arg
                                               : opt.cue_source_arg;
//        lex_src = (opt.lex_source_given) ? opt.lex_source_arg
//                                         : opt.cue_source_arg;

    mdl = mdlist_new();

    lex_b = ctxlex_new();
    ps_u = phonstats_new(maxng, NULL);
    ps_b = phonstats_new(maxng, NULL);
    ps_l = phonstats_new(maxng, NULL);

    for (mi = 0; mi < opt.cues_given; mi++) {
        switch (opt.cues_arg[mi]) {
        case cues_arg_phon: {
            switch (phon_src) {
            case pred_source_arg_utterances:
                nvotes += ub_init(mdl, ps_u, M_PUB, M_PUE);
            break;
            case pred_source_arg_segments:
                nvotes += ub_init(mdl, ps_b, M_PUB, M_PUE);
            break;
            case pred_source_arg_lexicon:
                nvotes += ub_init(mdl, ps_l, M_PUB, M_PUE);
            break;
            }
        } break;
        case cues_arg_stress: {
            assert(opt.stress_file_given);
            switch (stress_src) {
            case pred_source_arg_utterances:
                ss_u = phonstats_new(maxng, NULL);
                nvotes += ub_init(mdl, ss_u, M_SUB, M_SUE);
            break;
            case pred_source_arg_segments:
                ss_b = phonstats_new(maxng, NULL);
                nvotes += ub_init(mdl, ss_b, M_SUB, M_SUE);
            break;
            case pred_source_arg_lexicon:
                ss_l = phonstats_new(maxng, NULL);
                nvotes += ub_init(mdl, ss_l, M_SUB, M_SUE);
            break;
            }
        } break;
        case cues_arg_pred: {
            switch (pred_src) {
            case pred_source_arg_utterances:
                nvotes += pred_init(mdl, ps_u);
            break;
            case pred_source_arg_segments:
                nvotes += pred_init(mdl, ps_b);
            break;
            case pred_source_arg_lexicon:
                nvotes += pred_init(mdl, ps_b);
            break;
            }
        } break;
        case cues_arg_lex: {
            if (ps_l == NULL) ps_l = phonstats_new(maxng, NULL);
            if(opt.inlex_given){
                lex = cg_lexicon_load(opt.inlex_arg);
            } else {
                lex = cg_lexicon_new();
            }
            nvotes += lex_init(mdl, lex, ps_l, lex_b);
            seg_lex = 1;
        } break;
        default:
            fprintf(stderr, "I don not know how to combine method `%d'.\n", 
                     opt.cues_arg[mi]);
            exit(-1);
        break;
        }
    }


    mv_init();

    if (opt.prior_data_given) {
        phonstats_update_from_file(ps_u, opt.prior_data_arg);
        phonstats_copy(ps_b, ps_u);
    }

/*
    for (mi = 0; mi < nvotes; mi++) {
        printf ("%s:%d:%d,", md[mi].info->sname, md[mi].len_l, md[mi].len_r);
    }
    printf("\n");
*/
}

struct seglist * 
segment_combine(struct input *in, int idx)
{
    char *u = in->u[idx].s;
    char *stress = (in->stress) ? in->stress[idx] : NULL;
    struct seglist *segl = seglist_new();
    int len = strlen(u);
    int j = 0;
    int i = 1;
    unsigned short seg[len + 1];
    double votes[len];
    struct mlist *ml = mlist_new(nvotes);

    seg[0] = 0;
    
    if (ps_u) phonstats_update(ps_u, u);
    if (ss_u) phonstats_update(ss_u, stress);
    if (opt.psb_cheat_flag && ps_b) phonstats_update(ps_b, u);

    ml->s = u;
    ml->slen = len;
    for (j = 0; j < nvotes; j++) {
        if(mdl->md[j]->info->mid == M_SUB || mdl->md[j]->info->mid == M_SUE)
            mdl->md[j]->s = stress;
        else
            mdl->md[j]->s = u;
// printf("%s:%d:%d:\n", md[j].info->sname, md[j].len_l, md[j].len_r);
        mlist_add(ml, mdl->md[j]);
//        printf("%s... ", md[j].info->sname);
//        print_pred_list(u, ml->mlist[j]);
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
    mlist_free(ml, 0);

    segment_combine_update(u, stress, segl);
    return segl;
}

void 
segment_combine_update(char *s, char *stress, struct seglist *segl)
{
    char **words = seg_to_strlist(s, segl->segs[0]);
    char **wstress = (stress) ? seg_to_strlist(stress, segl->segs[0]) : NULL;
    int i;

    for (i = 0; i <= segl->segs[0][0]; i++) {
        if (ps_b) {
            phonstats_update(ps_b, words[i]);
        }
        if (ss_b) phonstats_update(ss_b, wstress[i]);
        struct lexdata *ld = ctxlex_add(lex_b, words[i], 
                             (i == 0) ? "<" : words[i - 1],
                             (i == segl->segs[0][0]) ? ">" : words[i + 1]);
        if (ld->freq == 1) {
            if (ps_l) phonstats_update(ps_l, words[i]);
            if (ss_l) phonstats_update(ss_l, wstress[i]);
        }

        if (seg_lex) {
            if (phonstats_freq_ng(ps_u, words[i]) > opt.lex_minfreq_arg 
              &&cond_entropy(ps_u, words[i], 1) > opt.lex_minent_arg
              &&cond_entropy_r(ps_u, words[i], 1) > opt.lex_minent_arg) {
                cg_lexicon_add(lex, words[i], "x", NULL);
            }
        }
    }

    free_strlist(words);
}

void 
segment_combine_cleanup()
{
    if (ps_u) phonstats_free(ps_u);
    if (ps_b) phonstats_free(ps_b);
    if (ps_l) phonstats_free(ps_l);
    if (ss_u) phonstats_free(ss_u);
    if (ss_b) phonstats_free(ss_b);
    if (ss_l) phonstats_free(ss_l);
    if (lex) cg_lexicon_free(lex);
    if (lex_b) ctxlex_free(lex_b);
    mdlist_free(mdl);
}
