// Example of persistent afl
//
// Compile with:
//	afl-clang-fast -o afl_persistent afl_persistent.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void stateless_func(char* var)
{
char buffer[256];
memcpy(buffer, var, strlen(var));
printf(buffer);
}

int main()
{
char input[256];
while(__AFL_LOOP(1000))
{
read(0, input, 256);
stateless_func(input);
}
return 0;
}
