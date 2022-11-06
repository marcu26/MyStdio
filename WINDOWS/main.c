#include "so_stdio.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char*argv[], char *env[])
{
    SO_FILE *fptr = so_fopen("ceva.txt","w+");

    if(fptr==NULL)
    {
        printf("aaa\n");
    }
    else
    {
        printf("bbb\n");
    }

    char a[256]="ana are mere";

    so_fwrite(a,sizeof(char),strlen(a),fptr);

    so_fputc('$',fptr);

    so_fseek(fptr,3,SEEK_SET);

    so_fputc('!',fptr);

    so_fseek(fptr,0,SEEK_SET);

    char b[256];

    so_fread(b,sizeof(char),256,fptr);

    printf("%s",b);




    so_fclose(fptr);
}