#ifndef CSTR_G_H
#define CSTR_G_H

#include "Char_G.h"
#include "CStr.h"
#include "String.h"

void CStr_RenderAlxFont(unsigned int* Target,int Target_Width,int Target_Height,AlxFont* f,unsigned char* cstr,float x,float y,unsigned int Color){
    int Size = CStr_Size((char*)cstr);
    for(int i = 0;i<Size;i++){
        Char_RenderAlxFont(Target,Target_Width,Target_Height,f,cstr[i],x+i*f->CharSizeX,y,Color);
    }
}
void CStr_RenderSizeAlxFont(unsigned int* Target,int Target_Width,int Target_Height,AlxFont* f,unsigned char* cstr,int Size,float x,float y,unsigned int Color){
    for(int i = 0;i<Size;i++){
        Char_RenderAlxFont(Target,Target_Width,Target_Height,f,cstr[i],x+i*f->CharSizeX,y,Color);
    }
}
void CStr_RenderAlxFontf(unsigned int* Target,int Target_Width,int Target_Height,AlxFont* f,float x,float y,unsigned int Color,unsigned char* FormatCStr,...){
    va_list args;
    va_start(args,FormatCStr);
    String s = String_FormatA(FormatCStr,args);
    va_end(args);
    CStr_RenderSizeAlxFont(Target,Target_Width,Target_Height,f,s.Memory,s.size,x,y,Color);
    String_Free(&s);
}

#endif//!CSTR_G_H