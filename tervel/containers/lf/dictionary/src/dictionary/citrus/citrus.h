#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_
#include <stdbool.h>
#include "urcu.h"

/**
 * Copyright 2014 Maya Arbel (mayaarl [at] cs [dot] technion [dot] ac [dot] il).
 * 
 * This file is part of Citrus. 
 * 
 * Citrus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Author Maya Arbel
 */


#define infinity 0xffffffff 


typedef struct ctr_node_t {
    unsigned long key;
    struct ctr_node_t* child[2];
    pthread_mutex_t lock;
    bool marked;
    int tag[2];
    unsigned long value;
} ctr_node_t;

typedef struct ctr_node_t* ctr_node;

ctr_node ctr_init();
unsigned long ctr_contains(ctr_node root, unsigned long key);
bool ctr_insert(ctr_node root, unsigned long key, unsigned long value);
bool ctr_delete(ctr_node root, unsigned long key);

#endif
