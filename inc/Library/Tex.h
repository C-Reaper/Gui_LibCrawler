#ifndef TEX_H
#define TEX_H

#include "../Container/Vector.h"
#include "../Container/Pair.h"

#include "ObjectNotation.h"
#include "ConstParser.h"
#include "String.h"

typedef struct SymbolInfo {
    unsigned int c;
} SymbolInfo;

typedef Vector HighLight;
typedef Vector Tex;

void Tex_SyncString(Tex* t,String* s){
    if(t->size!=s->size){
        if(t->size<s->size){
            int Delta = s->size - t->size;
            for(int i = 0;i<Delta;i++){
                SymbolInfo si = { 0xFFFFFFFF };
                Vector_Push(t,&si);
            }
        }else{
            int Delta = t->size - s->size;
            for(int i = 0;i<Delta;i++){
                Vector_PopTop(t);
            }
        }
    }
}

int HighLight_Key(HighLight* ps,char* cstr){
    for(int i = 0;i<ps->size;i++){
        Pair* p = (Pair*)Vector_Get(ps,i);
        if(CStr_Cmp(cstr,*(char**)Pair_First(p))){
            return i;
        }
    }
    return -1;
}
unsigned int HighLight_ColorOf(HighLight* ps,char* cstr){
    for(int i = 0;i<ps->size;i++){
        Pair* p = (Pair*)Vector_Get(ps,i);
        if(CStr_Cmp(cstr,*(char**)Pair_First(p))){
            Number n = Number_Parse((i8*)*(char**)Pair_Second(p));
            if(n!=NUMBER_PARSE_ERROR) return n;
        }
    }

    const int size = CStr_Size(cstr);
    if(size > 0){
        int key = -1;
        if(Char_Alpha(cstr[0])) key = HighLight_Key(ps,"#STR");
        if(Char_Num(cstr[0]))   key = HighLight_Key(ps,"#NUM");

        if(key>=0){
            Pair* p = (Pair*)Vector_Get(ps,key);
            Number n = Number_Parse((i8*)*(char**)Pair_Second(p));
            if(n!=NUMBER_PARSE_ERROR) return n;
        }
    }
    return 0xFFFFFFFF;
}
void HighLight_Set(HighLight* ps,char* Path){
    for(int i = 0;i<ps->size;i++){
        Pair* p = (Pair*)Vector_Get(ps,i);
        CStr_Free(Pair_First(p));
        CStr_Free(Pair_Second(p));
        Pair_Free(p);
    }
    Vector_Free(ps);
    
    *ps = Vector_New(sizeof(Pair));
    Table t = Table_New();
    Table_Parse(&t,Path);
    //Table_Print(&t);
    for(int i = 0;i<t.Root->Childs.size;i++){
        Branch* b = *(Branch**)Vector_Get(&t.Root->Childs,i);
        Entry* p = (Entry*)b->Memory;
        Vector_Push(ps,(Pair[]){ Pair_New(sizeof(char*),(char*[]){ CStr_Cpy(p->Name) },sizeof(char*),(char*[]){ CStr_Cpy(p->Value) }) });
    }
    Table_Free(&t);
}
void HighLight_Clear(HighLight* ps){
    for(int i = 0;i<ps->size;i++){
        Pair* p = (Pair*)Vector_Get(ps,i);
        CStr_Free(Pair_First(p));
        CStr_Free(Pair_Second(p));
        Pair_Free(p);
    }
    Vector_Free(ps);
    *ps = Vector_Null();
}
void HighLight_Tex_String(HighLight* ps,Tex* t,String* s){
    Tex_SyncString(t,s);

    char fcstr = 0;
    int Last = 0;
    for(int i = 0;i<s->size;i++){
        char c = *(char*)Vector_Get(s,i);
        
        if(Char_StringPart(c)){
            if(i!=s->size-1) continue;
            else             i++;
        }

        if(c=='\"'){
            if(!fcstr){
                char* cstr = String_SubCStr(s,Last,i-Last);
                unsigned int Color = HighLight_ColorOf(ps,cstr);
                for(int j = Last;j<i;j++){
                    SymbolInfo si = { Color };
                    Vector_Set(t,&si,j);
                }
                free(cstr);
                Last = i;

                fcstr = 1;
            }else{
                const int end = i+1;
                char* cstr = String_SubCStr(s,Last,end-Last);
                unsigned int Color = HighLight_ColorOf(ps,"#CSTR");
                for(int j = Last;j<end;j++){
                    SymbolInfo si = { Color };
                    Vector_Set(t,&si,j);
                }
                free(cstr);
                Last = end;

                fcstr = 0;
            }
        }else if(fcstr)
            continue;
        else if(c==' ' || c=='\n' || c=='\t' || (Char_StringPart(c) && i>=s->size-1)){
            char* cstr = String_SubCStr(s,Last,i-Last);
            unsigned int Color = HighLight_ColorOf(ps,cstr);
            for(int j = Last;j<i;j++){
                SymbolInfo si = { Color };
                Vector_Set(t,&si,j);
            }
            free(cstr);
            Last = i+1;
        }else{
            char* cstr = String_SubCStr(s,Last,i-Last);
            unsigned int Color = HighLight_ColorOf(ps,cstr);
            for(int j = Last;j<i;j++){
                SymbolInfo si = { Color };
                Vector_Set(t,&si,j);
            }
            free(cstr);

            cstr = String_SubCStr(s,i,1);
            Color = HighLight_ColorOf(ps,cstr);
            SymbolInfo si = { Color };
            Vector_Set(t,&si,i);
            free(cstr);

            Last = i+1;
        }
    }
}


typedef struct GHighLight {
    void* highlighter;
    void (*fn_init)(void*,char*);
    void (*fn_update)(void*,char*);
    void (*fn_free)(void*,char*);
} GHighLight;

GHighLight GHighLight_StdHL(){
    GHighLight ghl;

    return ghl;
}
void GHighLight_StdHL_Free(){
    
}

#endif //!TEX