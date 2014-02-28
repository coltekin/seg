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

#ifndef _PEAK_H
#define _PEAK_H 1
#include "options.h"
#include "mlist.h"
#include "mdata.h"

double get_vote_peak(double prev, double curr, double next, 
                     int peak_type, struct mdata *md);

int get_votes_peak(double ***vote_l, double ***vote_r, struct mlist *ml);


#endif // _PEAK_H

#include <math.h>
#define SIGN(x) ( ((x) > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0) )
// #define SIGN(x) ( ((x) > 0.0) ? 1.0 :  -1.0 )
#define LOGISTIC(x) ( 1.0 / (1.0 + exp(-(x))) )

