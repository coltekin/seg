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
#include "lexicon.h"
#include "packed_chart.h"

/*
 * cg_combine(cg_cat *catL, cg_cat *catR)
 *
 * combine two cg categories if they do, and return the resulting category
 */
 /*
cg_cat *
cg_combine(cg_cat *catL, cg_cat *catR)
{
    if(catL->slash == '/' && catL->arg == catR) {// FA
        return catL->res;
    }  
    else if(catR->slash == '\\' && catR->arg == catL){// BA
        return catR->res; 
    } else { // no other combinators
        return NULL;
    }
}
*/


/* This is a standard CYK parser. This function  returns only the chart.
 * 
 * conceptually, the chart is a two-dimentional array of the form:
 *
 *   Leical categories --> [0,0][0,1][0,2] ... [0,N-1]
 *                         [1,0]
 *                           .
 *                           .
 *              Result --> [N-1,0]
 *
 *   Each cell in the chart contains a list with a category 
 *   and back links to each sibling for recovering the parses.
 *   
 */

struct chart *
cyk_parse_p(cg_lexicon *l, char **input, size_t N)
{
    unsigned short i, j, k;

    struct chart *chart = chart_new(N);

    /* lexical categories */
    for (j=0; j < N; j++){
        cg_lexilist *ll;
        
        ll = cg_lexicon_lookup(l, input[j]);
        if(ll == NULL){
            fprintf(stderr, "Unknown lexical item: %s\n", input[j]);
            chart_free(chart);
            return NULL;
        } else {
            chart->input[j] = strdup(input[j]);
        }
        while(ll){
            chart_node_add(chart, 0, j, ll->lexi->cat, NULL, NULL);
            ll = ll->next_hom;
        }
    }

    for(i=1; i <= N; i++) {
        for(j=0; j < (N - i ); j++){
            for(k=1; k <= i; k++){
                struct chart_node *nodeL, *nodeR;
                cg_cat *catL, *catR;
                unsigned short iL, iR, jL, jR; // indx for l/r consitituents 

                iL = k - 1; jL = j;
                iR = i - k; jR = j + k;
                
                /* search if any of the L,R pairs combine */
                
                nodeL = chart->node[iL][jL];
                while(nodeL != NULL) {
                    catL = nodeL->cat;
                    nodeR = chart->node[iR][jR];
                    while(nodeR != NULL) {
                        cg_cat  *res;
                        catR = nodeR->cat;
                        res = cg_combine(catL, catR);
                        if(res){
                            chart_node_add(chart, i, j, res, nodeL, nodeR);
                        }
                        nodeR = nodeR->next;
                    }
                    nodeL = nodeL->next;
                }

            }
        }
    }
    return chart;
}
