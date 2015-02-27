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

#ifndef _STACK_H
#define _STACK_H       1

#include <stdbool.h>

struct stack {
    void *data;
    struct stack *next;
};

/**
 * stack_init() - initialze a stack structure
 *
 * Stack is implemented as a linked list with dummy head node, which
 * is allocated and returnd by stack_init();
 */
struct stack    *stack_init();

/**
 * stack_push() - push a pointer to the stack
 */
void            stack_push(struct stack *st, void *data);

/**
 * stack_push() - return and remove the pointer on top of the stack
 */
void            *stack_pop(struct stack *st);

/**
 * stack_push() - deallocate all memory used by the stack
 * @st:           the stack to destroy
 * @free_data:    instruct stack_free() to also remove the data in the
 *                already existing items, if any.
 */
void            stack_free(struct stack *st, bool free_data);

bool stack_is_empty(struct stack *st);

#endif /* _STACK_H */
