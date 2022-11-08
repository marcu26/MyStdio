#include "so_stdio.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define BUFFER_SIZE 4096
#define PIPE_READ 0
#define PIPE_WRITE 1

struct _so_file
{
    int FileDescriptor;
    char Buffer[BUFFER_SIZE];
    int BufferCursor;
    int IsError;
    int LastOperation; //-1 = nu a fost o alta operatie, 0 = a fost read, 1 = a fost write, 2 = a fost append
    int BytesRead;
    int IsOpenForAppend;
    int pid;
    int Flags; // 1=r, 2=r+, 3=w, 4=w+, 5=a, 6=a+;
    int EOF;
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
    FILE->EOF=0;
    FILE->pid=-1;
    FILE->FileDescriptor=-1;
    FILE->BytesRead=0;

    return FILE;
}

FUNC_DECL_PREFIX SO_FILE *so_fopen(const char *pathname, const char *mode)
{
    SO_FILE *FILE = NULL;

    FILE = AllocFilePtr();

    if(FILE==NULL)
    return NULL;

    if(strcmp(mode,"r")==0)
    {
        FILE->FileDescriptor=open(pathname,O_RDONLY);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
        FILE->Flags=1;
    }

    else  if(strcmp(mode,"r+")==0)
    {
        FILE->FileDescriptor=open(pathname,O_RDWR);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
        FILE->Flags=2;
    }

    else  if(strcmp(mode,"w")==0)
    {
        FILE->FileDescriptor=open(pathname,O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
        FILE->Flags=3;
    }
    else if(strcmp(mode,"w+")==0)
    {
        FILE->FileDescriptor=open(pathname,O_RDWR | O_CREAT | O_TRUNC, 0644);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
        FILE->Flags=4;
    }

    else if(strcmp(mode,"a")==0)
    {
        FILE->FileDescriptor=open(pathname,O_APPEND | O_CREAT | O_WRONLY, 0644);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
         FILE->Flags=5;
         FILE->IsOpenForAppend=1;
    }

    else if(strcmp(mode,"a+")==0)
    {
        FILE->FileDescriptor=open(pathname,O_APPEND | O_CREAT | O_RDWR, 0644);
        if(FILE->FileDescriptor==-1)
        {
            free(FILE);
            return NULL;
        }
        FILE->IsOpenForAppend=1;
        FILE->Flags=6;
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

    if(stream==NULL)
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

  
    a = close(stream->FileDescriptor);

    if(a==-1)
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

FUNC_DECL_PREFIX int so_fileno(SO_FILE *stream)
{
    if(stream!=NULL)
    return stream->FileDescriptor;

    else return -1;
}


FUNC_DECL_PREFIX int so_fflush(SO_FILE *stream)
{
    if(stream==NULL)
    return -1;
  
    if(stream->LastOperation!=0 && stream->LastOperation!=-1 && stream->BufferCursor!=0 && stream->Flags!=1 && stream->Flags!=0) 
    {

        if(stream->IsOpenForAppend==1)
        {
            off_t poz = lseek(stream->FileDescriptor,0,SEEK_END);
            if(poz==-1)
            {
                stream->IsError=1;
                return -1;
            }
        }

    ssize_t written = 0;
    ssize_t a = 0;
    

    if(stream->EOF!=-1 && stream->BufferCursor>0)
    {
    while(written < stream->BufferCursor)
    {
    a = write(stream->FileDescriptor, stream->Buffer+written, stream->BufferCursor-written);
    if(a<=0)
    {
        stream->EOF=SO_EOF;
        stream->IsError=1;
        return -1;
    }
    written+=a;
    }
    }
    
   
    stream->BufferCursor=0;
    }
    return 0;
}

FUNC_DECL_PREFIX int so_ferror(SO_FILE *stream)
{
    return stream->IsError | stream->EOF;
}

FUNC_DECL_PREFIX int so_feof(SO_FILE *stream)
{
    return stream->EOF;
}

FUNC_DECL_PREFIX int so_fgetc(SO_FILE * stream)
{
    if(stream->Flags==3 || stream->Flags==5 || stream==NULL)
    {
        return -1;
    }

    if(stream->LastOperation==1 || stream->LastOperation==-1 || stream->BufferCursor==BUFFER_SIZE || stream->LastOperation==2 || stream->BufferCursor==0 || stream->BufferCursor==stream->BytesRead)
    {
        ssize_t a=read(stream->FileDescriptor,stream->Buffer,BUFFER_SIZE);

        if(a<=0)
        {
            stream->EOF=-1;
            return SO_EOF;
        }

        stream->BytesRead=a;
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

FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream)
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

FUNC_DECL_PREFIX size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
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

FUNC_DECL_PREFIX size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
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

FUNC_DECL_PREFIX long so_ftell(SO_FILE *stream)
{
    off_t position = lseek(stream->FileDescriptor,0,SEEK_CUR);

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

FUNC_DECL_PREFIX int so_fseek(SO_FILE *stream, long offset, int whence)
{
    if(stream->LastOperation == 1 || stream->LastOperation == 2)
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

    int a=lseek(stream->FileDescriptor,offset,whence);

  

    if(a==-1)
    return -1;

    return 0;
}

FUNC_DECL_PREFIX SO_FILE *so_popen(const char *command, const char *type)
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
        stream->Flags=2;
    }

    else
    {
        free(stream);
        return NULL;
    }

    int fds[2];

	int a = pipe(fds);
	if (a != 0) {
		free(stream);
		return NULL;
	}

    if(stream->Flags==1)
    {
        stream->FileDescriptor = fds[PIPE_READ];
    }

    else
    {
        stream->FileDescriptor = fds[PIPE_WRITE]; 
    }

    int pid = fork();

    if(pid==-1)
    {
        close(fds[PIPE_READ]);
		close(fds[PIPE_WRITE]);
		free(stream);

		return NULL;
    }

    else if(pid==0)
    {
        if(stream->Flags==1)
        {
            close(fds[PIPE_READ]);
			dup2(fds[PIPE_WRITE], STDOUT_FILENO);
        }
        else
        {
            close(fds[PIPE_WRITE]);
			dup2(fds[PIPE_READ], STDIN_FILENO);
        }

        int a = execl("/bin/sh", "sh", "-c", command, (char *)0);
		if (a)
		return NULL;
    }

    else
    {
        stream->pid = pid;

		if (stream->Flags == 1)
			close(fds[PIPE_WRITE]);
		else
			close(fds[PIPE_READ]);
    }

    return stream;

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