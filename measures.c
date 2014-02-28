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

#include "measures.h"
#include "pred.h"
#include "ub.h"
#include "lex.h"

struct minfo m_info[]={
    {.sname = "jp", 
     .lname = "Joint Probability",
     .calc_single = calc_pred_single,
     .calc_list = calc_pred_list,
     .mid = M_JP,
     .dir = -1,
     .lr = 0,
     .mmask = 0x0001},
    {.sname = "tp",
     .lname = "Transitional Probability",
     .calc_single = calc_pred_single,
     .calc_list = calc_pred_list,
     .mid = M_TP,
     .dir = -1,
     .lr = 1,
     .mmask = 0x0002},
    {.sname = "mi",
     .lname = "Pointwise mutual information",
     .calc_single = calc_pred_single,
     .calc_list = calc_pred_list,
     .mid = M_MI,
     .dir = -1,
     .lr = 0,
     .mmask = 0x0004},
    {.sname = "sv",
     .lname = "Successor Variety",
     .calc_single = calc_pred_single,
     .calc_list = calc_pred_list,
     .mid = M_SV,
     .dir = 1,
     .lr = 1,
     .mmask = 0x0008},
    {.sname = "h",
     .lname = "Conditional Entropy",
     .calc_single = calc_pred_single,
     .calc_list = calc_pred_list,
     .mid = M_H,
     .dir = 1,
     .lr = 1,
     .mmask = 0x0010},
    {.sname = "rtp",
     .lname = "Reverse Transitional Probability",
     .calc_single = calc_pred_single,
     .calc_list = calc_pred_list,
     .mid = M_RTP,
     .dir = -1,
     .lr = -1,
     .mmask = 0x0100},
    {.sname = "rsv",
     .lname = "Predecessor variety",
     .calc_single = calc_pred_single,
     .calc_list = calc_pred_list,
     .mid = M_RSV,
     .dir = 1,
     .lr = -1,
     .mmask = 0x0200},
    {.sname = "rh",
     .lname = "Reverse Conditional Entropy",
     .calc_single = calc_pred_single,
     .calc_list = calc_pred_list,
     .mid = M_RH,
     .dir = 1,
     .lr = -1,
     .mmask = 0x0400},
    {.sname = "pub",
     .lname = "Phonotactics using utterance beginnings",
     .calc_single = calc_ub_single,
     .calc_list = calc_ub_list,
     .mid = M_PUB,
     .dir = 1,
     .lr = -1,
     .mmask = 0x10000},
    {.sname = "pue",
     .lname = "Phonotactics using utterance ends",
     .calc_single = calc_ue_single,
     .calc_list = calc_ue_list,
     .mid = M_PUE,
     .dir = 1,
     .lr = 1,
     .mmask = 0x20000},
    {.sname = "sub",
     .lname = "Stress using utterance beginnings",
     .calc_single = calc_ub_single,
     .calc_list = calc_ub_list,
     .mid = M_SUB,
     .dir = 1,
     .lr = -1,
     .mmask = 0x10000000},
    {.sname = "sue",
     .lname = "Stress using utterance ends",
     .calc_single = calc_ue_single,
     .calc_list = calc_ue_list,
     .dir = 1,
     .lr = 1,
     .mid = M_SUE,
     .mmask = 0x20000000},
    {.sname = "LCe",
     .lname = "Number of contexts the lexical items before show up together",
     .calc_single = calc_lex_single,
     .calc_list = calc_lex_list,
     .dir = 1,
     .lr = 1,
     .mid = M_LCE,
     .mmask = 0x100000},
    {.sname = "lCb",
     .lname = "Number of contexts the lexical items after show up together",
     .calc_single = calc_lex_single,
     .calc_list = calc_lex_list,
     .mid = M_LCB,
     .dir = 1,
     .lr = -1,
     .mmask = 0x200000},
    {.sname = "LFe",
     .lname = "Frequency of lexical items that end at the boundary",
     .calc_single = calc_lex_single,
     .calc_list = calc_lex_list,
     .dir = 1,
     .lr = 1,
     .mid = M_LFE,
     .mmask = 0x400000},
    {.sname = "LFb",
     .lname = "Frequency of lexical items that begin at the boundary",
     .calc_single = calc_lex_single,
     .calc_list = calc_lex_list,
     .mid = M_LFB,
     .dir = 1,
     .lr = -1,
     .mmask = 0x800000},
    {.sname = "LPe",
     .lname = "Phonotactics score before the boundary",
     .calc_single = calc_ub_single,
     .calc_list = calc_ue_list,
     .dir = 1,
     .lr = 1,
     .mid = M_LPE,
     .mmask = 0x1000000},
    {.sname = "LPb",
     .lname = "Phonotactics score after the boundary",
     .calc_single = calc_ub_single,
     .calc_list = calc_ub_list,
     .mid = M_LPB,
     .dir = 1,
     .lr = -1,
     .mmask = 0x2000000},
};

