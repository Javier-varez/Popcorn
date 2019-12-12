
#include "linked_list.h"

void LinkedList_RemoveElement(LinkedList_t** head, LinkedList_t* element)
{
    while (*head != NULL)
    {
        if (*head == element)
        {
            *head = element->next;
            break;
        }
        head = &(*head)->next;
    }
}

void LinkedList_AddElement(LinkedList_t** head, LinkedList_t* element)
{
    while (*head != NULL)
    {
        head = &(*head)->next;
    }
    *head = element;

    element->next = NULL;
}