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

#include "Inc/linked_list.h"

void LinkedList_RemoveElement(LinkedList_t** head, LinkedList_t* element) {
    while (*head != NULL) {
        if (*head == element) {
            *head = element->next;
            break;
        }
        head = &(*head)->next;
    }
}

void LinkedList_AddElement(LinkedList_t** head, LinkedList_t* element) {
    while (*head != NULL) {
        head = &(*head)->next;
    }
    *head = element;
    element->next = NULL;
}
