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

#include "seg_TMPLT.h"
#include "seg.h"

void 
segment_TMPLT_init(cg_lexicon *l, struct input *in)
{
    return;
}

struct seglist * 
segment_TMPLT(char *u)
{
    struct seglist *segl = seglist_new();

    return segl;
}

void 
segment_TMPLT_update(char *s, struct seglist *segl)
{
    return;
}

void 
segment_TMPLT_cleanup()
{
    return;
}
