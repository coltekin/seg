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

#ifndef _CCLIB_DEBUG_H
#define _CCLIB_DEBUG_H 1

extern int cclib_debug;
extern int cclib_color;
extern int cclib_verbose;

/*
 * macros PDEBUG, PINFO, PERROR, PFATAL
 */

#define PDEBUG(level, format, args...) \
        {if(level <= cclib_debug){\
            if(cclib_color) {\
                fprintf(stderr, "\033[32m"format"\033[0m", ##args);\
            } else {\
                fprintf(stderr, "(DD) "format, ##args);\
            }\
         }\
        }

#define PINFO(format, args...) \
        {if(cclib_verbose){\
            if(cclib_color) {\
                fprintf(stderr, "\033[36m"format"\033[0m", ##args);\
            } else {\
                fprintf(stderr, "(II) "format, ##args);\
            }\
         }\
        }

#define PERROR(format, args...) \
        {if(cclib_color) {\
             fprintf(stderr, "\033[31m"format"\033[0m", ##args);\
         } else {\
             fprintf(stderr, "(E)  "format, ##args);\
         }\
        }

#define PFATAL(format, args...) \
        {if(cclib_color) {\
             fprintf(stderr, "\033[1;31m"format"\033[0m", ##args);\
         } else {\
             fprintf(stderr, "(EE) "format, ##args);\
         }\
         exit(-1);\
        }

void cclib_debug_init(int debug, int verbose, int color);

#endif // _CCLIB_DEBUG_H
