#include "so_stdio.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char*argv[], char *env[])
{
    SO_FILE *fptr = so_fopen("ceva.txt","a");

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



    so_fclose(fptr);
}