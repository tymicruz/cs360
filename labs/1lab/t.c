/******************** t.c file *********************************/
//NAME: TYLER CRUZ
#include <stdlib.h>
#include <stdio.h>
typedef unsigned int u32;
char *table = "0123456789ABCDEF";
int BASE = 10;

int rpu(u32 x);
int printu(u32 x);
int printo(u32 x);
int printx(u32 x);
int printd(int x);
int prints(char *s);
int myprintf(char *fmt, ...);

int mymain(int argc, char *argv[ ], char *env[ ])
{
	int i;
	myprintf("\nin mymain(): argc=%d\n", argc);

	for (i=0; i < argc; i++){
		myprintf("argv[%d] = %s\n", i, argv[i]);
	}
	i = 0;
	// WRITE CODE TO PRINT THE env strings
	myprintf("\nenvironment variables:\n");

	while(*env)
	{
		myprintf("env[%d]: ", i);
		prints(*env);
		putchar('\n');	
		env++;
		i++;
	}

  myprintf("---------- testing YOUR myprintf() ---------\n");
  myprintf("this is a test\n");
  myprintf("testing a=%d b=%x c=%c s=%s\n", 123, 123, 'a', "testing");
  myprintf("string=%s, a=%d  b=%u  c=%o  d=%x\n",
           "testing string", -1024, 1024, 1024, 1024);
  myprintf("mymain() return to main() in assembly\n"); 
}
//incomplete
int myprintf(char* fmt, ...)
{
/**1**/
	char *cp;// = fmt;
	int *ebp;//initially will point to stack frame pointer
	ebp = (int *)get_ebp();//point ebp at Stack frame pointer
	ebp++;//move ebp to return address (4 bytes up stack)
	ebp++;//move to first parameter	(8 bytes up stack)
	cp = (char *)(*ebp);//intially point cp to first parameter, then cast cp to a char * because we expect the int * to point to a strin
/*
char *cp = fmt;
int *ip = (&fmt) + 1;

*/
	while(*cp){
		if((*cp) == '%'){
			ebp++;//move to next parameter on the stack(4bytes up)
			cp++;//look at next character in the first parameter (fmt)
			switch((*cp))
			{	
				case 99: putchar(*ebp); ;//'c'  
				break;
				case 100: printd(*ebp);;//'d'
				break;
				case 111: printo(*ebp);;//'o'
				break;
				case 115: prints((char *)(*ebp));;//'s' //I dont have to manually cast ebp into a char*
				break;
				case 117: printu(*ebp);;//'u'
				break;
				case 120: printx(*ebp);;//'x'
				break;
				default: putchar(*cp);
				break;
			}
		}
		else if((*cp) == '\n'){//if new line character
			putchar('\n');
			putchar('\r');
		}
		else{
			putchar(*cp);
		}
		cp++;
	}
}

int rpu(u32 x)
{
	char c;
	if (x){
		c = table[x % BASE];
		rpu(x / BASE);
		putchar(c);
	}
	return 0;
}

int printu(u32 x)//print unsigned integer
{
	BASE = 10;
	if (x==0){
		putchar('0');
	}
	else{
		rpu(x);
	}
	return 0;
}

int printd(int x)//print integer
{
	BASE = 10;
	if(x==0){
		putchar(48); //48 is the ascii value of '0'
	}
	else if(x < 0){
		putchar('-');
		x*=(-1);//make negative x, positive x
		rpu(x);
	}
	else{
		rpu(x);
	}
	return 0;
}

int printo(u32 x)//print octal number
{
	BASE = 8;

	putchar(48);//put a '0' infront of octal number

	if (x==0){
		putchar('0');
	}
	else{
		rpu(x);
	}
	return 0;
}

int printx(u32 x)//print hexidecimal number
{
	BASE = 16;

	putchar(48); //48 is the ascii value of '0'
	putchar(120); //120 is the acsii value of 'x'
	
	if (x==0){
		putchar('0');
	}
	else{
 	 rpu(x);
	}
	return 0;
}

int prints(char *s)//print string
{
	while(*s)
	{
		if((*s) == '\n')
		{
			putchar(*s);
			putchar('\r');
		}else{
			putchar(*s);
		}
		s++;
	}
}
