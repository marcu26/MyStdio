#include "so_stdio.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char*argv[], char *env[])
{
    SO_FILE *fptr = so_fopen("ceva.txt","a+");

    char b[256]="aaa acume toamna";

    so_fwrite(b,sizeof(char),strlen(b),fptr);

    char a[512];

    so_fseek(fptr,0,SEEK_SET);

    so_fread(a,sizeof(char),512,fptr);


    printf("%s",a);

    so_fclose(fptr);
}