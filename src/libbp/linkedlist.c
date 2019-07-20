#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "klass.h"
#include "new.h"

#include "linkedlist.h"

#define MAX_COUNT 999

static void*
LinkedList_constructor(void* _self, va_list* args)
{
	(void)args;

	struct LinkedList* self = _self;

	/* size_t size = va_arg(*args, size_t); */
	/* void* linkedlist = va_arg(*args, void*); */
	/* self->data = malloc(size); */
	/* assert(self->data); */
	/* memcpy((void*)self->data, linkedlist, size); */

	self->next = NULL;
	self->item = NULL;

	/* fprintf(stderr, "Constructing linkedlist %p\n", (void*)self); */
	return self;
}

static void*
LinkedList_destructor(void* _self)
{
	struct LinkedList* self = _self;

	/* free((void*)self->data), self->data = NULL; */

	/* fprintf(stderr, "Destructing linkedlist %p\n", (void*)self); */
	return self;
}

/* This struct acts as the class definition */
static const struct Klass _LinkedList = {sizeof(struct LinkedList),
					 LinkedList_constructor,
					 LinkedList_destructor};

/* This extern variable is a template of the class defined above,
 * which is cloned by new() into an indepenent instance */
const void* LinkedList = &_LinkedList;


/**********************************************************************
 *                              Methods                               *
 **********************************************************************/

t_LinkedList*
getNext(t_LinkedList* self)
{
	return self->next;
}

int
getCount(t_LinkedList* item)
{ /* Note: Returned count includes *self parameter if not null */

	int count = 0;

	while (item != NULL) {
		item = item->next;
		count++;
		assert(count < MAX_COUNT);
	}

	return count;
}

void*
getItem(t_LinkedList* self)
{
	assert(self);
	return self->item;
}

void
setItem(t_LinkedList* list, void* item)
{
	assert(list);
	list->item = item;
}

t_LinkedList*
getParent(t_LinkedList* list, t_LinkedList* child)
{ /* We don't have a pointer to the parent,
     so we have to run through the list until we find it */

	while (list) {
		if (list->next == child) break;
		list = list->next;
	}
	return list;
}

t_LinkedList*
getLast(t_LinkedList* self)
{
	return getParent(self, NULL);
}

t_LinkedList*
addChild(t_LinkedList* self)
{ /* Allocate new list entry and append to existing.
     If existing already has a child, insert new item */

	assert(self);
	t_LinkedList* newList = new (LinkedList);

	if (self->next) newList->next = self->next;

	self->next = newList;
	return newList;
}

t_LinkedList*
removeEntry(t_LinkedList* self, t_LinkedList* entry)
{ /* Remove this list item and return the parent
     Join the parent to any existing child */

	assert(self);
	/* ll_showFamily(self); */
	if (entry == self) {
		delete (entry);
		return NULL;
	}
	if (!isChild(self, entry)) return self; /* Entry not in list */

	t_LinkedList* parent = getParent(self, entry);
	t_LinkedList* child = entry->next;

	if (parent) parent->next = child ? child : NULL;

	delete (entry);
	return self;

	/* ll_showFamily(self); */
	/* if (child) return child; */
	/* if (parent) return parent; */

	/* abort(); */
	/* return NULL; */
}

bool
isChild(t_LinkedList* self, t_LinkedList* entry)
{ /* True if the entry exists within the list */

	assert(self);
	assert(entry);

	while (self->next) {
		if (self->next == entry) return true;
		self = self->next;
	}
	return false;
}

void
ll_deleteFamily(t_LinkedList* self)
{ /* Deletes this list item and children, including messages */

	while (self) {
		t_LinkedList* next = self->next;
		if (self->item) delete (self->item);
		delete (self);
		self = next;
	}
}

void
ll_showFamily(t_LinkedList* self)
{
	while (self) {
		fprintf(stderr, "ll item %p has child %p\n", (void*)self,
			(void*)self->next);
		self = self->next;
	}
}
