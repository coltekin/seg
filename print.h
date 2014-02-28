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

#ifndef _PRINT_H
#define _PRINT_H 1
#include "options.h"
#include "io.h"

void print_pred(FILE *fp, struct input *in); 
void print_ptp(FILE *fp, struct input *in); 
void print_wfreq(FILE *fp, struct input *in); 

void pprint_pred(struct input *in, 
                 enum pred_measure m, 
                 int x_len, int y_len, 
                 int prec);

void print_pred_list(char *s, double *mlist);

#endif // _PRINT_H
