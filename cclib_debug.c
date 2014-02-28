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

/*
 * cclib_debug.c 
 * 
 * A set of routines (currently only macros, see cclib_debug.h)
 * for printing debug/info messages
 *
 */
int cclib_debug = 0;
int cclib_color = 0;
int cclib_verbose = 1;

/* void cclib_debug_init(debug, verbose, color) 
 *      set the values used macros used for debugging
 */
void 
cclib_debug_init(int debug, int verbose, int color) 
{
    cclib_debug = debug;
    cclib_color = color;
    cclib_verbose = verbose;
}
