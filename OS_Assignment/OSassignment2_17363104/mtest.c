#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <unistd.h>
#include <stddef.h>

int bss;       // uninitialized variable
int data0 = 1;   // initialized variable

void stack_fun();  

int main(int argc,char **argv)
{
  static int data1 = 7;
  char *b = sbrk((ptrdiff_t)0), *stack_p;   
  
  printf("Code Address: '\n");
  printf("\tAddress of main function: %p\n",main);
  printf("\tAddress of stack_fun function: %p\n",stack_fun);
  printf("-------------------------------------------------------------------\n");

  
  printf("Data Address:\n");
  printf("\tAddress of data0: %p\n",&data0);
  printf("\tAddress of data1: %p\n",&data1);
  printf("-------------------------------------------------------------------\n");
  
  printf("BSS Address: \n");
  printf("\tAddress of bss: %p\n",&bss);
  printf("-------------------------------------------------------------------\n"); 
   
  printf("Stack Address: \n");
  stack_fun();
  stack_p = (char *) alloca(64);
	if (stack_p != NULL) {
		printf("\tAddress of array head: %p\n", stack_p);
		printf("\tAddress of array tail: %p\n", stack_p + 63);
	}
  printf("-------------------------------------------------------------------\n");
  

  printf("Heap Address:\n");
  printf("\tHeap original address: %p\n",b);
  b = sbrk((ptrdiff_t) 32);     // grow heap
  b = sbrk((ptrdiff_t) 0);
  printf("\tAddress after growth: %p\n",b);
  b = sbrk((ptrdiff_t) -16);
  b = sbrk((ptrdiff_t)0);        // shrink it
  printf("\tAddress after shrink: %p\n",b); 
  printf("-------------------------------------------------------------------\n");
  while(1);
  return 0;
}

void stack_fun(){
   static int time = 0; 
  auto int stack0;  
  if (++time == 3) 
		return -1;
  printf("\tStack function NO: %d, address of stack0: %p \n", time, & stack0);  
  stack_fun(); 
}

