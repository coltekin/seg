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

/* seg.c -- main routine to process the command line 
 *          and call the appropriate segmentation method with 
 *          given paramethers
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include "lexicon.h"
#include "cmdline.h"
#include "seg.h"
#include "io.h"
#include "seg_lm.h"
#include "seg_pred.h"
#include "seg_ub.h"
#include "seg_nv.h"
#include "seg_random.h"
#include "seg_combine.h"
#include "seg_lexicon.h"
#include "seg_lexc.h"
#include "seglist.h"
#include "score.h"
#include "predictability.h"
#include "print.h"
#include "cclib_debug.h"

void process_input(struct input *in);

int
main(int argc, char **argv)
{
    struct input *I;

    if (cmdline_parser(argc, argv, &opt) != 0) {
        PFATAL("");
    }

    cclib_debug_init(opt.debug_arg, !opt.quiet_flag, opt.color_flag);

    assert(opt.print_flag || opt.method_given);

    I = read_input(opt.input_arg);
    if (opt.shuffle_given) {
       shuffle_input(I); 
    }

    if(opt.print_flag) {
        FILE *fp = stdout;

        if (strcmp("-", opt.output_arg)) {
            fp = fopen(opt.output_arg, "w");
            if(fp == NULL) {
                fprintf(stderr, "cannot open file `%s' for wrting\n", 
                                opt.output_arg);
                exit (1);
            }
        }
        if (opt.print_ptp_given) {
            print_ptp(fp, I);
        } else if (opt.print_wfreq_given){
            print_wfreq(fp, I);
        } else {
            print_pred(fp, I);
        }

    } else {
        process_input(I);
    }
    input_free(I);
    cmdline_parser_free(&opt);
    return 0;
} /* main */


/* process_input()
 * 
 * This is where the main loop over the input is run.
 *
 */
void 
process_input(struct input *in)
{
    struct seglist *(*seg_func)(struct input *, int);
    void (*seg_cleanup_func)();
    struct output *out;
    int i;
    size_t prf_off = 0;
    size_t prf_incr = 0;

    switch (opt.method_arg) {
        case method_arg_combine:
            seg_func = segment_combine;
            seg_cleanup_func = segment_combine_cleanup;
            segment_combine_init(in);
        break;
        case method_arg_lm:
            seg_func = segment_lm;
            seg_cleanup_func = segment_lm_cleanup;
            segment_lm_init(in);
        break;
        case method_arg_pred:
            fprintf(stderr, "Warning `-m pred' is obsolete, use `-m combine' instead.\n");
            seg_func = segment_pred;
            seg_cleanup_func = segment_pred_cleanup;
            segment_pred_init(in);
        break;
        case method_arg_ub:
            fprintf(stderr, "Warning `-m ub' is obsolete, use `-m combine' instead.\n");
            seg_func = segment_ub;
            seg_cleanup_func = segment_ub_cleanup;
            segment_ub_init(in);
        break;
        case method_arg_random:
            seg_func = segment_random;
            seg_cleanup_func = segment_random_cleanup;
            segment_random_init(in);
        break;
        case method_arg_lexicon:
            seg_func = segment_lexicon;
            seg_cleanup_func = segment_lexicon_cleanup;
            segment_lexicon_init(in);
        break;
        case method_arg_nv:
            seg_func = segment_nv;
            seg_cleanup_func = segment_nv_cleanup;
            segment_nv_init(in);
        break;
        case method_arg_lexc:
            seg_func = segment_lexc;
            seg_cleanup_func = segment_lexc_cleanup;
            segment_lexc_init(in);
        break;
        default:
            assert(opt.print_flag);
        break;
    }

    out = output_new(0);
    if (opt.print_prf_arg < 0) {
        prf_incr = opt.print_prf_arg = -opt.print_prf_arg;
    }

    for (i = 0; i < in->size; i++) {
        struct seglist *segl;
        segl = seg_func(in, i);
        output_add(out, in->u[i].s, segl);
        if (opt.progress_given) {
            if((i %  opt.progress_arg) == 0) {
                fprintf(stderr,"%*d/%zu\r", 6, i, in->size);
            }
        }
        if (opt.print_prf_arg && ((i+1) % opt.print_prf_arg) == 0){
            if (i < opt.print_prf_arg) {
                print_prf(in, out, prf_off, opt.print_header_flag);
            } else {
                print_prf(in, out, prf_off, 0);
            }
            prf_off += prf_incr;
        }
    }

    if (opt.print_prf_given) {
        if (opt.print_prf_arg) {
            print_prf(in, out, prf_off, 0);
        } else {
            print_prf(in, out, prf_off, opt.print_header_flag);
        }
    }

    seg_cleanup_func();

    output_write(opt.output_arg, out);
    output_free(out);

/*
    if (opt.outlex_given){
        FILE *lfp = fopen(opt.outlex_arg, "w");
        assert (L != NULL);
        assert (lfp != NULL);
        cg_lexicon_write(lfp, L);
    }
*/
}
