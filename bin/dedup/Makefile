CC=clang
CXX=clang++
CFLAGS=-I. -O0 -ggdb3
CXXFLAGS=-I. -O0 -ggdb3
DEPS = dedup.h
OBJ = dedup.o 

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

dedup: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
		rm -f *.o dedup
