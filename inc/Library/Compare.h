#ifndef COMPARE_H
#define COMPARE_H

#ifndef STD_STDIO_H
    #define STD_STDIO_H
    #include <stdio.h>
#endif // !STD_STDIO_H
#ifndef STD_STRING_H
    #define STD_STRING_H
    #include <string.h>
#endif // !STD_STRING_H
#ifndef STD_STDLIB_H
    #define STD_STDLIB_H
    #include <stdlib.h>
#endif // !STD_STDLIB_H

typedef char CMP_BOOL;

#define CMP_NOTHING     0b0
#define CMP_EQUAL       0b1
#define CMP_GREATER     0b10
#define CMP_LESS        0b100

CMP_BOOL Cmp_Dir_Bytewise(size_t ElementSize,void* e1,void* e2){
    char* Item1 = (char*)e1;
    char* Item2 = (char*)e2;
    for(int i = 0;i<ElementSize;i++){
        if(Item1[i]==Item2[i]){
            continue;
        }else return CMP_NOTHING;
    }
    return CMP_EQUAL;
}

CMP_BOOL Cmp_Bytewise(size_t ElementSize,void* e1,void* e2){
    char* Item1 = (char*)e1;
    char* Item2 = (char*)e2;
    for(int i = 0;i<ElementSize;i++){
        if(Item1[i]==Item2[i]){
            continue;
        }else if(Item1[i]<Item2[i]){
            return CMP_LESS;
        }else if(Item1[i]>Item2[i]){
            return CMP_GREATER;
        }
    }
    return CMP_EQUAL;
}

CMP_BOOL Cmp_Bytewise_Reverse(size_t ElementSize,void* e1,void* e2){
    char* Item1 = (char*)e1;
    char* Item2 = (char*)e2;
    for(int i = 0;i<ElementSize;i++){
        if(Item1[i]==Item2[i]){
            continue;
        }else if(Item1[i]>Item2[i]){
            return CMP_LESS;
        }else if(Item1[i]<Item2[i]){
            return CMP_GREATER;
        }
    }
    return CMP_EQUAL;
}

void* Cmp_SubBytewise(size_t ElementSize,void* e1,void* e2){
    char* Item1 = (char*)e1;
    char* Item2 = (char*)e2;
    char* Out = malloc(ElementSize);
    for(int i = 0;i<ElementSize;i++){
        Out[i] = Item1[i] - Item2[i];
    }
    return Out;
}

unsigned long long Cmp_NumberRep(size_t ElementSize,void* Item){
    unsigned long long Out = 0ULL;
    for(int i = 0;i<ElementSize && i<sizeof(unsigned long long);i++){
        Out |= (unsigned long long)((char*)Item)[i] << i;
    }
    return Out;
}

unsigned long long Cmp_NumberRepSub(size_t ElementSize,void* e1,void* e2){
    void* Item = Cmp_SubBytewise(ElementSize,e1,e2);
    unsigned long long Out = Cmp_NumberRep(ElementSize,Item);
    free(Item);
    return Out;
}

#endif //!COMPARE_H