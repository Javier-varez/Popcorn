
#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LinkedList {
    struct LinkedList* next;
} LinkedList_t;

#define CONTAINER_OF(ptr, obj, member) \
    ((obj *)((ptr == NULL) ? NULL : (((uint8_t*)ptr) - offsetof(obj, member))))

#define LinkedList_NextEntry(element, obj, member) \
    ((element == NULL) ? NULL : CONTAINER_OF(element->member.next, obj, member))

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
    LinkedList_AddElement(&head, &element->member)

#define LinkedList_RemoveEntry(head, element, member) \
    LinkedList_RemoveElement(&head, &element->member)

#ifdef __cplusplus
}
#endif

#endif // LINKED_LIST_H_