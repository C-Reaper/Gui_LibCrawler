#ifndef STRING_H
#define STRING_H

#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "../Container/Vector.h"
#include "ConstParser.h"
#include "CStr.h"

typedef Vector String;

#ifdef __GNUC__
#define String_A    __attribute__((cleanup(String_Free))) String
#endif //__GNUC__

String String_New(){
    String s = Vector_New(sizeof(char));
    return s;
}
String String_Null(){
    String s = Vector_Null();
    return s;
}

char String_Get(String* s,int Index){
    if(s && s->Memory) return ((char*)s->Memory)[Index];
    return -1;
}
void String_Set(String* s,char c,int Index){
    ((char*)s->Memory)[Index] = c;
}
void String_Append(String* s,char* cstr){
    if(cstr){
        int Size = CStr_Size(cstr);
        for(int i = 0;i<Size;i++){
            Vector_Push(s,(char*)cstr+i);
        }
    }
}
void String_AppendChar(String* s,char app){
    Vector_Push(s,&app);
}
void String_AppendString(String* s,String* app){
    for(int i = 0;i<app->size;i++){
        String_AppendChar(s,((char*)app->Memory)[i]);
    }
}

void String_AppendNumber(String* s,Number n){
    CStr d = (CStr)Number_Get(n);
    String_Append(s,d);
    CStr_Free(&d);
}
void String_AppendDouble(String* s,Double n){
    CStr d = (CStr)Double_Get(n,8);
    String_Append(s,d);
    CStr_Free(&d);
}
void String_AppendBoolean(String* s,Boolean n){
    CStr d = (CStr)Boolean_Get(n);
    String_Append(s,d);
    CStr_Free(&d);
}

String String_Make(char* cstr){
    String s = Vector_New(sizeof(char));
    String_Append(&s,cstr);
    return s;
}
String String_Cpy(String* str){
    String s = Vector_New(sizeof(char));
    String_AppendString(&s,str);
    return s;
}
char* String_CStr(String* s){
    if(s->size==0) return NULL;
    char* cstr = (char*)malloc(s->size + 1);
    memcpy(cstr,s->Memory,s->size);
    cstr[s->size] = '\0';
    return cstr;
}
char* String_SubCStr(String* s,int Start,int Length){
    if(Start+Length>s->size) Length = s->size - Start;
    char* cstr = (char*)malloc(Length+1);
    memcpy(cstr,(char*)s->Memory + Start,Length);
    cstr[Length] = '\0';
    return cstr;
}
void String_RemoveCount(String* s,unsigned int Index,unsigned int Count){
    Vector_RemoveCount(s,Index,Count);
}
void String_Add(String* s,char* cstr,unsigned int Index){
    int Size = CStr_Size(cstr);
    for(int i = 0;i<Size;i++){
        Vector_Add(s,cstr+i,Index+i);
    }
}
void String_AddChar(String* s,char c,unsigned int Index){
    Vector_Add(s,&c,Index);
}
void String_Replace(String* s,CStr replaced,CStr replacer){
    int size = CStr_Size(replaced);

    for(int i = 0;i<s->size;i++){
        char c = String_Get(s,i);
        if(c==replaced[0]){
            for(int j = 0;j<size;j++){
                if(c!=replaced[j]) break;
                if(j==size-1){
                    String_RemoveCount(s,i,size);
                    String_Add(s,replacer,i);
                }
            }
        }
    }
}
void String_Pop(String* s){
    Vector_PopTop(s);
}
void String_Remove(String* s,unsigned int Index){
    Vector_Remove(s,Index);
}
void String_Clear(String* s){
    Vector_Clear(s);
}
void String_Free(String* s){
    Vector_Free(s);
}

void String_SetCStr(String* s,char* setter){
    String_Clear(s);
    String_Append(s,setter);
}
void String_SetS(String* s,String* setter){
    String_Clear(s);
    String_AppendString(s,setter);
}

void String_CompressEsc(String* s){
    for(int i = 0;i<s->size-1;i++){
        char ch = String_Get(s,i);

        if(ch=='\\'){
            char ind = String_Get(s,i+1);
            if(ind=='n'){
                String_Set(s,'\n',i);
                String_Remove(s,i+1);
            }else if(ind=='t'){
                String_Set(s,'\t',i);
                String_Remove(s,i+1);
            }else if(ind=='r'){
                String_Set(s,'\r',i);
                String_Remove(s,i+1);
            }else if(ind=='b'){
                String_Set(s,'\b',i);
                String_Remove(s,i+1);
            }else if(ind=='\\'){
                String_Set(s,'\\',i);
                String_Remove(s,i+1);
            }else if(ind=='f'){
                String_Set(s,'\f',i);
                String_Remove(s,i+1);
            }else if(ind=='v'){
                String_Set(s,'\v',i);
                String_Remove(s,i+1);
            }else if(ind=='0'){
                String_Set(s,'\0',i);
                String_Remove(s,i+1);
            }else if(ind=='x'){
                const char hex0 = Char_IntofHex(String_Get(s,i+2));
                const char hex1 = Char_IntofHex(String_Get(s,i+3));
                const int num = (hex0 << 4) | hex1;
                String_Set(s,num,i);
                String_Remove(s,i+3);
                String_Remove(s,i+2);
                String_Remove(s,i+1);
            }else if(ind=='a'){
                String_Set(s,'\a',i);
                String_Remove(s,i+1);
            }else if(ind=='e'){
                String_Set(s,'\e',i);
                String_Remove(s,i+1);
            }else if(ind=='\''){
                String_Set(s,'\'',i);
                String_Remove(s,i+1);
            }else if(ind=='\"'){
                String_Set(s,'\"',i);
                String_Remove(s,i+1);
            }
        }
    }
}

String String_FormatA(char* FormatCStr,va_list args){
    String s = String_New();
    int Size = CStr_Size(FormatCStr);
    //int Count = CStr_CountOf(FormatCStr,'%');
    
    for(int i = 0;i<Size;i++){
        char c = FormatCStr[i];
        if(c=='%'){
            i++;
            char fc = FormatCStr[i];
            if(fc=='s'){
                char* out = va_arg(args,char*);
                if(out)    String_Append(&s,out);
                else       String_Append(&s,(char*)"NULL");
            }else if(fc=='c'){
                //char c = va_arg(args,char);
                int out = va_arg(args,int);
                String_AppendChar(&s,out);
            }else if(fc=='d'){
                Number out = va_arg(args,Number);
                String_AppendNumber(&s,out);
            }else if(fc=='f'){
                Double out = va_arg(args,Double);
                String_AppendDouble(&s,out);
            }else if(fc=='b'){
                Boolean out = (Boolean)va_arg(args,int);
                String_AppendBoolean(&s,out);
            }else if(fc=='x'){
                Number out = va_arg(args,Number);
                CStr app = U64_Get_X(out);
                String_Append(&s,app);
                CStr_Free(&app);
            }
        }else{
            String_AppendChar(&s,c);
        }
    }

    return s;
}
String String_Format(char* FormatCStr,...){
    va_list args;
    va_start(args,FormatCStr);
    String s = String_FormatA(FormatCStr,args);
    va_end(args);
    return s;
}

CStr CStr_FormatA(char* FormatCStr,va_list args){
    String s = String_FormatA(FormatCStr,args);
    CStr ret = String_CStr(&s);
    String_Free(&s);
    return ret;
}
CStr CStr_Format(char* FormatCStr,...){
    va_list args;
    va_start(args,FormatCStr);
    String s = String_FormatA(FormatCStr,args);
    va_end(args);

    CStr ret = String_CStr(&s);
    String_Free(&s);
    return ret;
}

void String_Printf(int (*Print)(char*),char* FormatCStr,...){
    va_list args;
    va_start(args,FormatCStr);
    String s = String_FormatA(FormatCStr,args);
    va_end(args);

    char* cstr = String_CStr(&s);
    Print(cstr);
    free(cstr);
    String_Free(&s);
}
void String_Appendf(String* s,char* FormatCStr,...){
    va_list args;
    va_start(args,FormatCStr);
    String app = String_FormatA(FormatCStr,args);
    va_end(args);

    String_AppendString(s,&app);
    String_Free(&app);
}
void String_RemoveAll(String* s,char c){
    for(int i = 0;i<s->size;i++){
        if(String_Get(s,i)==c){
            String_Remove(s,i);
            i--;
        }
    }
}
int String_TransformX(String* s,int Pos){
    int CurserX = 0;
    for(int i = Pos-1;i>=0;i--){
        if(String_Get(s,i)=='\n' || i==0){
            CurserX = Pos - i - 1;
            if(i==0) CurserX++;
            break;
        }
    }
    return CurserX;
}
int String_TransformY(String* s,int Pos){
    int Lines = 0;
    for(int i = 0;i<Pos;i++){
        if(String_Get(s,i)=='\n'){
            Lines++;
        }
    }
    return Lines;
}

int String_FirstCharOfLineLast(String* s,int Line,int FirstCharBefore,int LastCharBefore,int LineBefore){
    if(Line==0)             return 0;
    if(Line==LineBefore+1)  return LastCharBefore+1;
    int Lines = LineBefore+1;
    for(int i = LastCharBefore+1;i<s->size;i++){
        char c = String_Get(s,i);
        if(c=='\n' || i==s->size-1){
            Lines++;
            if(Lines==Line) return i+1;
        }
    }
    return s->size;
}
int String_LastCharOfLineLast(String* s,int Line,int FirstCharBefore,int LastCharBefore,int LineBefore){
    if(Line==LineBefore) return LastCharBefore;
    int Lines = LineBefore+1;
    for(int i = LastCharBefore+1;i<s->size;i++){
        char c = String_Get(s,i);
        if(c=='\n'){
            if(Lines==Line) return i;
            Lines++;
        }
        if(i==s->size-1){
            if(Lines==Line) return i+1;
            Lines++;
        }
    }
    return s->size;
}

int String_FirstCharOfLine(String* s,int Line){
    return String_FirstCharOfLineLast(s,Line,-1,-1,-1);
}
int String_LastCharOfLine(String* s,int Line){
    return String_LastCharOfLineLast(s,Line,-1,-1,-1);
}

int String_CharsInLine(String* s,int Line){
    int CharBegin = String_FirstCharOfLine(s,Line);
    int i = CharBegin;
    for(;i<s->size;i++){
        char c = String_Get(s,i);
        if(c=='\n') break;
    }
    return i - CharBegin;
}
int String_LineOfChar(String* s,int Char){
    int Lines = 0;
    int i = 0;
    for(;i<s->size && i<Char;i++){
        char c = String_Get(s,i);
        if(c=='\n'){
            Lines++;
        }
    }
    return Lines;
}

#endif