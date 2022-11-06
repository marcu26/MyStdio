#include "so_stdio.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 4096

struct _so_file
{
    HANDLE Handle;
    char *Buffer;
    int BufferCursor;
    int IsError;
    int LastOperation; //-1 = nu a fost o alta operatie, 0 = a fost read, 1 = a fost write, 2 = a fost append
    int BytesRead;
    int IsOpenForAppend;
    int pid;
};

SO_FILE* AllocFilePtr()
{
    SO_FILE *FILE = (SO_FILE*)malloc(sizeof(SO_FILE));

    if (FILE == NULL)
        {
            return NULL;
        }

    FILE->Buffer=(char*)malloc(BUFFER_SIZE*sizeof(char));

    if(FILE->Buffer == NULL)
    {
            return NULL;
    }

    FILE->BufferCursor=0;
    FILE->IsError=0;
    FILE->LastOperation=-1;

    return FILE;
}

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
    SO_FILE *FILE = NULL;

    FILE = AllocFilePtr();

    if(strcmp(mode,"r")==0)
    {
       FILE->Handle=CreateFile
       (
        pathname,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
       );

       if(FILE->Handle == INVALID_HANDLE_VALUE)
       {
         free(FILE);
        return NULL;
       }

    }

    else  if(strcmp(mode,"r+")==0)
    {
         FILE->Handle=CreateFile
       (
        pathname,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
       );

       if(FILE->Handle == INVALID_HANDLE_VALUE)
       {
         free(FILE);
        return NULL;
       }
    }

    else  if(strcmp(mode,"w")==0)
    {
        FILE->Handle=CreateFile
       (
        pathname,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
       );

       if(FILE->Handle == INVALID_HANDLE_VALUE)
       {
         free(FILE);
        return NULL;
       }
    }
    else if(strcmp(mode,"w+")==0)
    {
         FILE->Handle=CreateFile
       (
        pathname,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
       );

       if(FILE->Handle == INVALID_HANDLE_VALUE)
       {
         free(FILE);
        return NULL;
       }
    }

    else if(strcmp(mode,"a")==0)
    {
        FILE->Handle=CreateFile
       (
        pathname,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
       );

       FILE->IsOpenForAppend=1;

       if(FILE->Handle == INVALID_HANDLE_VALUE)
       {
         free(FILE);
        return NULL;
       }
    }

    else if(strcmp(mode,"a+")==0)
    {
       FILE->Handle=CreateFile
       (
        pathname,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
       );

        FILE->IsOpenForAppend=1;

       if(FILE->Handle == INVALID_HANDLE_VALUE)
       {
         free(FILE);
        return NULL;
       }
    }

    else
    {
        free(FILE);
        return NULL;
    }

    return FILE;
}

int so_fclose(SO_FILE *stream)
{
    if(stream->LastOperation==1 || stream->LastOperation==2)
    {
    int a = so_fflush(stream);
    if(a==-1)
       {
        stream->IsError=1;
        return -1;
        }
    }


  
    int a = CloseHandle(stream->Handle);

    if(a==0)
    return -1;

    free(stream->Buffer);
    free(stream);
   

    return 1;
}

HANDLE so_fileno(SO_FILE *stream)
{
    return stream->Handle;
}

int so_fflush(SO_FILE *stream)
{
    if(stream->LastOperation!=0 && stream->LastOperation!=-1)
    {
       //if(stream->IsOpenForAppend==1)
        //{
          //  so_fseek(stream,0,SEEK_END);
        //}

        int written;

    int a = WriteFile(stream->Handle,stream->Buffer,stream->BufferCursor,&written,NULL);

    if(a==0)
    {
        stream->IsError=1;
        return -1;
    }
    stream->BufferCursor=0;
    }
}

int so_fgetc(SO_FILE *stream)
{
    if(stream->LastOperation==1 || stream->LastOperation==-1 || stream->BufferCursor==BUFFER_SIZE-1 || stream->LastOperation==2)
    {
        int bytesRead;
        stream->BufferCursor=0;
        int a = ReadFile(stream->Handle,stream->Buffer,BUFFER_SIZE,&stream->BytesRead,NULL);

        if(a==0)
        {
            stream->IsError=1;
        }

        if(stream->BytesRead==0)
        {
            return SO_EOF;
        }

        stream->LastOperation=0;
        stream->BufferCursor+=1;
        return stream->Buffer[stream->BufferCursor-1];
    }

    else if(stream->LastOperation==0)
    {
    stream->BufferCursor+=1;
    return stream->Buffer[stream->BufferCursor-1];
    }
}

int so_ferror(SO_FILE *stream)
{
    if(stream->IsError==1)
    return 1;

    return 0;
}

int so_fputc(int c, SO_FILE *stream)
{
    if(stream->BufferCursor == BUFFER_SIZE)
    {
        int a = so_fflush(stream);

        if(a==-1)
        {
            stream->IsError=1;
            return -1;
        }
    }

    stream->LastOperation=1;
    stream->Buffer[stream->BufferCursor]=c;
    stream->BufferCursor++;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{

    char *p = (char*)ptr;

    int count = 0;

    for(size_t i = 0;i < nmemb * size; i++)
    {
        int a = so_fgetc(stream);

        if(i==stream->BytesRead)
        {
            break;
        }
        p[i]=a;
        count++;
    }

    p[count]='\0';

    return count/size;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    char *p = (char*)ptr;

    int count = 0;

    for(size_t i=0; i<nmemb*size;i++)
    {
        int a = so_fputc(p[i],stream);
        count++;    
    }

    return count/size;
}

long so_ftell(SO_FILE *stream)
{
    long position = SetFilePointer(stream->Handle,0,0,FILE_CURRENT);

    if(position == INVALID_SET_FILE_POINTER)
    {
        return -1;
    }

    if(stream->LastOperation =-1)
    {
        return position;
    }

    if(stream->LastOperation == 0)
    {
    position = position - stream->BytesRead + stream->BufferCursor;
     return position;
    }

    else
    {
        position = position + stream->BufferCursor;
         return position;
    }   
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
    if(stream->LastOperation == 1 || stream->LastOperation==2)
    {
        int a = so_fflush(stream);
        if(a==-1)
        {
            return -1;
        }
    }
    else if(stream->LastOperation==0)
    {
        stream->BufferCursor=0;
    }

    int a=SetFilePointer(stream->Handle,offset,0,whence);

    if(a==INVALID_SET_FILE_POINTER)
    return -1;

    return 0;
}

SO_FILE *so_popen(const char *command, const char *type)
{
    SO_FILE *FILE = AllocFilePtr();
    int flag=0;


    return FILE;
}

int so_pclose(SO_FILE *stream)
{
    return -1;
}
