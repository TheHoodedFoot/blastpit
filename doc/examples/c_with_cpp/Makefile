.PHONY:	all clean

library:
	g++ -shared -fPIC -o mylib.so mylib.cpp
	gcc -c -o main.o main.c
	gcc -o a.out -Wl,-R$(shell pwd) main.o mylib.so
	./a.out

object:
	g++ -c -o mylib.o mylib.cpp
	gcc -c -o main.o main.c
	gcc -o a.out main.o mylib.o -lstdc++
	./a.out

clean:
	rm -f a.out mylib.*o main.o
