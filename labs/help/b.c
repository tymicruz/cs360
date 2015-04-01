/***************** b.c file ************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main(int argc, char *argv[ ])
{
  printf("proc %d in b.out: argc=%d\n", getpid(), argc);

  // WRITE YOUR CODE TO PRINT arv strings
  while(*argv)
  {
  	printf("%s\n", *argv);
	argv++;
  }

  printf("proc %d in b.out exit\n", getpid());
}
/************** end of b.c file ******************/

