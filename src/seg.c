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
/*
#include "seg_nv.h"
#include "seg_random.h"
#include "seg_lexicon.h"
#include "seg_lexc.h"
#include "score.h"
#include "predictability.h"
#include "print.h"
#include "cclib_debug.h"
*/

struct input *inp_dbg; // REMOVE ME

struct gengetopt_args_info opt;

void process_input(struct input *in);

int main(int argc, char **argv)
{
    struct input *inp;

    if (cmdline_parser(argc, argv, &opt) != 0) {
        fprintf(stderr, "Invalid command line argument(s).");
        exit(-1);
    }

//    cclib_debug_init(opt.debug_arg, !opt.quiet_flag, opt.color_flag);

//    assert(opt.print_flag || opt.method_given);

    uint8_t iflags = 0;
    if (opt.ipa_flag) 
        iflags |= INPMODE_IPA;
    if (opt.syl_given) 
        iflags |= INPMODE_SYL;
    if (opt.stress_given) {
        if (opt.stress_file_given) {
            iflags |= INPMODE_STRSEXT;
        } else {
            iflags |= INPMODE_STRSIPA;
            iflags |= INPMODE_IPA;
        }
    }
    inp = input_read(opt.input_arg, 
            opt.stress_file_given ? opt.stress_file_arg : NULL,
            iflags);
    inp_dbg = inp; // REMOVE
    if (opt.shuffle_given) {
       input_shuffle(inp, opt.shuffle_arg); 
    }

    /*
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
*/
        process_input(inp);
/*
    }
*/
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
/*
    size_t prf_off = 0;
    size_t prf_incr = 0;
*/

    switch (opt.method_arg) {
        case method_arg_lm:
            seg_func = segment_lm;
            seg_cleanup_func = segment_lm_cleanup;
            seg_h = segment_lm_init(in, opt.alpha_arg, SEG_PHON);
        break;
/*
        case method_arg_combine:
            seg_func = segment_combine;
            seg_cleanup_func = segment_combine_cleanup;
            segment_combine_init(in);
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
*/
        default:
            assert(opt.print_flag);
        break;
    }

/*
    if (opt.print_prf_arg < 0) {
        prf_incr = -opt.print_prf_arg;
    }
*/

    for (i = 0; i < in->len; i++) {
        out[i] = seg_func(seg_h, i);
        if (opt.progress_given) {
            if((i %  opt.progress_arg) == 0) {
                fprintf(stderr,"%*zu/%zu\r", 6, i, in->len);
            }
        }
    }

/*
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
*/

    seg_cleanup_func(seg_h);

    write_segs(opt.output_arg, out, in);

    for (i = 0; i < in->len; i++) {
        free(out[i]);
    }
    free(out);


/*
    if (opt.outlex_given){
        FILE *lfp = fopen(opt.outlex_arg, "w");
        assert (L != NULL);
        assert (lfp != NULL);
        cg_lexicon_write(lfp, L);
    }
*/
}
