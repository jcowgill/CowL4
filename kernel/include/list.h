#ifndef KERNEL_LIST_H
#define KERNEL_LIST_H

/*
 * kernel/src/include/list.h
 * Linked Lists
 *
 * Copyright (C) 2013 James Cowgill
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY# without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http:// www.gnu.org/licenses/>.
 */

#include "global.h"

// Structure used to link data together
typedef struct ListNode
{
    struct ListNode * prev;
    struct ListNode * next;
} ListNode;

// Structure used to store the head of the list
typedef struct List
{
    ListNode sentinal;
} List;

#define LIST_CREATE(name) { { &(name).sentinal, &(name).sentinal } }

// Initializes the given list (as empty)
static inline void ListInit(List * list)
{
    list->sentinal.prev = &list->sentinal;
    list->sentinal.next = &list->sentinal;
}

// Returns true if the given list is empty
static inline bool ListIsEmpty(List * list)
{
    return list->sentinal.next == &list->sentinal;
}

// Adds an item before the given node
static inline void ListAddBefore(ListNode * node, ListNode * newNode)
{
    newNode->next = node;
    newNode->prev = node->prev;
    node->prev->next = newNode;
    node->prev = node;
}

// Adds an item after the given node
static inline void ListAddAfter(ListNode * node, ListNode * newNode)
{
    newNode->prev = node;
    newNode->next = node->next;
    node->next->prev = newNode;
    node->next = node;
}

// Adds an item at the beginning of the list
static inline void ListAddFirst(List * list, ListNode * newNode)
{
    ListAddAfter(&list->sentinal, newNode);
}

// Adds an item at the end of the list
static inline void ListAddLast(List * list, ListNode * newNode)
{
    ListAddBefore(&list->sentinal, newNode);
}

// Removes the given node from the list
static inline void ListDelete(ListNode * node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = NULL;
    node->next = NULL;
}

// Gets the structure from a list node
#define ListGet(node, type, member) \
    (type *) ((char *) (node) - offsetof((type), (member)))

// Iterates over the given list using var as the loop counter variable
//  The type of objects are infered from the type of var
#define ListForEach(var, list, member) \
    for(var = ListGet((list)->sentinal.next, typeof(*(var)), member); \
        &(var)->member != &(list)->sentinal; \
        var = ListGet((var)->member.next, typeof(*(var)), member))

// Iterates over the given list safely (you can modify the list while iterating)
//  The temporary variable supplied is created in the loop
#define ListForEachSafe(var, tmp, list, member) \
    for(var = ListGet((list)->sentinal.next, typeof(*(var)), member), \
        typeof(var) tmp = ListGet(var->member.next, typeof(*(var)), member); \
        &var->member != (list); \
        var = tmp, tmp = ListGet(tmp->member.next, typeof(*(var)), member))

#endif
