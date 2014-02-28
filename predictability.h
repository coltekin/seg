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

#ifndef _PREDICTABILITY_H
#define _PREDICTABILITY_H 1

#include "phonstats.h"

enum pred_measure { // ordering has to match with getopt & p_info
    PM_JP = 0,
    PM_TP,
    PM_MI,
    PM_SV,
    PM_H,
    PM_TPR, PM_RTP = PM_TPR,
    PM_SVR, PM_PV = PM_SVR,
    PM_HR, PM_RH = PM_HR,
    PM_MAX
};



struct pred_minfo {
    char *sname;
    char *lname;
    unsigned mmask;
};

struct mlist_list {
    size_t   llen;
    unsigned *x;
    unsigned *y;
    enum pred_measure *m;
    double **mlist;
};

#define PM_MMASK 0x00ff
#define PM_REVMASK 0xff00

extern struct pred_minfo p_info[];


double joint_p(struct phonstats *ps, char *x, char *y);
double cond_p(struct phonstats *ps, char *x, char *y);
double pmi(struct phonstats *ps, char *x, char *y);
double cond_entropy(struct phonstats *ps, char *x, int y_len);
size_t sv(struct phonstats *ps, char *x, int y_len);

double cond_p_r(struct phonstats *ps, char *x, char *y);
double cond_entropy_r(struct phonstats *ps, char *x, int y_len);
size_t sv_r(struct phonstats *ps, char *x, int y_len);

double npmi(struct phonstats *ps, char *x, char *y);

void pred_mlist_list_free(struct mlist_list *ml);


double * pred_list(struct phonstats *ps, 
                   char *s, 
                   enum pred_measure m, 
                   int x_len, int y_len);
double * pred_list_reverse(struct phonstats *ps, 
                           char *s, 
                           enum pred_measure m, 
                           int x_len, int y_len);
double * pred_list_forward(struct phonstats *ps, 
                           char *s, 
                           enum pred_measure m, 
                           int x_len, int y_len);

struct mlist_list * pred_mlist_list(struct phonstats *ps, 
                                    char *s, 
                                    unsigned mmask, 
                                    int xmin, int xmax,
                                    int ymin, int ymax);



double cond_entropy_str(struct phonstats *ps, char *s, int pos, int x_len, int y_len);
double pmi_str(struct phonstats *ps, char *s, int pos, int x_len, int y_len);
double joint_p_str (struct phonstats *ps, char *s, int pos, int len_x, int len_y);
double cond_p_str (struct phonstats *ps, char *s, int pos, int len_x, int len_y);
#endif // _PREDICTABILITY_H
