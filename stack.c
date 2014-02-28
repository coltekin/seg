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

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

/* create & return a dummy head-node */
struct stack *stack_init()
{
    struct stack *new; 

    new = malloc(sizeof (*new));
    new->data = NULL;
    new->next = NULL;
    return new;
}

void stack_push(struct stack *st, void *data)
{
    struct stack *new;

    new = malloc(sizeof (*new));
    new->data = data;
    new->next = st->next;
    st->next = new;
}

void *stack_pop(struct stack *st)
{
    struct stack   *top;
    void    *data;

    if (st->next == NULL)
        return NULL;

    top = st->next;
    data = top->data;
    st->next = top->next;
    free(top);
    return data;
}

void stack_free(struct stack *st, int free_data)
{
    void *data = stack_pop(st);

    while (data != NULL){
        if (free_data) {
            free(data);
        }
        data = stack_pop(st);
    }
    free(st);
}
