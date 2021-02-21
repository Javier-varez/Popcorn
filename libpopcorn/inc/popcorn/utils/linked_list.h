/* 
 * This file is part of the Cortex-M Scheduler
 * Copyright (c) 2020 Javier Alvarez
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// -V:LinkedList_WalkEntry_Safe:623

#ifndef POPCORN_UTILS_LINKED_LIST_H_
#define POPCORN_UTILS_LINKED_LIST_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LinkedList_t_ {
  struct LinkedList_t_* next;
} LinkedList_t;

#define CONTAINER_OF(ptr, obj, member) \
  ((obj*)(((ptr) == NULL) ? NULL : (((uint8_t*)(ptr)) - offsetof(obj, member))))  // NOLINT

#define LinkedList_NextEntry(element, obj, member) \
  (((element) == NULL) ? NULL : CONTAINER_OF(element->member.next, obj, member))

#define LinkedList_WalkEntry(head, element, member) \
  for (element = CONTAINER_OF(head, typeof(*element), member); \
       element != NULL; \
       element = CONTAINER_OF(element->member.next, typeof(*element), member))

#define LinkedList_WalkEntry_Safe(head, element, next, member) \
  for (element = CONTAINER_OF(head, typeof(*element), member), \
       next = LinkedList_NextEntry(element, typeof(*element), member); \
       element != NULL; \
       element = next, \
       next = LinkedList_NextEntry(element, typeof(*element), member))

void LinkedList_RemoveElement(LinkedList_t** head, LinkedList_t* element);
void LinkedList_AddElement(LinkedList_t** head, LinkedList_t* element);

#define LinkedList_AddEntry(head, element, member) \
  LinkedList_AddElement(&(head), &(element)->member)

#define LinkedList_RemoveEntry(head, element, member) \
  LinkedList_RemoveElement(&(head), &(element)->member)

#ifdef __cplusplus
}
#endif

#endif  // POPCORN_UTILS_LINKED_LIST_H_
