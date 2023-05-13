LIBPQ_INCLUDES = $(shell pkg-config --cflags libpq)
LIBPQ_LIBS     = $(shell pkg-config --libs libpq)

run:	libpq
	./$^

libpq:	libpq.c
	gcc -o $@ $(LIBPQ_INCLUDES) $^ $(LIBPQ_LIBS)

clean:
	rm libpq
