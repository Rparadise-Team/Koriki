#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/definitions.h"
#include "../headers/globals.h"
#include "../headers/logic.h"

struct Node* GetNewNode(struct Rom *rom) {
	struct Node* newNode= (struct Node*)malloc(sizeof(struct Node));
	newNode->data = rom;
	newNode->prev = NULL;
	newNode->next = NULL;
	return newNode;
}

void InsertAtTail(struct Rom *rom) {
	struct Node* temp = CURRENT_SECTION.head;
	struct Node* newNode = GetNewNode(rom);
	if(CURRENT_SECTION.head == NULL) {
		CURRENT_SECTION.head = newNode;
		return;
	}
	while(temp->next != NULL) {
		temp = temp->next; // Go To last Node
	}
	temp->next = newNode;
	newNode->prev = temp;
}


void cleanListForSection(struct MenuSection *section) {
	struct Node *current = NULL;
	while ((current = section->head)) {
		if (!current) {
			break;
		}
		if (current->data->alias!=NULL&&strlen(current->data->alias)>1) {
			free(current->data->alias);
		}
		free(current->data->name);
		free(current->data->directory);
		free(current->data);
		section->head = section->head->next;
		free(current);
	}
}

void InsertAtTailInSection(struct MenuSection *section, struct Rom *rom) {
	struct Node* temp = section->head;
	struct Node* newNode = GetNewNode(rom);
	if(section->head == NULL) {
		section->head = newNode;
		return;
	}
	while(temp->next != NULL) {
		temp = temp->next; // Go To last Node
	}
	temp->next = newNode;
	newNode->prev = temp;
}

void PrintDoublyLinkedRomList() {
	struct Node* temp = CURRENT_SECTION.head;
	while(temp != NULL) {
		printf("%s \n",temp->data->name);
		temp = temp->next;
	}
	printf("\n");
}

struct Rom* GetNthElement(int index)
{
	struct Node* current = CURRENT_SECTION.head;
	int count = 0;
	while (current != NULL) {
		if (count == index) {
			return(current->data);
		}
		count++;
		current = current->next;
	}
	return NULL;
}

struct Rom* getCurrentRom() {
	return GetNthElement(CURRENT_GAME_NUMBER);
}

struct Node* GetNthNode(int index) {
	struct Node* current = CURRENT_SECTION.head;
	int count = 0;
	while (current != NULL) {
		if (count == index) {
			return(current);
		}
		count++;
		current = current->next;
	}
	return NULL;
}

struct Node *getCurrentNode() {
	return GetNthNode(CURRENT_GAME_NUMBER);
}

char *getCurrentSectionExistingLetters() {
	char *letters=calloc(29,1);
	struct Node* temp = CURRENT_SECTION.head;
	int hadNumbers = 0;
	while(temp != NULL) {
		char *name = getFileNameOrAlias(temp->data);
		char *upperInitialLetter = malloc(2);
		upperInitialLetter[0]=toupper(name[0]);
		upperInitialLetter[1]='\0';
		if (strstr(letters,upperInitialLetter)==NULL) {
			if(isdigit(upperInitialLetter[0])||!isalpha(upperInitialLetter[0])) {
				if(!hadNumbers) {
					strcat(letters,"#");
				}
				hadNumbers=1;
			} else {
				strcat(letters,upperInitialLetter);
			}
		}
		free(name);
		free(upperInitialLetter);
		temp = temp->next;
	}
	return letters;
}
