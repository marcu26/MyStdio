#include "so_stdio.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>


#define BUFFER_SIZE 4096

struct _so_file
{
    HANDLE Handle;
    char Buffer[BUFFER_SIZE];
    int BufferCursor;
    int IsError;
    int LastOperation; //-1 = nu a fost o alta operatie, 0 = a fost read, 1 = a fost write, 2 = a fost append
    int BytesRead;
    int IsOpenForAppend;
    PROCESS_INFORMATION process;
    int Flags; // 1=r, 2=r+, 3=w, 4=w+, 5=a, 6=a+;
    int Eof;
};

SO_FILE* AllocFilePtr()
{
    SO_FILE *FILE = (SO_FILE*)malloc(sizeof(SO_FILE));

    if (FILE == NULL)
        {
            return NULL;
        }

    if(FILE->Buffer == NULL)
    {
            return NULL;
    }
    memset(FILE->Buffer,0,BUFFER_SIZE);
    FILE->BufferCursor=0;
    FILE->IsError=0;
    FILE->LastOperation=-1;
    FILE->Flags=0;
    FILE->IsOpenForAppend=0;
    FILE->Eof=0;
    FILE->BytesRead=0;
    FILE->Handle=INVALID_HANDLE_VALUE;

    return FILE;
}

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
    SO_FILE *FILE = NULL;

    FILE = AllocFilePtr();

    if(FILE == NULL)
    return NULL;

    if(strcmp(mode,"r")==0)
    {
       FILE->Handle=CreateFile
       (
        pathname,
        GENERIC_READ,
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
       FILE->Flags=1;

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
        FILE->Flags=2;
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
       FILE->Flags=3;
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
       FILE->Flags=4;
    }

    else if(strcmp(mode,"a")==0)
    {
        FILE->Handle=CreateFile
       (
        pathname,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
       );

       FILE->IsOpenForAppend=1;

       if(FILE->Handle == INVALID_HANDLE_VALUE)
       {
        free(FILE);
        return NULL;
       }
       FILE->Flags=5;
    }

    else if(strcmp(mode,"a+")==0)
    {
       FILE->Handle=CreateFile
       (
        pathname,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
       );

        FILE->IsOpenForAppend=1;

       if(FILE->Handle == INVALID_HANDLE_VALUE)
       {
        free(FILE);
        return NULL;
       }
       FILE->Flags=6;
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
    if(stream==NULL)
    return -1;

    if(stream->Handle==INVALID_HANDLE_VALUE)
    return -1;

    if(stream->Flags==0)
    return -1;

    int a=0;

    if(stream->Flags!=1)
    {
    a = so_fflush(stream);
        if(a<0)
        {
                stream->IsError=1;
                if(stream!=NULL)
                free(stream);
                return -1;
        }
    }

  
    BOOL b = CloseHandle(stream->Handle);

    if(b==FALSE)
    {
    stream->IsError=1;
    free(stream);
    return -1;
    }

    if(stream==NULL)
    return -1;
    free(stream);
    
    return 0;
}

HANDLE so_fileno(SO_FILE *stream)
{
    if(stream!=NULL)
    return stream->Handle;

    else return INVALID_HANDLE_VALUE;
}

int so_fflush(SO_FILE *stream)
{
    if(stream==NULL)
    return -1;
  
    if(stream->LastOperation!=0 && stream->LastOperation!=-1 && stream->BufferCursor!=0 && stream->Flags!=1 && stream->Flags!=0) 
    {

        if(stream->IsOpenForAppend==1)
        {
            DWORD poz = SetFilePointer(stream->Handle,0,NULL,SEEK_END);
            if(poz==INVALID_SET_FILE_POINTER)
            {
                stream->IsError=1;
                return -1;
            }
        }

    int written = 0;
    BOOL a=0; 
    int writenNow=0;

    if(stream->Eof!=-1 && stream->BufferCursor>0)
    {
    while(written < stream->BufferCursor)
    {
    a = WriteFile(stream->Handle, (char*)(stream->Buffer+written), stream->BufferCursor-written,&writenNow,NULL);

    if(a==FALSE)
    {
        
        stream->Eof=-1;
        stream->IsError=1;
        return -1;
    }

   
    written+=writenNow;
    }
    }
    
   
    stream->BufferCursor=0;
    }
    return 0;
}

int so_ferror(SO_FILE *stream)
{
    return stream->IsError | stream->Eof;
}

int so_feof(SO_FILE *stream)
{
    return stream->Eof;
}

int so_fgetc(SO_FILE *stream)
{
     if(stream->Flags==3 || stream->Flags==5 || stream==NULL)
    {
        stream->IsError=1;
        stream->Eof=-1;
        return -1;
    }

   if(stream->LastOperation==1 || stream->LastOperation==-1 || stream->BufferCursor==BUFFER_SIZE || stream->LastOperation==2
    || stream->BufferCursor==0 || stream->BufferCursor==stream->BytesRead)
    {
        BOOL a=0;
        a = ReadFile(stream->Handle,(char*)stream->Buffer,BUFFER_SIZE,&stream->BytesRead,NULL);

        if(a==FALSE)
        {
            stream->Eof=-1;
            return SO_EOF;
        }

        if(GetLastError()==ERROR_BROKEN_PIPE)
         {
            stream->Eof=-1;
            return SO_EOF;
        }

        if(stream->BytesRead<=0)
        {
            stream->Eof=-1;
            return SO_EOF;
        }

        stream->BufferCursor=0;

        stream->LastOperation=0;
        stream->BufferCursor+=1;
        return (int)stream->Buffer[stream->BufferCursor-1];
    }

    else if(stream->LastOperation==0)
    {
    stream->BufferCursor+=1;
    return (int)stream->Buffer[stream->BufferCursor-1];
    }
    return 0;
}



int so_fputc(int c, SO_FILE *stream)
{
 if(stream==NULL)
    {
        return -1;
    }

     if(stream->Flags==1)
    {
        return -1;
    }

     if(stream->IsOpenForAppend==1)
    {
        stream->LastOperation=2;
    }
    else
    stream->LastOperation=1;


    if(stream->BufferCursor == BUFFER_SIZE)
    {
        int a = so_fflush(stream);

        if(a==-1)
        {
            stream->IsError=1;
            return -1;
        }
    }

    
    stream->Buffer[stream->BufferCursor]=c;
    stream->BufferCursor++;
    return c;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    if(stream==NULL)
    return -1;

    char *p = (char*)ptr;

    if(p==NULL)
    {
    stream->IsError=1;
    return -1;
    }

    size_t  count = 0;

    for(size_t  i = 0;i < nmemb * size; i++)
    {
        int a = so_fgetc(stream);

        if(so_feof(stream)==-1)
        break;

        p[i]=a;

        count++;
    }


    return count/size;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
     char *p = (char*)ptr;

    int count = 0;

    for(size_t i=0; i<nmemb*size;i++)
    {
        if(so_feof(stream)==-1)
        break;

        size_t a = so_fputc(p[i],stream);

        count++;    
    }

    return count/size;
}

long so_ftell(SO_FILE *stream)
{
    DWORD position = SetFilePointer(stream->Handle,0,NULL,FILE_CURRENT);

     if(position == -1)
    {
        return -1;
    }

    if(stream->LastOperation ==-1)
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
        int a;

        a = so_fflush(stream);

        if(a==-1)
        {
            return -1;
        }
    }
    else if(stream->LastOperation==0)
    {
        stream->BufferCursor=0;
    }

    int a=SetFilePointer(stream->Handle,offset,NULL,whence);

    if(a==INVALID_SET_FILE_POINTER)
    return -1;

    return 0;
}

SO_FILE *so_popen(const char *command, const char *type)
{
    SO_FILE *stream = NULL;

    stream = AllocFilePtr();

    if(stream==NULL)
    return NULL;

    if(strcmp(type,"r")==0)
    {
        stream->Flags=1;
    }
    else if(strcmp(type,"w")==0)
    {
        return NULL;
        stream->Flags=2;
    }

    else
    {
        free(stream);
        return NULL;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES si_pipe;
    DWORD dwRes;
    BOOL bRes;
    CHAR cmdLine[BUFFER_SIZE];

    strcpy(cmdLine,"");
    strcat(cmdLine, command);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    ZeroMemory(&pi, sizeof(pi));

    ZeroMemory(&si_pipe, sizeof(si_pipe));
    si_pipe.nLength = sizeof(si_pipe);

    si_pipe.bInheritHandle = TRUE;

    HANDLE hReadPipe, hWritePipe;
    BOOL a = CreatePipe(&hReadPipe, &hWritePipe, &si_pipe, 0);

    if(a==0)
    return NULL;

    if(stream->Flags==1)
    {
        si.hStdInput = hReadPipe;
        a = SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, 0);
        if(a==0)
    return NULL;
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        stream->Handle=hReadPipe;
    }
    else
    {
        si.hStdOutput = hWritePipe;
         a = SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);
             if(a==0)
    return NULL;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        stream->Handle=hWritePipe;
    }

    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags |= STARTF_USESTDHANDLES;


     bRes = CreateProcessA(
        NULL,           /* No module name (use command line) */
        cmdLine,        /* Command line */
        NULL,           /* Process handle not inheritable */
        NULL,           /* Thread handle not inheritable */
        TRUE,           /* Set handle inheritance to TRUE */
        0,              /* No creation flags */
        NULL,           /* Use parent's environment block */
        NULL,           /* Use parent's starting directory */
        &si,            /* Pointer to STARTUPINFO structure */
        &pi             /* Pointer to PROCESS_INFORMATION structure */
    );

    if(bRes==0)
    return NULL;

    stream->process=pi;

    return stream;
}

void CloseProcess(LPPROCESS_INFORMATION lppi) 
{
    CloseHandle(lppi->hThread);
    CloseHandle(lppi->hProcess);
}


int so_pclose(SO_FILE *stream)
{
    if(stream==NULL)
    return -1;


    DWORD dwRes;
    BOOL bRes;

    BOOL a= CloseHandle(stream->Handle);
    if(a==0)
    return -1;

    //dwRes = WaitForSingleObject(stream->process.hProcess, 5000);
    //if(dwRes== WAIT_TIMEOUT)
    //return -1;

    bRes = GetExitCodeProcess(stream->process.hThread, &dwRes);
    if(bRes==0)
    return -1;
    
    CloseProcess(&stream->process);

    return 0;

}
