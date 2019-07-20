#ifndef CLAS_H
#define CLAS_H

#include <stdarg.h>
#include <stdio.h>

struct Klass {
	size_t size;
	void* (*ctor)(void* self, va_list* app);
	void* (*dtor)(void* self);
};

#endif
