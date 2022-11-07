#include "so_stdio.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char*argv[], char *env[])
{
    SO_FILE *fptr = so_popen("ls *","r");

    if(fptr==NULL)
    {
        printf("aaa\n");
    }
    else
    {
        printf("bbb\n");
    }

   

    char b[256];

    so_fseek(fptr,0,SEEK_SET);

    so_fread(b,sizeof(char),256,fptr);

    printf("%s",b);



    so_fclose(fptr);
}