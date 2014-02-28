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

#ifndef _IO_H
#define _IO_H 1

#include <stdlib.h>
#include "seglist.h"

struct input_rec {
    char            *s;    // the input string, without delimeters
    unsigned short  *seg;  // offsets to each segment seg[0] is the number
};                         // of segments, seg[n] is offset to nth seg.

struct input {
    size_t              size;
    size_t              nalloc;  // for memory management
    struct input_rec    *u;
    char                **stress; //stress pattern. has to match with u.s, can be NULL
};

struct output_rec {
    char            *s;     // the string, without delimeters
    struct seglist  *segl;
};

struct output {
    size_t              size;
    size_t              nalloc; // for memory management
    struct output_rec   *u;
};

struct input *read_input(char *infile);
void input_free(struct input *inp);


void output_write(char *outfile, struct output *O);
void output_add(struct output *out, char *s, struct seglist *segs);
struct output * output_new(size_t len);
void output_free(struct output *out);
void shuffle_input(struct input *in);

#endif // _IO_H

