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
#include <string.h>
#include <glib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "cgparse/lexicon.h"
#include "cgparse/wparse.h"
#include "io.h"

float get_mem_usage()
{
    FILE *fp;
    unsigned size;
    
    fp = fopen("/proc/self/statm", "r");
    if (fp) {
        fscanf(fp, "%u", &size);
    }
    fclose(fp); 
    return size / 1024.0;
}

void
segment_lex_simple(char *inf, char *inlf, char *outlf)
{
    struct mc_input *inp = mc_read_input(inf);
    GHashTableIter iter;
    struct seglist *segs;
    char *w;
    int  *freq;
    cg_lexicon *L = cg_lexicon_new();
    int     c=0;
    struct timeval t1, t2;
    float  mem1, mem2;


    g_hash_table_iter_init (&iter, inp->whash);
    while (g_hash_table_iter_next (&iter, (gpointer *)&w, (gpointer *)&freq)) {

        ++c;
        fprintf(stderr, "[%d] %s: ", c, w);

        gettimeofday(&t1, NULL);
        mem1 = get_mem_usage();

        cyk_chart *chart = wsegment(L, w);

        gettimeofday(&t2, NULL);
        mem2 = get_mem_usage();

        fprintf(stderr, "%.3f/%.3f ", 
                (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)/1000000.0,
                mem2 - mem1);

        segs = wparse_get_seglist(chart);

        gettimeofday(&t1, NULL);
        mem1 = get_mem_usage();
        fprintf(stderr, "%.3f/%.3f\n", 
                (t1.tv_sec - t2.tv_sec) + (t1.tv_usec - t2.tv_usec) / 1000000.0,
                mem1 - mem2);

        cg_lexicon_add(L, w, "X", NULL);
        cyk_chart_free(chart);
        if (segs) seglist_free(segs);
        if((c % 100) == 0) {
            fprintf(stderr, "%-8d/%d\r", c, inp->size);
        }
    }
    mc_input_free(inp);
    cg_lexicon_free(L);
}


int
main(int argc, char **argv)
{
    segment_lex_simple(argv[1], NULL, NULL);
    return 0;
}
