#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "klass.h"
#include "new.h"

#include "parser.h"

static void*
Parser_constructor(void* _self, va_list* args)
{
	(void)args;

	struct Parser* self = (struct Parser*)_self;

	/* size_t size = va_arg(*args, size_t); */
	/* void* parser = va_arg(*args, void*); */
	/* self->data = malloc(size); */
	/* assert(self->data); */
	/* memcpy((void*)self->data, parser, size); */

	return self;
}

static void*
Parser_destructor(void* _self)
{
	struct Parser* self = (struct Parser*)_self;

	/* free((void*)self->data), self->data = NULL; */

	return self;
}

/* This struct acts as the class definition */
static const struct Klass _Parser = {sizeof(struct Parser),
				     Parser_constructor, Parser_destructor};

/* This extern variable is a template of the class defined above,
 * which is cloned by new() into an indepenent instance */
const void* Parser = &_Parser;


/**********************************************************************
 *                              Methods                               *
 **********************************************************************/
