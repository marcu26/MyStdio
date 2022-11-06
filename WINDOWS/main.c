#include "so_stdio.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char*argv[], char *env[])
{
    SO_FILE *fptr = so_popen("mspaint","r");

    char a[256];

    fread(a,sizeof(char),256,fptr);


    printf("%s",a);



    so_fclose(fptr);
}