// Thomas Kaunzinger
// slist implementation

#include "slist.h"
#include <stdlib.h>
#include <string.h>

slist* s_cons(const char* text, slist* rest) {
	slist* newNode = malloc(sizeof(slist));
	newNode->data = strdup(text);
	newNode->refs = 0;
	newNode->next = rest;
	return newNode;
}

void s_free(slist* xs) {
	while (xs) {
		free(xs->data);
		slist* next = xs->next;
		free(xs);
		xs = next;
	}
}

slist* s_split(const char* text, char delim) {
	if (!text) {
		return 0;
	}
	slist* ret = malloc(sizeof(slist));
	char* toParse = strdup(text);
	char* running = strtok(toParse, &delim);
	while (running) {
		ret = s_cons(running, ret);
		running = strtok(0, &delim);
	}
	return ret;
}

