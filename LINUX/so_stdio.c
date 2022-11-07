#include "so_stdio.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define BUFFER_SIZE 4096

struct _so_file
{
    int FileDescriptor;
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

FUNC_DECL_PREFIX SO_FILE *so_fopen(const char *pathname, const char *mode)
{
    SO_FILE *FILE = NULL;

    FILE = AllocFilePtr();

    if(strcmp(mode,"r")==0)
    {
        FILE->FileDescriptor=open(pathname,O_RDONLY);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
    }

    else  if(strcmp(mode,"r+")==0)
    {
        FILE->FileDescriptor=open(pathname,O_RDWR);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
    }

    else  if(strcmp(mode,"w")==0)
    {
        FILE->FileDescriptor=open(pathname,O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
    }
    else if(strcmp(mode,"w+")==0)
    {
        FILE->FileDescriptor=open(pathname,O_RDWR | O_CREAT | O_TRUNC, 0644);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
    }

    else if(strcmp(mode,"a")==0)
    {
        FILE->FileDescriptor=open(pathname,O_APPEND | O_CREAT | O_WRONLY, 0644);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
         FILE->IsOpenForAppend=1;
    }

    else if(strcmp(mode,"a+")==0)
    {
        FILE->FileDescriptor=open(pathname,O_APPEND | O_CREAT | O_RDONLY | O_WRONLY, 0644);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
        FILE->IsOpenForAppend=1;
    }

    else
    {
        free(FILE);
        return NULL;
    }

    return FILE;
}

FUNC_DECL_PREFIX int so_fclose(SO_FILE *stream)
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
  
    int a = close(stream->FileDescriptor);
    if(a==-1)
    return -1;

    free(stream->Buffer);
    free(stream);
   

    return 1;
}

FUNC_DECL_PREFIX int so_fileno(SO_FILE *stream)
{
    return stream->FileDescriptor;
}

FUNC_DECL_PREFIX int so_fflush(SO_FILE *stream)
{
    if(stream->LastOperation!=0 && stream->LastOperation!=-1)
    {
        if(stream->IsOpenForAppend==1)
        {
            so_fseek(stream,0,SEEK_END);
        }

    int a = write(stream->FileDescriptor, stream->Buffer, stream->BufferCursor);

    if(a==-1)
    {
        stream->IsError=1;
        return -1;
    }
    stream->BufferCursor=0;
    }
}

FUNC_DECL_PREFIX int so_ferror(SO_FILE *stream)
{
    if(stream->IsError==1)
    return 1;

    return 0;
}

FUNC_DECL_PREFIX int so_feof(SO_FILE *stream)
{
    int currentPosition = lseek(stream->FileDescriptor,0,SEEK_CUR);
    if(currentPosition == -1)
    return -1;

    int eof = lseek(stream->FileDescriptor, 0, SEEK_END);
    if (eof==-1)
    return -1;



    int a = lseek(stream->FileDescriptor,currentPosition,SEEK_SET);
    if(a=-1)
    return -1;

    if(currentPosition == so_ftell(stream))
    return 1;

    return 0;
}

FUNC_DECL_PREFIX int so_fgetc(SO_FILE * stream)
{
    if(stream->LastOperation==1 || stream->LastOperation==-1 || stream->BufferCursor==BUFFER_SIZE-1 || stream->LastOperation==2)
    {
        stream->BufferCursor=0;
        stream->BytesRead = read(stream->FileDescriptor,stream->Buffer,BUFFER_SIZE);

        if(stream->BytesRead==0)
        {
            return SO_EOF;
        }

        if(stream->BytesRead==-1)
        {
            stream->IsError=1;
            return -1;
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

FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream)
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

    if(stream->IsOpenForAppend==1)
    {
        stream->LastOperation=2;
    }
    else
    stream->LastOperation=1;

    
    stream->Buffer[stream->BufferCursor]=c;
    stream->BufferCursor++;
}

FUNC_DECL_PREFIX size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
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

FUNC_DECL_PREFIX size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
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

FUNC_DECL_PREFIX long so_ftell(SO_FILE *stream)
{
    long position = lseek(stream->FileDescriptor,0,SEEK_CUR);

    if(position == -1)
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

FUNC_DECL_PREFIX int so_fseek(SO_FILE *stream, long offset, int whence)
{
    if(stream->LastOperation == 1 || stream->LastOperation == 2)
    {
        int a;
        if(stream->IsOpenForAppend!=1)
        {
        a = so_fflush(stream);
        }
        if(a==-1)
        {
            return -1;
        }
    }
    else if(stream->LastOperation==0)
    {
        stream->BufferCursor=0;
    }

    int a=lseek(stream->FileDescriptor,offset,whence);

    if(a==-1)
    return -1;

    return 0;
}

FUNC_DECL_PREFIX SO_FILE *so_popen(const char *command, const char *type)
{
    SO_FILE *FILE = AllocFilePtr();
    int flag=0;

    if(FILE == NULL)
    return NULL;

    if(strcmp(type,"r")==0)
    {
        flag = O_RDONLY;
    }
    else if(strcmp(type,"w")==0)
    {
        flag = O_WRONLY;
    }
    else
    {
        free(FILE);
        return NULL;
    }

    int fds[2];

    int a = pipe(fds);

    if(a!=0)
    {
        free(FILE);
        return NULL;
    }

    if(flag == O_RDONLY)
    FILE->FileDescriptor=fds[0];
    else
    FILE->FileDescriptor=fds[1];

    int pid = fork();

    if(pid==-1)
    {
        close(fds[0]);
		close(fds[1]);
		free(FILE);
        return NULL;
    }
    else if(pid == 0)
    {
        //copil
        if(flag == O_RDONLY)
        {
            close(fds[0]);
            dup2(fds[1],STDOUT_FILENO);
        }
        else
        {
            close(fds[1]);
            dup2(fds[0],STDOUT_FILENO);
        }

        a=execl("/bin/sh", "sh", "-c", command, (char *)0);

        if(a!=0)
        return NULL;
    }
    else 
    {
        FILE->pid = pid;
        if (flag == O_RDONLY)
			close(fds[1]);
		else
			close(fds[0]);

    }

    return FILE;
}

FUNC_DECL_PREFIX int so_pclose(SO_FILE *stream)
{
    if(stream->LastOperation == 1 || stream->LastOperation==2)
    {
        int a = so_fflush(stream);
        if(a==-1)
        {
            stream->IsError=1;
        }
    }

    int status;
    close(stream->FileDescriptor);
    int a = waitpid(stream->pid,&status,0);
    free(stream);

    if(a<0)
    return -1;

    return 0;
}