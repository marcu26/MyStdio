#include "so_stdio.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char*argv[], char *env[])
{
    SO_FILE *fptr = so_fopen("ceva.txt","a");

    char b[256]="aaa acume toamna";

    so_fseek(fptr,0,SEEK_END);

    so_fwrite(b,sizeof(char),strlen(b),fptr);

    so_fseek(fptr,0,SEEK_END);

    so_fwrite(b,sizeof(char),strlen(b),fptr);


    so_fclose(fptr);
}