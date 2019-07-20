#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "klass.h"
#include "message.h"
#include "new.h"

static void*
Message_constructor(void* _self, va_list* args)
{
	struct Message* self = (struct Message*)_self;

	size_t size = va_arg(*args, size_t);
	void* message = va_arg(*args, void*);
	self->data = (char*)malloc(size + 1);
	assert(self->data);
	memcpy((void*)self->data, message, size);
	self->size = size;
	*(self->data + self->size) = 0;

	return self;
}

static void*
Message_destructor(void* _self)
{
	struct Message* self = (struct Message*)_self;

	free((void*)self->data);
	self->data = NULL, self->size = 0;

	return self;
}

/* This struct acts as the class definition */
static const struct Klass _Message = {
	sizeof(struct Message), Message_constructor, Message_destructor};

/* This extern variable is a template of the class defined above,
 * which is cloned by new() into an indepenent instance */
const void* Message = &_Message;


/**********************************************************************
 *                              Methods                               *
 **********************************************************************/

int
isMessageEqual(t_Message* self, const char* comparison)
{
	return memcmp(self->data, comparison, self->size);
}

char*
getMessageData(t_Message* self)
{ /* Return a pointer to the message data */

	return self->data;
}

int
getMessageSize(t_Message* self)
{
	return self->size;
}
