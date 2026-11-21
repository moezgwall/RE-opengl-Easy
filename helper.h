#ifndef __HELPER_H 
#define __HELPER_H 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UNUSED(X) (void)(X) 
#define TODO(X) printf("[TODO] : %s @line:%d,@file:%s",#X,__LINE__,__FILE__)
#define ADD_NOTE(comment) mark_sus(__FILE__,__LINE__ + 1,comment)

void mark_sus(const char* fpath,int line,const char* comment);
char* shift_args_next(int* argc,char*** argv); 


#ifdef HELPER_IMPLEMENTATION 


void mark_sus(const char* fpath, int line, const char* comment){
    FILE* current_file = fopen(fpath,"r"); 
    FILE* temp = fopen("temp.c","w");
    if (!current_file || !temp){
        if (!current_file) fclose(current_file);
        if (!temp)  fclose(temp);
        return;
    }
    
    char buffer[1024];
    int cline = 1; 

    while (fgets(buffer,sizeof(buffer),current_file)){
        if (cline == line && !strstr(buffer,"// SUS_CODE")){
            #ifdef __WIN32__
            buffer[strcspn(buffer,"\r\n")] = '\0';
            #else
            buffer[strcspn(buffer,"\n")] = '\0';
            #endif
            fprintf(temp,"%s // SUS_CODE: %s \n",buffer,comment);
        }else{
            fputs(buffer,temp);
        }
        cline++;
    }
    fclose(current_file);
    fclose(temp);

    remove(fpath);
    if (rename("temp.c",fpath) !=0){
        perror("failed to replace original file");
    }
}

char* shift_args_next(int* argc,char*** argv){
    if ((*argc) <= 1) return NULL;
    (*argc)--;
    (*argv)++;
    return **argv;
}



#endif // HELPER_IMPLEMENTATION

#endif // __HELPER_H 
