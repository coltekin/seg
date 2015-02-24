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

#ifndef _SEGLIST_H
#define _SEGLIST_H 1
#include <stdio.h>

struct seglist {    // technically an array, not a list
    int nsegs;
    int nalloc;
    double *score;
    unsigned short **segs;
};


/* seg_check() - check if @pos is in @seg, i.e., if we have a 
 *               boundary at postion @pos.
 */
int seg_check (unsigned short *seg, unsigned short pos);

/* seglist_new() - create a new seglist structure
 */
struct seglist *seglist_new();

/* seglist_new() - destroy @segl created by seglist_new()
 */
void seglist_free(struct seglist *segl);

/* seglist_add() - add a new segmentation @seg to @segl
 */
void seglist_add(struct seglist *segl, unsigned short *seg);

/* print_intarray() - for debugging
 */
inline void print_intarray(unsigned short *a);

void seglist_add_signed(struct seglist *segl, short *seg);

#endif // _SEGLIST_H

