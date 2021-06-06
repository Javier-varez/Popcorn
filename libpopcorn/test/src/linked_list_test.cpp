/*
 * This file is part of Popcorn
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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "popcorn/utils/linked_list.h"

struct ListElement {
  uint32_t a;
  uint8_t b;
  LinkedList_t list;
};

class LinkedListTest: public ::testing::Test {
  void SetUp() override {
    head = nullptr;
  }

  void TearDown() override { }

 protected:
  LinkedList_t *head;

  ListElement element1 = {
    .a = 100,
    .b = 200,
    .list = { .next = nullptr }
  };
  ListElement element2 = {
    .a = 100,
    .b = 200,
    .list = { .next = nullptr }
  };
  ListElement element3 = {
    .a = 100,
    .b = 12,
    .list = { .next = nullptr }
  };
};

TEST_F(LinkedListTest, ContainerOfTest) {
  ListElement element;

  LinkedList_t *listPtr = &element.list;
  ListElement* elementPtr = CONTAINER_OF(listPtr, struct ListElement, list);
  ASSERT_EQ(elementPtr, &element);
}

TEST_F(LinkedListTest, AddElementTest) {
  LinkedList_AddElement(&head, &element1.list);

  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, nullptr);
}

TEST_F(LinkedListTest, RemoveElementTest) {
  LinkedList_AddElement(&head, &element1.list);

  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, nullptr);

  LinkedList_RemoveElement(&head, &element1.list);

  ASSERT_EQ(head, nullptr);
}

TEST_F(LinkedListTest, AddMultipleElementsTest) {
  LinkedList_AddElement(&head, &element1.list);
  LinkedList_AddElement(&head, &element2.list);

  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, &element2.list);
  ASSERT_EQ(head->next->next, nullptr);
}

TEST_F(LinkedListTest, CanRecoverListElement) {
  LinkedList_AddElement(&head, &element1.list);

  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(CONTAINER_OF(head, ListElement, list), &element1);
}

TEST_F(LinkedListTest, CanRemoveRandomElement) {
  LinkedList_AddElement(&head, &element1.list);
  LinkedList_AddElement(&head, &element2.list);
  LinkedList_AddElement(&head, &element3.list);

  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, &element2.list);
  ASSERT_EQ(head->next->next, &element3.list);
  ASSERT_EQ(head->next->next->next, nullptr);

  LinkedList_RemoveElement(&head, &element2.list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, &element3.list);
  ASSERT_EQ(head->next->next, nullptr);

  LinkedList_RemoveElement(&head, &element3.list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, nullptr);

  LinkedList_RemoveElement(&head, &element1.list);
  ASSERT_EQ(head, nullptr);
}

TEST_F(LinkedListTest, AddEntryTest) {
  LinkedList_AddEntry(head, &element1, list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, nullptr);

  LinkedList_AddEntry(head, &element2, list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, &element2.list);
  ASSERT_EQ(head->next->next, nullptr);
}

TEST_F(LinkedListTest, RemoveEntryTest) {
  LinkedList_AddEntry(head, &element1, list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, nullptr);

  LinkedList_AddEntry(head, &element2, list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, &element2.list);
  ASSERT_EQ(head->next->next, nullptr);

  LinkedList_RemoveEntry(head, &element1, list);
  ASSERT_EQ(head, &element2.list);
  ASSERT_EQ(head->next, nullptr);

  LinkedList_RemoveEntry(head, &element2, list);
  ASSERT_EQ(head, nullptr);

  // Try removing an element that is not in the list
  LinkedList_AddEntry(head, &element1, list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, nullptr);

  // The list shouldn't change
  LinkedList_RemoveEntry(head, &element2, list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, nullptr);
}

TEST_F(LinkedListTest, WalkEntryTest) {
  LinkedList_AddEntry(head, &element1, list);
  LinkedList_AddEntry(head, &element2, list);
  LinkedList_AddEntry(head, &element3, list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, &element2.list);
  ASSERT_EQ(head->next->next, &element3.list);
  ASSERT_EQ(head->next->next->next, nullptr);

  ListElement* element = nullptr;
  int i = 0;
  LinkedList_WalkEntry(head, element, list) {
    switch (i) {
    case 0:
      ASSERT_EQ(element, &element1);
      break;
    case 1:
      ASSERT_EQ(element, &element2);
      break;
    case 2:
      ASSERT_EQ(element, &element3);
      break;
    default:
      FAIL();
      break;
    }
    i++;
  }
  ASSERT_EQ(i, 3);
}

TEST_F(LinkedListTest, WalkEntrySafeTest) {
  LinkedList_AddEntry(head, &element1, list);
  LinkedList_AddEntry(head, &element2, list);
  LinkedList_AddEntry(head, &element3, list);
  ASSERT_EQ(head, &element1.list);
  ASSERT_EQ(head->next, &element2.list);
  ASSERT_EQ(head->next->next, &element3.list);
  ASSERT_EQ(head->next->next->next, nullptr);

  ListElement* element = nullptr;
  ListElement* next = nullptr;
  int i = 0;
  LinkedList_WalkEntry_Safe(head, element, next, list) {
    switch (i) {
    case 0:
      ASSERT_EQ(element, &element1);
      ASSERT_EQ(next, &element2);
      break;
    case 1:
      ASSERT_EQ(element, &element2);
      ASSERT_EQ(next, &element3);
      LinkedList_RemoveEntry(head, element, list);
      break;
    case 2:
      ASSERT_EQ(element, &element3);
      ASSERT_EQ(next, nullptr);
      break;
    default:
      FAIL();
      break;
    }
    i++;
  }
  ASSERT_EQ(i, 3);
}
