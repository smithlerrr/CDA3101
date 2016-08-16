#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int type = NULL;
int op = 0;
int rs = 0;
int rt = 0;
int rd = 0;
int shamrt = 0;
int funct = 0;
int immed = 0;
char* inst; 
char* args;
unsigned int hexcode;

void instrToInt(char*, char*, char*);

int main()
{
	char *i;
	char *a;
	char *h;
	instrToInt(inst, args, hexcode);
	printf("%08x\n", hexcode);
}

void instrToInt(inst, args, hexcode)
{
	if(strcmp!("add", inst))
	{
		op = 0;
		funct = 32;
		type = r;
	}
	
	else if(strcmp!("sub", inst))
	{
		op = 0;
		funct = 34;
		type = r;
	}
		
	else if(strcmp!("lw", inst))
	{
		op = 35;
		type = i;
	}

	else if(strcmp!("sw", inst))
	{
		op = 43;
		type = i;
	}

	else if(strcmp!("beq", inst))
	{
		op = 4;
		type = i;
	}

	else if(strcmp!("mov", inst))
	{
		op = 0;
		funct = 0;
	}

	else if(strcmp!("and", inst))
	{
		op = 0;
		funct = 36;
	}
	
	else if(strcmp!("not", inst))
		{op = 0;}	 

	int args_length = strlength(args);
	char registers[3];

	if(type == r)
	{
		int j = 0;
		for(int i = 0; i < args_length; i++)
		{
			if(isnum(args[i])
			{
				registers[j] = put(args[i])
				j++;
			}
		}

		rd = registers[0];
		rs = registers[1];
		rt = registers[2];

		hexcode += op;
      	hexcode = (hexcode << 5);
    	hexcode += rs;
    	hexcode = (hexcode << 5);
    	hexcode += rt;
    	hexcode = (hexcode << 5);
    	hexcode += rd;
    	hexcode = (hexcode << 5);
    	hexcode += shamt;
    	hexcode = (hexcode << 6);
    	hexcode += funct;
	}

	if(type == i)
	{
		int j = 0;
		for(int i = 0; i < args_length; i++)
		{
			if(isnum(args[i])
			{
				registers[j] = put(args[i])
				j++;
			}
		}

		rd = registers[0];
		rs = registers[1];
		immed = registers[2];

		hexcode += op;
        hexcode = (hexcode << 6);
        hexcode += rs;
        hexcode = (hexcode << 5);
        hexcode += rt;
        hexcode = (hexcode << 16);
        hexcode += immed;
	}
}