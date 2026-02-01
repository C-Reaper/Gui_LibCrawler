#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#if defined __linux__
#elif defined _WIN32
#include <windows.h>
#else
#error "Platform not supported!"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Clipboard_Cpy(char* cstr,int size){
    #if defined __linux__
    
    FILE *pipe = popen("xclip -selection clipboard", "w");
    if (pipe) {
        fwrite(cstr,1,size,pipe);
        pclose(pipe);
    }
    
    #elif defined _WIN32
    
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE,size);
    memcpy(GlobalLock(hMem),cstr,size);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT,hMem);
    CloseClipboard();
    
    #else
    #error "Platform not supported!"
    #endif
}
char* Clipboard_Paste(){
    #if defined __linux__
    
    FILE *pipe = popen("xclip -selection clipboard -o", "r");
    if(!pipe) return NULL;

    char* buffer = NULL;
    size_t size = 0;
    char tmp[256];

    while(!feof(pipe)){
        size_t n = fread(tmp,1,sizeof(tmp),pipe);
        buffer = realloc(buffer,size + n + 1);
        memcpy(buffer + size, tmp, n);
        size += n;
    }
    if(buffer) buffer[size] = '\0';
    pclose(pipe);
    return buffer;
    
    #elif defined _WIN32
    
    char* buffer = NULL;
    if(OpenClipboard(NULL)){
        HANDLE hData = GetClipboardData(CF_TEXT);
        if(hData!=NULL){
            buffer = (char*)GlobalLock(hData);
            GlobalUnlock(hData);
        }
        CloseClipboard();
    }
    return buffer;
    
    #else
    #error "Platform not supported!"
    #endif
}

#endif
