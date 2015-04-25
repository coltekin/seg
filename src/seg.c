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
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include "lexicon.h"
#include "cmdline.h"
#include "input.h"
#include "lm.h"
#include "score.h"
/*
#include "seg_nv.h"
#include "seg_random.h"
#include "seg_lexicon.h"
#include "seg_lexc.h"
#include "predictability.h"
#include "print.h"
#include "cclib_debug.h"
*/

struct gengetopt_args_info opt;

void process_input(struct input *in);

int main(int argc, char **argv)
{
    struct input *inp;

    if (cmdline_parser(argc, argv, &opt) != 0) {
        fprintf(stderr, "Invalid command line argument(s).");
        exit(-1);
    }

    uint8_t iflags = 0;
    if (opt.ipa_flag) 
        iflags |= INPMODE_IPA;
    if (opt.syl_given) 
        iflags |= INPMODE_SYL;
    if (opt.stress_given) {
        iflags |= INPMODE_STRSEXT;
    }
    inp = input_read(opt.input_arg, 
            opt.stress_given ? opt.stress_arg : NULL,
            iflags);
    if (opt.shuffle_given) {
       input_shuffle(inp, opt.shuffle_arg); 
    }
    process_input(inp);
    input_free(inp);
    cmdline_parser_free(&opt);
    return 0;
} /* main */


/* process_input()
 * 
 * This is where the main loop over the input is run.
 *
 */
void process_input(struct input *in)
{
    struct segmentation *(*seg_func)(struct seg_handle *, size_t);
    void (*seg_cleanup_func)(struct seg_handle *);
    struct segmentation **out = malloc(in->len * sizeof *out);
    size_t i;
    struct seg_handle *seg_h;
    size_t prf_off = 0;
    size_t prf_incr = 0;
    uint8_t print_prf_opts = 0;
    enum seg_unit unit = SEG_PHON;

    if (opt.print_header_flag)
        print_prf_opts |= SCORE_OPT_HEADER;
    if (opt.print_latex_flag) 
        print_prf_opts |= SCORE_OPT_LATEX;
    if (opt.score_edges_flag) 
        print_prf_opts |= SCORE_EDGES;

    if (opt.syl_given) unit = SEG_SYL;

    switch (opt.method_arg) {
        case method_arg_lm:
            seg_func = segment_lm;
            seg_cleanup_func = segment_lm_cleanup;
            seg_h = segment_lm_init(in, opt.alpha_arg, unit);
        break;
        default:
            fprintf(stderr, "You did not tell me what to do.\n");
            exit(-1);
        break;
    }


    if (opt.print_prf_arg < 0) {
        prf_incr = -opt.print_prf_arg;
    }

    for (i = 0; i < in->len; i++) {

        out[i] = seg_func(seg_h, i);

        if (unit == SEG_SYL && out[i]) { // convert syllables to phones
            size_t j;
            for (j = 0; j < out[i]->len; j++) {
                out[i]->bound[j] = in->u[i]->syl_seg->bound[out[i]->bound[j] - 1];
            }
        }

        /* print the progress if requested */
        if (opt.progress_given) {
            if((i %  opt.progress_arg) == 0) {
                fprintf(stderr,"%*zu/%zu\r", 6, i, in->len);
            }
        }

        /* print the evaluation measures, if requested */
        if (opt.print_prf_arg && ((i+1) % prf_incr) == 0){
            if (i < prf_incr) {
                print_prf(in, out, prf_off, i, print_prf_opts);
            } else {
                print_prf(in, out, prf_off, i,
                    print_prf_opts & ~SCORE_OPT_HEADER);
            }
            prf_off += prf_incr;
        }
    }

    if (opt.print_prf_given) {
        if (opt.print_prf_arg) {
            print_prf(in, out, prf_off, in->len, 
                    print_prf_opts & ~SCORE_OPT_HEADER);
        } else {
            print_prf(in, out, prf_off, in->len, print_prf_opts);
        }
    }

    seg_cleanup_func(seg_h);

    write_segs(opt.output_arg, out, in);

    for (i = 0; i < in->len; i++) {
        if (out[i] != NULL) {
            if (out[i]->bound != NULL) free(out[i]->bound);
            free(out[i]);
        }
    }
    free(out);
}
