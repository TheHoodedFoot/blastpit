#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdbool.h>

/* extern pointer to our class template */
extern const void *LinkedList;

typedef struct LinkedList { /* LinkedList object struct */
	const void *clas;   /* must be first */
	struct LinkedList *next;
	void *item; /* Pointer to item within the list */
} t_LinkedList;

bool isChild(t_LinkedList *, t_LinkedList *);
int getCount(t_LinkedList *);
t_LinkedList *addChild(t_LinkedList *);
t_LinkedList *getLast(t_LinkedList *);
t_LinkedList *getNext(t_LinkedList *);
t_LinkedList *getParent(t_LinkedList *, t_LinkedList *);
/* t_LinkedList *pop(t_LinkedList *); */
/* t_LinkedList *push(t_LinkedList *); */
t_LinkedList *removeEntry(t_LinkedList *, t_LinkedList *);
void *getItem(t_LinkedList *);
void setItem(t_LinkedList *, void *);
void ll_deleteFamily(t_LinkedList *self);
void ll_showFamily(t_LinkedList *self);

#endif /* end of include guard: LINKEDLIST_H */
