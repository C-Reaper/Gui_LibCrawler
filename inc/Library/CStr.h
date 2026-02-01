#ifndef CSTR_H
#define CSTR_H

#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "../Container/Vector.h"
#include "Char.h"

#define False       0
#define True        1

typedef char* CStr;
typedef short* WCStr;

#ifdef __GNUC__
#define CStr_A      __attribute__((cleanup(CStr_Free))) CStr
#define WCStr_A     __attribute__((cleanup(CStr_Free))) WCStr
#endif //__GNUC__

char* CStr_New(int Length){
    return (char*)malloc(Length);
}
int CStr_Size(char* cstr){
    if(!cstr) return 0;
    for(int i = 0;;i++){
        if(cstr[i]==0) return i;
    }
    return -1;
}
char* CStr_Cpy(char* cstr){
    if(!cstr) return NULL;
    
    int Size = CStr_Size(cstr) + 1;
    if(Size <= 0) return NULL;
    
    char* out = (char*)malloc(Size);
    memcpy(out,cstr,Size);
    return out;
}
char* CStr_Cpy_From_To(char* cstr,int Begin,int End){
    int Size = End-Begin;
    char* out = (char*)malloc(Size+1);
    memcpy(out,cstr + Begin,Size);
    out[Size] = '\0';
    return out;
}
char CStr_Cmp(char* cstr1,char* cstr2){
    if(cstr1==NULL || cstr2==NULL)  return 0;
    
    int Size1 = CStr_Size(cstr1);
    int Size2 = CStr_Size(cstr2);
    if(Size1!=Size2) return 0;
    for(int i = 0;i<Size1;i++){
        if(cstr1[i]!=cstr2[i]) return 0;
    }
    return 1;
}
char CStr_CmpSize(char* cstr1,char* cstr2,int size){
    if(cstr1==NULL || cstr2==NULL)  return 0;
    
    int Size1 = CStr_Size(cstr1);
    int Size2 = CStr_Size(cstr2);
    if(Size1 < size || Size2 < size) return 0;

    for(int i = 0;i<size;i++){
        if(cstr1[i]!=cstr2[i]) return 0;
    }
    return 1;
}
char CStr_CmpLike(char* cstr,char* cstrdc,Char dc){
    if(cstr==NULL || cstrdc==NULL)  return 0;
    
    int Size1 = CStr_Size(cstr);
    int Size2 = CStr_Size(cstrdc);

    int Offset = 0;
    int LastIndex = 0;
    char LastChar = 0;
    for(int i = 0;i<Size1;i++){
        if(LastChar==0){
            if(i-Offset>=Size2) return 0;

            if(cstrdc[i-Offset]==dc){
                LastIndex = i;
                if(i-Offset+1<Size2)    LastChar = cstrdc[i-Offset + 1];
                else                    LastChar = 1;
                i--;
            }else if(cstr[i]!=cstrdc[i-Offset]) return 0;
        }else{
            if(cstr[i]==LastChar){
                Offset += i - LastIndex - 1;
                LastChar = 0;
                i--;
            }
        }
    }

    if(LastChar!=0 && LastChar!=1) return 0;
    return 1;
}
char CStr_Great(char* cstr1,char* cstr2){
    int Size1 = CStr_Size(cstr1);
    int Size2 = CStr_Size(cstr2);
    for(int i = 0;i<Size1 && i<Size2;i++){
        if(cstr1[i]==cstr2[i]) continue;
        if(cstr1[i]>cstr2[i])  return 1;
        else                   return 0;
    }
    if(Size1>Size2) return 1;
    return 0;
}
char CStr_Less(char* cstr1,char* cstr2){
    int Size1 = CStr_Size(cstr1);
    int Size2 = CStr_Size(cstr2);
    for(int i = 0;i<Size1 && i<Size2;i++){
        if(cstr1[i]==cstr2[i]) continue;
        if(cstr1[i]<cstr2[i])  return 1;
        else                   return 0;
    }
    if(Size1<Size2) return 1;
    return 0;
}
char* CStr_ChopTo(char* cstr,char c){
    int Size = CStr_Size(cstr);
    char* out = NULL;
    for(int i = 0;i<Size;i++){
        if(cstr[i]==c){
            out = (char*)malloc(i+1);
            out[i] = '\0';
            memcpy(out,cstr,i);
            return out;
        }
    }
    out = (char*)malloc(Size+1);
    memcpy(out,cstr,Size+1);
    return out;
}
char* CStr_ChopFrom(char* cstr,char c){
    int Size = CStr_Size(cstr);
    char* out = NULL;
    for(int i = 0;i<Size;i++){
        if(cstr[i]==c){
            out = (char*)calloc(Size-i+1,sizeof(char));
            memcpy(out,cstr+i+1,Size-i);
            return out;
        }
    }
    out = (char*)calloc(Size+1,sizeof(char));
    memcpy(out,cstr,Size+1);
    return out;
}

int CStr_Find(char* cstr,char c){
    int size = CStr_Size(cstr);
    for(int i = 0;i<size;i++){
        if(cstr[i]==c) return i;
    }
    return -1;
}

char* CStr_ChopFromStack(char* cstr,char c){
    int Size = CStr_Size(cstr);
    for(int i = 0;i<Size;i++){
        if(cstr[i]==c){
            return (char*)(cstr + i);
        }
    }
    return cstr;
}

char* CStr_ChopEndTo(char* cstr,char c){
    int Size = CStr_Size(cstr);
    char* out = NULL;
    for(int i = Size-1;i>=0;i--){
        if(cstr[i]==c){
            out = (char*)malloc(i+1);
            out[i] = '\0';
            memcpy(out,cstr,i);
            return out;
        }
    }
    out = (char*)malloc(Size+1);
    memcpy(out,cstr,Size+1);
    return out;
}
char* CStr_ChopEndFrom(char* cstr,char c){
    int Size = CStr_Size(cstr);
    char* out = NULL;
    for(int i = Size-1;i>=0;i--){
        if(cstr[i]==c){
            out = (char*)calloc(Size-i+1,sizeof(char));
            memcpy(out,cstr+i+1,Size-i);
            return out;
        }
    }
    out = (char*)calloc(Size+1,sizeof(char));
    memcpy(out,cstr,Size+1);
    return out;
}
Vector CStr_ChopDown(char* cstr,char c){
    int Size = CStr_Size(cstr);
    
    Vector out = Vector_New(sizeof(char*));

    int Last = 0;
    for(int i = 0;i<Size;i++){
        if(cstr[i]==c){
            char* add = CStr_Cpy_From_To(cstr,Last,i);
            Vector_Push(&out,&add);
            Last = i+1;
        }
        if(i==Size-1){
            char* add = CStr_Cpy_From_To(cstr,Last,i+1);
            Vector_Push(&out,&add);
        }
    }
    return out;
}
int CStr_CountOf(char* s,char c){
    int Size = CStr_Size(s);
    int Count = 0;
    for(int i = 0;i<Size;i++){
        if(s[i]==c) Count++;
    }
    return Count;
}
char* CStr_ReplaceAll(char* s,char src,char dst){
    const int Size = CStr_Size(s);
    CStr cpy = CStr_Cpy(s);

    for(int i = 0;i<Size;i++){
        if(cpy[i]==src) cpy[i] = dst;
    }

    return cpy;
}

char* CStr_Sub(char* cstr1,unsigned int s,unsigned int e){
    int size = CStr_Size(cstr1);
    if(s>=e || e>size) return NULL;

    char* ret = (char*)malloc((e - s + 1) * sizeof(char));
    memcpy(ret,cstr1 + s,e - s);
    ret[(e - s)] = '\0';
    return ret;
}

int CStr_WordAt_Start(char* s,int index){
    if(!s) return 0;
    
    int size = CStr_Size(s);
    if(index>=size) return size;
    
    for(;index>=0 && Char_StringPart(s[index]);index--){}
    return index + 1;
}
int CStr_WordAt_End(char* s,int index){
    if(!s) return 0;
    
    int size = CStr_Size(s);
    for(;index<size && s[index]>' ';index++){}
    return index - 1;
}

char* CStr_Concat(char* cstr1,char* cstr2){
    int size1 = CStr_Size(cstr1);
    int size2 = CStr_Size(cstr2);
    CStr cstr = CStr_New(size1 + size2 + 1);
    memcpy(cstr,cstr1,size1);
    memcpy(cstr + size1,cstr2,size2);
    cstr[size1 + size2] = '\0';
    return cstr;
}
char* CStr_PopOff(char* cstr1){
    int size = CStr_Size(cstr1);
    CStr cstr = CStr_New(size);
    memcpy(cstr,cstr1,size - 1);
    cstr[size - 1] = '\0';
    return cstr;
}

unsigned long long CStr_SimpleHash(char* cstr){
    int len = CStr_Size(cstr);
    unsigned long long hash = 0;
    for(int i = 0;i<len;i++){
        hash += cstr[i];
    }
    return hash;
}
void CStr_Free(CStr* cstr){
    if(!cstr) return;
    if(*cstr) free(*cstr);
    *cstr = NULL;
}

void CStr_Set(char** cstr,char* setter){
    CStr_Free(cstr);
    *cstr = CStr_Cpy(setter);
}

char CStr_PCmp(CStr* cstr1,CStr* cstr2){
    return CStr_Cmp(*cstr1,*cstr2);
}
void CStr_print(CStr* cstr){
    printf("%s",*cstr);
}

int WCStr_Size(wchar_t* cstr){
    for(int i = 0;;i++){
        if(cstr[i]==0) return i;
    }
    return -1;
}

#endif //!CSTR_H