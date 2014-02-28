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

#include "lex.h"
#include "segparse.h"
#include "ctxlex.h"
#include <assert.h>
#include <math.h>
#include "ub.h"

static inline void
chart_word(char *w, struct chart *c, short pos, short end)
{
    int k;
    strcpy(w, "");
    for(k=pos; k <= end; k++) {
        strcat(w, c->input[k]);
    }
}

double 
word_score(char *s, struct ctxlex *cL, enum m_id mid) 
{
    double sc;
    
    if (mid == M_LFB || mid == M_LFE) {
        if (opt.lex_norm_arg == lex_norm_arg_none) {
            sc = (double) ctxlex_freq(cL, s);
        } else {
            sc = ctxlex_freq_z(cL, s);
        }
    }else {
        if (opt.lex_norm_arg == lex_norm_arg_none) {
            sc = (double) ctxlex_nctx(cL, s);
        } else {
            sc = ctxlex_nctx_z(cL, s);
        }
    }

    return sc;
}

double 
score_words_before(cg_lexicon *L, struct chart *c, 
                   struct ctxlex *cL, 
                   enum m_id mid, 
                   short pos)
{
    int i, j;
    double best = -INFINITY;
    double sum = 0.0;
    int count = 0;
    assert (c != NULL);
    assert (pos > 0);

    i = pos - 1;
    for (j = 0; j < pos; j++) {
        struct chart_node *node = c->node[i][j];
        
        while (node != NULL) {
            if (node->back == NULL) {
                char w[pos - j + 1];
                chart_word(w, c, j, pos - 1);
                double sc = word_score(w, cL, mid);
                sum += sc;
                if (sc > best) best = sc;
                count++;
                break;
            }
            node = node->next;
        }
        --i;
    }
    switch (opt.lex_wcombine_arg) {
    case lex_wcombine_arg_best:
        return best;
    break;
    case lex_wcombine_arg_sum:
        return sum;
    break;
    case lex_wcombine_arg_mean:
        return sum / (double) count;
    break;
    default:
        assert(!"this should not happen");
        return 0.0;
    break;
    }
}

double
score_words_after(cg_lexicon *L, struct chart *c, 
                   struct ctxlex *cL, 
                   enum m_id mid, 
                   short pos)
{
    int i, count = 0;
    double best = -INFINITY;
    double sum = 0.0;
    assert (c != NULL);
    assert (pos > 0);

    for (i = c->size - pos - 1; i > 0; i--) {
        struct chart_node *node = c->node[i][pos];
        
        while (node != NULL) {
            if (node->back == NULL) {
                char w[i + 1];
                chart_word(w, c, pos, pos + i);
                double sc = word_score(w, cL, mid);
                sum += sc;
                if (sc > best) best = sc;
                count++;
                break;
            }
            node = node->next;
        }
    }
    switch (opt.lex_wcombine_arg) {
    case lex_wcombine_arg_best:
        return best;
    break;
    case lex_wcombine_arg_sum:
        return sum;
    break;
    case lex_wcombine_arg_mean:
        return sum / (double) count;
    break;
    default:
        assert(!"this should not happen");
        return 0.0;
    break;
    }
}

int
words_before(cg_lexicon *L, struct chart *c, short pos, short freq)
{
    int i, j;
    int count = 0;
    assert (c != NULL);
    assert (pos > 0);

    i = pos - 1;
    for (j = 0; j < pos; j++) {
        struct chart_node *node = c->node[i][j];
        
        while (node != NULL) {
            if (node->back == NULL) {
                int delta = 1;
                if(freq) {
                    char w[pos - j + 1];
                    chart_word(w, c, j, pos - 1);
                    delta = cg_lexicon_get_freq_pf(L, w);
                }
                count += delta;
                break;
            }
            node = node->next;
        }
        --i;
    }

    return count;
}

int
words_after(cg_lexicon *L, struct chart *c, short pos, short freq)
{
    int i, count = 0;
    assert (c != NULL);
    assert (pos > 0);

    for (i = c->size - pos - 1; i > 0; i--) {
        struct chart_node *node = c->node[i][pos];
        
        while (node != NULL) {
            if (node->back == NULL) {
                int delta = 1;
                if(freq) {
                    char w[i + 1];
                    chart_word(w, c, pos, pos + i);
                    delta = cg_lexicon_get_freq_pf(L, w);
                }
                count += delta;
                break;
            }
            node = node->next;
        }
    }

    return count;
}

char *
best_word_before(cg_lexicon *L, struct chart *c, short pos)
{
    int i, j;
    char *ret = NULL;
    int max_freq = 0;
    assert (c != NULL);
    assert (pos > 0);

    i = pos - 1;
    for (j = 0; j < pos; j++) {
        struct chart_node *node = c->node[i][j];
        int found = 0;
        
        while (node != NULL) {
            if (node->back == NULL) {
                found = 1;
                break;
            }
            node = node->next;
        }
        if (found) {
            int k;
            int freq;
            char word[pos - j + 1];
            strcpy(word, "");
            for(k=j; k < pos; k++) {
                strcat(word, c->input[k]);
            }
            freq = cg_lexicon_get_freq_pf(L, word);
            if (freq > max_freq) {
                max_freq = freq;
                if (ret != NULL) free(ret);
                ret = strdup(word);
            }
        }
        --i;
    }
    return ret;
}

char *
best_word_after(cg_lexicon *L, struct chart *c, short pos)
{
    int i;
    char *ret = NULL;
    int max_freq = 0;
    assert (c != NULL);
    assert (pos > 0);

// printf("--- bwa: pos %d: ", pos);
    for (i = c->size - pos - 1; i > 0; i--) {
        struct chart_node *node = c->node[i][pos];
        int found = 0;
        
        while (node != NULL) {
            if (node->back == NULL) {
                found = 1;
                break;
            }
            node = node->next;
        }
        if (found) {
            int k;
            char word[i + 1];
            int freq;
            strcpy(word, "");
            for(k=pos; k <= pos + i; k++) {
                strcat(word, c->input[k]);
            }
            freq = cg_lexicon_get_freq_pf(L, word);
            if (freq > max_freq) {
                max_freq = freq;
                if (ret != NULL) free(ret);
                ret = strdup(word);
            }
        }
    }
    return ret;
}



// TODO: avoid multiple lexicon lookups.
static inline double 
_calc_lex_single(struct phonstats *ps, struct mdata *m, int pos, int len)
{
    double val;
    
    assert(pos <= len);
    assert(m->info->mid == M_LFB || 
           m->info->mid == M_LFE ||
           m->info->mid == M_LCB ||
           m->info->mid == M_LCE );
    assert(m->L != NULL);
    assert(m->c != NULL);

    switch (m->info->mid) {
    case M_LFB:
    case M_LCB:
        val = score_words_before(m->L, m->c, m->cL, m->info->mid, pos);
    break;
    case M_LFE:
    case M_LCE:
        val = score_words_after(m->L, m->c, m->cL, m->info->mid, pos);
    break;
    default: 
        assert(!"we should not be here!");
    break;
    }


    return val;
}

double 
calc_lex_single(struct phonstats *ps, struct mdata *m, int pos)
{
    int len = strlen(m->s);
    return  _calc_lex_single(ps, m, pos, len);
}


double *
calc_lex_list(struct phonstats *ps, struct mdata *m)
{
    int len = strlen(m->s);
    int j = 0;
    double *lexl = NULL;
    short  chart_alloc = 0;

    assert(m->L != NULL);

    if(m->c == NULL) {
        m->c = seg_parse(m->L, m->s, seg_combine);
        chart_alloc = 1;
    }

    lexl = malloc((len + 1) * sizeof (*lexl));

    lexl[0] = 0.0;
    for (j = 1; j < len; j++) {
        lexl[j] = _calc_lex_single(ps, m, j, len);
    }
    lexl[len] = 0.0;

    if (chart_alloc) {
        chart_free(m->c);
        m->c = NULL;
    }

//    print_pred_list(m->s, lexl);
    return lexl;
}

static inline int
lex_add_measure(struct mdlist *mdl, enum m_id mid, struct cg_lexicon *L, struct phonstats *ps, struct ctxlex *cL)
{
    struct mdata *md = mdata_new_full(mid, NULL, -1, -1);
    md->L = L;
    md->ps = ps;
    md->cL = cL;
    mdlist_add(mdl, md);
    return 1;
}

int
lex_init(struct mdlist *mdl, struct cg_lexicon *L, struct phonstats *ps, struct ctxlex *cL)
{
    int j;
    int votec = 0;
    int lr = (opt.lex_dir_arg == lex_dir_arg_both ||
              opt.lex_dir_arg == lex_dir_arg_lr);
    int rl = (opt.lex_dir_arg == lex_dir_arg_both || 
              opt.lex_dir_arg == lex_dir_arg_rl);
    int mcount = (opt.lex_given) ? opt.lex_given : 1;



    for (j = 0; j < opt.lex_mult_arg; j++) {
        int mi;
        for(mi = 0; mi < mcount; mi++) {
            switch (opt.lex_arg[mi]) {
            case lex_arg_lf:
                if(lr) 
                    votec += lex_add_measure(mdl, M_LFE, L, NULL, cL);
                if(rl)
                    votec += lex_add_measure(mdl, M_LFB, L, NULL, cL);
            break;
            case lex_arg_lc:
                if(lr) 
                    votec += lex_add_measure(mdl, M_LCE, L, NULL, cL);
                if(rl)
                    votec += lex_add_measure(mdl, M_LCB, L, NULL, cL);
            break;
            case lex_arg_lp:
                votec += ub_init(mdl, ps, M_LPE, M_LPB);
            break;
            default:
                printf("%d: %d\n", mi, opt.lex_arg[mi]);
                assert(!"we should not be here!");
            }
        }
    }

    return votec;
}

void
print_words_before(cg_lexicon *L, struct chart *c, short pos)
{
    int i, j;
    assert (c != NULL);
    assert (pos > 0);

    i = pos - 1;
    for (j = 0; j < pos; j++) {
        struct chart_node *node = c->node[i][j];
        int found = 0;
        
        while (node != NULL) {
            if (node->back == NULL) {
                found = 1;
                break;
            }
            node = node->next;
        }
        if (found) {
//            int k;
            char word[pos - j + 1];
//            strcpy(word, "");
//            printf("wb[%d]: ", pos);
chart_word(word, c, j, pos - 1);
//            for(k=j; k < pos; k++) {
//                strcat(word, c->input[k]);
//            }
            printf("%s :: %zu\n", word, cg_lexicon_get_freq_pf(L, word));
        }
        --i;
    }
}

void
print_words_after(cg_lexicon *L, struct chart *c, short pos)
{
    int i;
    assert (c != NULL);
    assert (pos > 0);

    for (i = c->size - pos - 1; i > 0; i--) {
        struct chart_node *node = c->node[i][pos];
        int found = 0;
        
        while (node != NULL) {
            if (node->back == NULL) {
                found = 1;
                break;
            }
            node = node->next;
        }
        if (found) {
//            int k;
            char word[i + 1];
//            strcpy(word, "");
//            printf("wa[%d]:", pos);
//            for(k=pos; k <= pos + i; k++) {
//                strcat(word, c->input[k]);
//            }
chart_word(word, c, pos, pos + i);
            printf("%s :: %zu\n", word, cg_lexicon_get_freq_pf(L, word));
        }
    }
}

