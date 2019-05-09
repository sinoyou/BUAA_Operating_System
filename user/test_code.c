/*************************************************************************
    > File Name: test_code.c
    > Author: Lebway
    > Mail: lebway@foxmail.com 
    > Created Time: Sat Apr 27 00:27:16 2019
 ************************************************************************/
#include"lib.h"

void umain(void)
{
	int i;
	for(i=0;i<2000000;i++) {
		
		int id = fork();
		if(id == 0)
			break;
		writef("I have creat %d env\n", i);
	}
}
