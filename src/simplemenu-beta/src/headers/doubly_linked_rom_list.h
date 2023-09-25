/*
 * doubly_linked_rom_list.h
 *
 *  Created on: Feb 10, 2020
 *      Author: bittboy
 */

#ifndef SRC_HEADERS_DOUBLY_LINKED_ROM_LIST_H_
#define SRC_HEADERS_DOUBLY_LINKED_ROM_LIST_H_
#include "../headers/globals.h"



struct Rom* GetNthElement(int index);
struct Node* GetNthNode(int index);
struct Rom* getCurrentRom();
void InsertAtTail(struct Rom *rom);
void InsertAtTailInSection(struct MenuSection *section, struct Rom *rom);
void PrintDoublyLinkedRomList();
struct Node *getCurrentNode();
void cleanListForSection(struct MenuSection *section);
char* getCurrentSectionExistingLetters();

#endif /* SRC_HEADERS_DOUBLY_LINKED_ROM_LIST_H_ */
