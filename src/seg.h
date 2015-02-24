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

#ifndef _SEG_H
#define _SEG_H 1

enum seg_method {
    SEG_LM = 0
};

enum seg_unit {
    SEG_PHON,
    SEG_SYL
};

struct seg_handle {
    enum seg_method method; // method
    enum seg_unit unit;     // basic unit to use for segmentation
    struct input *in;       // the input structure
    void *options;          // method specific options
};

#endif // _SEG_H

