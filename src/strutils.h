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

#ifndef _STRUTILS_H
#define _STRUTILS_H       1

char *str_span(char *s, int start, int length);
char *str_rangecpy(char *dest, char *src, int start, int length);


char *str_rev(char *s);
char *str_revn(char *s, unsigned n);
char *str_revi(char *s);
char *str_revni(char *s, unsigned n);

char *str_rmch(char *s, char ch, unsigned short **pos);
char *str_rmchs(char *s, const char *rm, unsigned short **pos);


char *str_lstrip(char *s, const char *rm);
char *str_rstrip(char *s, const char *rm);
char *str_strip(char *s, const char *rm);

char *str_astrcat(char **dst, char *src);


#endif // _STRUTILS_H 
