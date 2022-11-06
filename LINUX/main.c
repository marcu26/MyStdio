#include "so_stdio.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char*argv[], char *env[])
{
    SO_FILE *FILE = so_popen("grep A*","w");
    
    char a[256]="A A A";
    so_fwrite(a,sizeof(char),strlen(a),FILE);

    so_pclose(FILE);

}