#include "so_stdio.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char*argv[], char *env[])
{
    SO_FILE *fptr = so_fopen("abasdfasdc.txt","w");

    if(fptr==NULL)
    {
        printf("aaa\n");
    }
    else
    {
        printf("bbb\n");
    }

    so_fputc('a',fptr);
     so_fputc('a',fptr);
      so_fputc('a',fptr);
       so_fputc('a',fptr);


    so_fclose(fptr);
}