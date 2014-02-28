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
#include "strutils.h"
#include "cyk.h"

cyk_chart *
cyk_chart_new(int size)
{
    cyk_chart   *ret;
    int         i, j;

    ret = malloc(sizeof (cyk_chart));
    ret->node = malloc (size * sizeof (cyk_chart_node *));
    ret->input = malloc (size * sizeof (char *));
    ret->size = size;
    for(i = 0; i < size; i++) { 
        ret->node[i] = malloc ((size - i) * sizeof (cyk_chart_node));
        for(j=0; j < size - i; j++){
            ret->node[i][j] = NULL;
        }
        ret->input[i] = NULL;
    }
    return ret;
}

void 
cyk_chart_free(cyk_chart *chart)
{
    int i, j;

    if(chart == NULL) 
        return;

    for(i=0; i < chart->size; i++) {
        for(j=0; j < chart->size - i; j++) {
            cyk_chart_node *tmp = chart->node[i][j]; 
            while(tmp) {
                cyk_chart_node *tmp2 = tmp;
                tmp = tmp->next;
                free(tmp2);
            }
        }
        free(chart->node[i]);
        if(chart->input[i]) free(chart->input[i]);
    }
    free(chart->input);
    free(chart->node);
    free(chart);
    
}

cyk_chart_node *
cyk_node_new(cg_cat *cat, 
             cyk_chart_node *bL, 
             cyk_chart_node *bR, 
             cyk_chart_node *next, 
             int i, int j)
{
    cyk_chart_node *ret = malloc(sizeof(cyk_chart_node));
    ret->cat = cat;
    ret->backL = bL;
    ret->backR  = bR;
    ret->next = next;
    ret->i = i;
    ret->j = j;
    return ret;
}

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


/* This is a standard CYK parser. This function  returns only the chart.
 * 
 * conceptually, the cart a two-dimentional array of the form:
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

cyk_chart *cyk_parse(cg_lexicon *l, char **input, size_t N)
{
    int i, j, k;

    cyk_chart *chart = cyk_chart_new(N);

    /* lexical categories */
    for (j=0; j < N; j++){
        cg_lexilist *ll;
        
        ll = cg_lexicon_lookup(l, input[j]);
        if(ll == NULL){
            fprintf(stderr, "Unknown lexical item: %s\n", input[j]);
            cyk_chart_free(chart);
            return NULL;
        } else {
            chart->input[j] = strdup(input[j]);
        }
        while(ll){
            chart->node[0][j] = cyk_node_new(ll->lexi->cat, 
                                             NULL, NULL, chart->node[0][j],
                                             0, j);
            ll = ll->next_hom;
        }
    }

    for(i=1; i <= N; i++) {
        for(j=0; j < (N - i ); j++){
            for(k=1; k <= i; k++){
                cyk_chart_node *nodeL, *nodeR;
                cg_cat *catL, *catR;
                int iL, iR, jL, jR; // indices for left/right consitituents 

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
                            chart->node[i][j] = cyk_node_new(res, 
                                                       nodeL, nodeR, 
                                                       chart->node[i][j],
                                                       i, j);
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


void 
cyk_chart_write(FILE *fp, cyk_chart *chart)
{
    if (chart == NULL) {
       fprintf(stderr,"Chart is NULL\n");   
       return;
    }

    int i, j;
    int max_len = 0;
    char *str;
    char *cellstr[chart->size][chart->size];

    for(i=0; i < chart->size; i++){
        int len = strlen(chart->input[i]);
        if(len > max_len) max_len = len;
    }
    for(i=0; i < chart->size; i++){
        for(j=0; j < chart->size - i; j++) {
            cyk_chart_node *node = chart->node[i][j];
            if (node) {
                size_t len;
                str = str_astrcat(&str, node->cat->str);
                node = node->next;
                while(node) {
                    str = str_astrcat(NULL, ",");
                    str = str_astrcat(NULL, node->cat->str);
                    node = node->next;
                }
                len = strlen(str);
                if(len > max_len) max_len = len;
                cellstr[i][j] = str;
            } else {
                cellstr[i][j] = NULL;
            }
        }
    }

    fprintf(fp, "------ Chart: \n");
    for(i=0; i < chart->size; i++){
        fprintf(fp, "%-*s |", max_len, chart->input[i]);
    }
    fprintf(fp, "\n");

    for(i=0; i < chart->size; i++){
        for(j=0; j < chart->size - i; j++) {
            char *s = (cellstr[i][j]) ? cellstr[i][j] : "";
            fprintf(fp, "%-*s |", max_len, s);
            if(cellstr[i][j]) free(cellstr[i][j]);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "------ End of chart\n");
}


void
cyk_write_parse(FILE *fp, cyk_chart_node *node, cyk_chart *chart)
{

    if(node->backR == NULL) {
        fprintf(fp, "%s -> %s\n", node->cat->str, 
                                  chart->input[node->j]);
        return;
    }
    if(node->backL == NULL) // this should not happen
        return;
    fprintf(fp, "%s -> %s %s\n", node->cat->str, 
                                 node->backL->cat->str,
                                 node->backR->cat->str);
    cyk_write_parse(fp, node->backL, chart);
    cyk_write_parse(fp, node->backR, chart);
}

void 
cyk_write_parses(FILE *fp, cyk_chart *chart)
{
    if (chart == NULL) {
       fprintf(stderr,"Chart is NULL\n");   
       return;
    }

    if(!chart->node[chart->size-1][0]) {
       fprintf(stderr,"No parses\n");   
       return;
    }

    cyk_chart_node *tmp = chart->node[chart->size-1][0];
    int             i=1;
    while(tmp) {
        fprintf(fp, "------------ Parse #%d:\n", i);
        cyk_write_parse(fp, tmp, chart);
        tmp = tmp->next;
        fprintf(fp, "------------ End of Parse #%d:\n", i);
        i++;
    }

}
