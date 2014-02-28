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

int seg_check (unsigned short *seg, unsigned short pos);
struct seglist *seglist_new();
void seglist_free(struct seglist *segl);
void seglist_add(struct seglist *segl, unsigned short *seg);
inline void print_intarray(unsigned short *a);
void free_strlist(char **s);
char **seg_to_strlist(char *s, unsigned short *seg);
void seglist_add_signed(struct seglist *segl, short *seg);
void seglist_write(FILE *fp, struct seglist *segl);
void seglist_shuffle(struct seglist *segl);
void seglist_print_segs(FILE *fp, struct seglist *segl, char *s);



#endif // _SEGLIST_H

