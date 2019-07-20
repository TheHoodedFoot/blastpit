#include <assert.h>
#include <stdlib.h>

#include "klass.h"
#include "new.h"

void *new (const void *_klass, ...)
{ /* Will segfault if called with wrong number of arguments */

	/* Allocate memory for our class template */
	const struct Klass *klass = _klass;
	void *p = calloc(1, klass->size);
	assert(p);

	*(const struct Klass **)p = klass;

	if (klass->ctor) {
		va_list ap;

		va_start(ap, _klass);
		p = klass->ctor(p, &ap);
		va_end(ap);
	}
	/* fprintf(stderr, "new: %p (type %p)\n", p, _klass); */
	return p;
}

void delete (void *self)
{
	const struct Klass **cp = self;

	if (self && *cp && (*cp)->dtor) self = (*cp)->dtor(self);
	free(self);
	/* fprintf(stderr, "delete complete (destructor called): %p\n", self);
	 */
}

size_t
sizeOf(const void *self)
{
	const struct Klass *const *cp = self;

	assert(self && *cp);
	return (*cp)->size;
}
