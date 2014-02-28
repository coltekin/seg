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

#ifndef _MEASURES_H
#define _MEASURES_H 1
#include "phonstats.h"

enum m_id { // ordering has to match with getopt & p_info
    M_JP = 0,
    M_TP,
    M_MI,
    M_SV,
    M_H,
    M_RTP,
    M_RSV,
    M_RH,
    M_PUB,
    M_PUE,
    M_SUB,
    M_SUE,
    M_LCB,
    M_LCE,
    M_LFB,
    M_LFE,
    M_LPB,
    M_LPE,
    M_MAX
};

#define M_PFMASK 0x00ff
#define M_PRMASK 0xff00
#define M_UBMASK 0xf0000

struct mdata;

struct minfo {
    char *sname;             //short name for the measure
    char *lname;            //long name for the measure
    unsigned long mmask;   //a unique mask
    enum m_id mid;        //id of the measure
    short dir;           //if +1, higher, if -1 lower values indicate a boundary
    short lr;           //if +1, conditioning is left-to-right, -1 right-to-left, 0 bidirectional (MI)
    double (*calc_single)(struct phonstats *ps,
                          struct mdata *mdata,
                          int pos);
    double *(*calc_list)(struct phonstats *ps,
                         struct mdata *mdata);
};


extern struct minfo m_info[];

#endif // _MEASURES_H
