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

#ifndef _SEG_NV_H
#define _SEG_NV_H 1
#include "seg.h"
#include "seglist.h"
#include "io.h"
#include "lexicon.h"

void segment_nv_init(struct input *in);
struct seglist *segment_nv(struct input *in, int i);
void segment_nv_update(char *s, struct seglist *segl);
void segment_nv_cleanup();



#endif // _SEG_NV_H
