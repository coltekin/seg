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
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include "seg.h"
#include "seglist.h"
#include "io.h"
#include "cclib_debug.h"
#include "strutils.h"

#define COMMENT_CHAR ';'
#define MAX_LINE_LEN 256

/* struct input *read_input(char *inf)
 *      read input file with segmented input. the file is 
 *      assumed to have one utterance per line, and delimted 
 *      with space.
 */

struct input *
read_input(char *inf)
{
    FILE            *fp = NULL, *sfp = NULL;
    struct input    *ret;
//    char            linebuf[MAX_LINE_LEN];

    if(!strcmp("-", inf)) {
        fp = stdin;
    } else {
        fp = fopen(inf, "r");
        if(fp == NULL)  {
            PFATAL("cannot open `%s' for reading\n", inf);
        }
    }
    
    if(opt.stress_file_given) {
        sfp = fopen(opt.stress_file_arg, "r");
        if(fp == NULL)  {
            PFATAL("cannot open `%s' for reading\n", inf);
        }
    }

    PINFO("reading file `%s'...\n", inf);
    ret = malloc(sizeof(struct input));
    ret->size = 0;
    ret->nalloc = 0;
    ret->u = NULL;
    ret->stress = NULL;

    while (!feof(fp)) {
       char *linebuf = NULL;
       char *slinebuf = NULL;

        fscanf(fp, "%m[^\n]\n", &linebuf);
        if (opt.stress_file_given) {
            fscanf(sfp, "%m[^\n]\n", &slinebuf);
        }

        if(*linebuf == COMMENT_CHAR) {
            free(linebuf);
            if (opt.stress_file_given) {
                free(slinebuf);
            }
            continue;
        }

        if(ret->size  >= ret->nalloc ) {
            struct input_rec    *tmp;
            ret->nalloc += BUFSIZ;
            tmp = realloc(ret->u, ret->nalloc * sizeof (struct input_rec));
            if(tmp) {
                ret->u = tmp;
            } else {
                PFATAL("unable to allocate memory\n");
            }
            if (opt.stress_file_given) {
                char **stmp;
                stmp = realloc(ret->stress, ret->nalloc * sizeof (*stmp));
                if(stmp) {
                    ret->stress = stmp;
                } else {
                    PFATAL("unable to allocate memory\n");
                }
            }
        }

        str_rmch(linebuf, ' ', &ret->u[ret->size].seg);
        str_strip(linebuf, " \t\n");
        ret->u[ret->size].s = strdup(linebuf);
        if (opt.stress_file_given) {
            str_rmch(slinebuf, ' ', NULL);
            str_strip(slinebuf, " \t\n");
            assert(strlen(linebuf) == strlen(slinebuf));
            ret->stress[ret->size] = strdup(slinebuf);
            free(slinebuf);
        }
        ret->size++;
        free(linebuf);
    }

    fclose(fp);
    if(sfp) fclose(sfp);
    PINFO("done reading file `%s' (%zu lines).\n", inf, ret->size);
    return ret;
}


/* Shuffle the given input list */
void shuffle_input(struct input *in)
{
    int i;
    if (in->size < 1) {
        return;
    }

    if (opt.shuffle_arg == -1) {
        srand(time(NULL));
    } else {
        srand(opt.shuffle_arg);
    }

    for (i = 0; i < in->size - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (in->size - i) + 1);
        struct input_rec t;
        memcpy(&t, &in->u[j], sizeof t);
        memcpy(&in->u[j], &in->u[i], sizeof t);
        memcpy(&in->u[i], &t, sizeof t);
    }
}

void 
input_free(struct input *inp)
{
    int i;

    for(i = 0; i < inp->size; i++) {
        free(inp->u[i].s);
        if(inp->u[i].seg)
            free(inp->u[i].seg);
        if(inp->stress && inp->stress[i]){
            free(inp->stress[i]);
        }
    }
    free(inp->u);
    if (inp->stress) free(inp->stress);
    free(inp);
}


struct output *
output_new(size_t len)
{
    struct output *ret = malloc(sizeof *ret);

    if (len != 0) {
        ret->size = 0;
        ret->nalloc = len * sizeof (struct output_rec);
        ret->u = malloc (ret->nalloc);
        assert (ret->u != NULL);
    } else {
        ret->size = 0;
        ret->nalloc = 0;
        ret->u = NULL;
    }

    return ret;
}


void 
output_free(struct output *out)
{
    if (out->u != NULL) {
        int i;
        for (i = 0; i < out->size; i++) {
            seglist_free(out->u[i].segl);
            free(out->u[i].s);
        }
        free (out->u);
    }
    free(out);
}

void 
output_add(struct output *out, char *s, struct seglist *segs)
{
    int   n = out->size + 1;

    if(n * sizeof (struct output_rec) >= out->nalloc) {
        struct output_rec    *tmp;
        out->nalloc += BUFSIZ;
        tmp = realloc(out->u, out->nalloc);
        if(tmp)
            out->u = tmp;
        else
            PFATAL("unable to allocate memory\n");
    }

    out->u[out->size].s = strdup(s);
    out->u[out->size].segl = segs;
    ++out->size;
}

void 
output_write(char *outf, struct output *out)
{
    FILE *fp;
    int     i = 0;
    if(!strcmp("-", outf)) {
        fp = stdout;
PINFO("ourput_write: stdout\n");
    } else {
        fp = fopen(outf, "w");
        if(fp == NULL)  {
            PFATAL("cannot open `%s' for reading\n", outf);
        }
PINFO("ourput_write: %s\n", outf);
    }

PINFO("ourput_write: 0..%zd\n", out->size);
    for (i = 0; i < out->size; i++) {
//        struct output_rec *o = &out->u[i]; // just a shorthand
        seglist_print_segs(fp, out->u[i].segl, out->u[i].s);
    }

    if(fp != stdout) {
        fclose(fp);
    }
}


