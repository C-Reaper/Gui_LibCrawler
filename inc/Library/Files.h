#ifndef FILES_H
#define FILES_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "String.h"

#if defined __linux__
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <fcntl.h>
#elif defined _WINE || _WIN32
//#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#else
#error "Plattform not supported!"
#endif


typedef long long               FilesSize;
#define FILES_INVALID_SIZE      0xFFFFFFFFFFFFFFFF

#define FILES_INVALID           0
#define FILES_DIRECTORY         1
#define FILES_FILE              2
#define FILES_UNKOWNTYPE        3
#define FILES_MAX_PATH    4096
#define FILES_BUFFER_SIZE 8192

FilesSize Files_Size(char* Path){
    FILE* f = fopen(Path,"rb");
    if(f){
        fseek(f,0,SEEK_END);
        int Size = ftell(f);
        fseek(f,0,SEEK_SET);
        fclose(f);
        return Size;
    }
    return FILES_INVALID_SIZE;
}

char* Files_cwd() {
    char buffer[FILES_MAX_PATH];
    memset(buffer, 0, sizeof(buffer));

#if defined(_WIN32)
    if (_getcwd(buffer, sizeof(buffer)) == NULL)
        return NULL;
#else
    if (getcwd(buffer, sizeof(buffer)) == NULL)
        return NULL;
#endif

    return CStr_Cpy(buffer);
}
char Files_getType(char* Path) {
#if defined(_WIN32)
    struct _stat st;
    if (_stat(Path, &st) != 0)
        return FILES_INVALID;

    if (st.st_mode & _S_IFDIR)
        return FILES_DIRECTORY;
    if (st.st_mode & _S_IFREG)
        return FILES_FILE;
#else
    struct stat st;
    if (stat(Path, &st) != 0)
        return FILES_INVALID;

    if (S_ISDIR(st.st_mode))
        return FILES_DIRECTORY;
    if (S_ISREG(st.st_mode))
        return FILES_FILE;
#endif

    return FILES_UNKOWNTYPE;
}

void Files_Mkdir(const char* dir){
#if defined _WINE || _WIN32
    DWORD attr = GetFileAttributesA(dir);

    if (attr == INVALID_FILE_ATTRIBUTES) {
        if (CreateDirectoryA(dir, NULL)) {
            printf("[Files]: Mkdir -> Dir '%s' created!\n", dir);
        } else {
            printf("[Files]: Mkdir -> Error %lu\n", GetLastError());
        }
    } else if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        printf("[Files]: Mkdir -> Dir '%s' exists already!\n", dir);
    } else {
        printf("[Files]: Mkdir -> Path exists but is not a directory!\n");
    }
#elif defined __linux__
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0755) == 0) {
            printf("[Files]: Mkdir -> Dir '%s' created!\n", dir);
        } else {
            perror("[Files]: Mkdir -> Error during creation!");
        }
    } else {
        printf("[Files]: Mkdir -> Dir '%s' exists already!\n", dir);
    }
#else
#error "Plattform not supported!"
#endif
}

char Files_isDir(char* Path){
    return Files_getType(Path)==FILES_DIRECTORY;
}
char Files_isFile(char* Path){
    return Files_getType(Path)==FILES_FILE;
}

char* Files_Read(char* Path){
    FILE* f = fopen(Path,"rb");
    if(f){
        FilesSize Size = Files_Size(Path);
        char* Buffer = (char*)malloc(Size+1);
        memset(Buffer,0,Size+1);
        size_t out = fread(Buffer,sizeof(char),Size,f);
        while(out <= 0) out = fread(Buffer,sizeof(char),Size,f);
        fclose(f);
        return Buffer;
    }
    return NULL;
}
char* Files_ReadB(char* Path,FilesSize* size){
    FILE* f = fopen(Path,"rb");
    if(f){
        FilesSize Size = Files_Size(Path);
        char* Buffer = (char*)malloc(Size);
        memset(Buffer,0,Size);
        size_t out = fread(Buffer,sizeof(char),Size,f);
        while(out <= 0) out = fread(Buffer,sizeof(char),Size,f);
        *size = Size;
        fclose(f);
        return Buffer;
    }
    return NULL;
}

char* Files_ReadT(char* Path){
    FILE* f = fopen(Path,"r");
    if(f){
        FilesSize Size = Files_Size(Path);
        char* Buffer = (char*)malloc(Size+1);
        memset(Buffer,0,Size+1);
        size_t out = fread(Buffer,sizeof(char),Size,f);
        while(out <= 0) out = fread(Buffer,sizeof(char),Size,f);
        fclose(f);
        return Buffer;
    }
    return NULL;
}
char* Files_ReadTB(char* Path,FilesSize* size){
    FILE* f = fopen(Path,"r");
    if(f){
        FilesSize Size = Files_Size(Path);
        char* Buffer = (char*)malloc(Size);
        memset(Buffer,0,Size);
        size_t out = fread(Buffer,sizeof(char),Size,f);
        while(out <= 0) out = fread(Buffer,sizeof(char),Size,f);
        *size = Size;
        fclose(f);
        return Buffer;
    }
    return NULL;
}

void Files_Write(char* Path,void* Data,FilesSize Size){
    FILE* f = fopen(Path,"wb");
    if(f){
        if(Data && Size > 0LL) fwrite(Data,sizeof(char),Size,f);
        fclose(f);
    }
}
void Files_WriteT(char* Path,void* Data,FilesSize Size){
    FILE* f = fopen(Path,"w");
    if(f){
        fwrite(Data,sizeof(char),Size,f);
        fclose(f);
    }
}

char Files_Remove(char* Path){
    return remove(Path);
}

char Files_PathEndsWith(char* Path,char c){
    int s = CStr_Size(Path);
    return Path[s-1] == c;
}

char* Files_Path(char* Path){
    char* dir = CStr_ChopEndTo(Path,'/');
    return dir;
}
char* Files_NameFull(char* Path){
    char* name = CStr_ChopEndFrom(Path,'/');
    return name;
}
char* Files_Name(char* Path){
    char* name_type = CStr_ChopEndFrom(Path,'/');
    char* name = CStr_ChopEndTo(name_type,'.');

    if(name_type) free(name_type);
    return name;
}
char* Files_NameStep(char* Path,int step){
    int Size = CStr_Size(Path);
    char* out = NULL;
    for(int i = Size-1;i>=0;i--){
        if(Path[i]=='/'){
            step--;
            if(step < 0) return CStr_Cpy_From_To(Path,i + 1,Size);
        }
    }
    return CStr_Cpy(Path);
}
char* Files_Type(char* Path){
    char* t = CStr_ChopEndFrom(Path,'.');
    return t;
}
char* Files_UnixPath(char* Path){
    return CStr_ReplaceAll(Path,'\\','/');
}

int Files_StepBackPath(Vector* v,int* i,int rm){
    if(*i >= 0){
        CStr spcstr = *(CStr*)Vector_Get(v,*i - 1);

        if(CStr_Cmp(spcstr,"..")){
            rm = Files_StepBackPath(v,i,rm + 1);
            CStr_Free((CStr*)Vector_Get(v,*i));
            Vector_Remove(v,*i);
            (*i)--;
        }else{
            CStr_Free((CStr*)Vector_Get(v,*i));
            Vector_Remove(v,*i);
            (*i)--;
        }
        rm--;
    }
    return rm;
}
char* Files_CompressPath(char* Path){
    Vector split = CStr_ChopDown(Path,'/');

    for(int i = 0;i<split.size;i++){
        CStr spcstr = *(CStr*)Vector_Get(&split,i);
        
        if(CStr_Cmp(spcstr,".")){
            CStr_Free(&spcstr);
            Vector_Remove(&split,i);
            i--;
        }else if(CStr_Cmp(spcstr,"..")){
            Files_StepBackPath(&split,&i,1);
            CStr_Free((CStr*)Vector_Get(&split,i));
            Vector_Remove(&split,i);
            i--;
        }
    }

    String builder = String_New();

    for(int i = 0;i<split.size - 1;i++){
        CStr spcstr = *(CStr*)Vector_Get(&split,i);
        String_Append(&builder,spcstr);
        String_AppendChar(&builder,'/');
        CStr_Free(&spcstr);
    }
    if(split.size > 0){
        CStr spcstr = *(CStr*)Vector_Get(&split,split.size - 1);
        String_Append(&builder,spcstr);
        CStr_Free(&spcstr);
    }
    Vector_Free(&split);
    
    CStr out = String_CStr(&builder);
    String_Free(&builder);
    return out;
}
char* Files_FromPath(char* Path,char* relative){
    String builder = String_Make(Path);
    if (!Files_PathEndsWith(Path,'/')){
        String_AppendChar(&builder,'/');
    }

    String_Append(&builder,relative);
    
    CStr out = String_CStr(&builder);
    String_Free(&builder);

    CStr comp = Files_CompressPath(out);
    CStr_Free(&out);
    return comp;
}

char* Files_DirNameStepBack(char* Path,char* other,int step){
    char* found = Files_NameStep(other,step);
    char* add = Files_FromPath(Path,found);
    CStr_Free(&found);
    return add;
}


void Files_Walk(const char *base_path, void (*fn)(char* path)) {

#if defined(_WIN32)

    char search_path[FILES_MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", base_path);

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(search_path, &fd);

    if (h == INVALID_HANDLE_VALUE)
        return;

    do {
        if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
            continue;

        char full_path[FILES_MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s\\%s", base_path, fd.cFileName);

        fn(full_path);

    } while (FindNextFileA(h, &fd));

    FindClose(h);

#else

    DIR *dir = opendir(base_path);
    if (!dir) return;

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        char full_path[FILES_MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        fn(full_path);
    }

    closedir(dir);

#endif
}
void Files_WalkR(const char *base_path, void (*fn)(char* path)) {

#if defined(_WIN32)

    char search_path[FILES_MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", base_path);

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(search_path, &fd);
    if (h == INVALID_HANDLE_VALUE) return;

    do {
        if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
            continue;

        char full_path[FILES_MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s\\%s", base_path, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            fn(full_path);
            Files_WalkR(full_path, fn);
        }

    } while (FindNextFileA(h, &fd));

    FindClose(h);

#else

    DIR *dir = opendir(base_path);
    if (!dir) return;

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        char full_path[FILES_MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            fn(full_path);
            Files_WalkR(full_path, fn);
        }
    }

    closedir(dir);

#endif
}

char Files_HasSub(char *base_path, char *sub_path) {

#if defined(_WIN32)

    char search_path[FILES_MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", base_path);

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(search_path, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;

    do {
        if (!strcmp(fd.cFileName, sub_path)) {
            FindClose(h);
            return 1;
        }
    } while (FindNextFileA(h, &fd));

    FindClose(h);
    return 0;

#else

    DIR *dir = opendir(base_path);
    if (!dir) return 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, sub_path)) {
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;

#endif
}

#endif // !FILES_H