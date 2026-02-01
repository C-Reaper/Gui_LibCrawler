#ifndef OBJECTNOTATION_H
#define OBJECTNOTATION_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Container/Vector.h"
#include "../Container/Array.h"
#include "../Container/List.h"
#include "../Container/Tree.h"

#include "String.h"
#include "ConstParser.h"

#define STATE_NONE          0
#define STATE_NAME          1
#define STATE_VALUE         2
#define STATE_COMMENT       3
#define STATE_CHAINED       4

typedef struct Entry {
    char* Name;
    char* Value;
    CMP_BOOL Comment;
} Entry;

Entry Entry_New(char* Name,char* Value,CMP_BOOL Comment){
    Entry t;
    if(Name)    t.Name = CStr_Cpy(Name);
    else        t.Name = NULL;
    if(Value)   t.Value = CStr_Cpy(Value);
    else        t.Value = NULL;
    t.Comment = Comment;
    return t;
}
Entry Entry_Move(char* Name,char* Value,CMP_BOOL Comment){
    Entry t;
    t.Name = Name;
    t.Value = Value;
    t.Comment = Comment;
    return t;
}
void Entry_Print(Branch* n,FILE* f,int Indent){
    Entry* e = (Entry*)n->Memory;
    
    if(e){
        for(int i = 0;i<Indent;i++) fprintf(f,"\t");
        if(e->Name && !e->Comment)           fprintf(f,"\"%s\"",e->Name);
        if(e->Name && !e->Comment)           fprintf(f," = ");
        if(e->Comment){
            fprintf(f,"#");
            if(e->Value)                     fprintf(f,"%s",e->Value);
        }else if(e->Value && !e->Comment)    fprintf(f,"\"%s\"",e->Value);
    }

    if(n->Childs.size>0){
        //for(int i = 0;i<Indent;i++) fprintf(f,"\t");
        if(e->Name) fprintf(f," ");
        fprintf(f,"{\n");
        for(int i = 0;i<n->Childs.size;i++){
            Branch* c = *(Branch**)Vector_Get(&n->Childs,i);
            if(c) Entry_Print(c,f,Indent+1);
        }
        for(int i = 0;i<Indent;i++) fprintf(f,"\t");
        fprintf(f,"}\n");
    }else{
        fprintf(f,"\n");
    }
}
void Entry_Free(Entry* e){
    if(e){
        if(e->Name)     free(e->Name);
        if(e->Value)    free(e->Value);
        e->Name = NULL;
        e->Value = NULL;
    }
}
void Entry_Branch_Free(Branch* n) {
    if(n){
        Entry_Free(n->Memory);
    }       
}
void Entry_Branch_Delete(Branch* n) {
    for(int i = 0;i<n->Childs.size;i++){
        Branch* c = *(Branch**)Vector_Get(&n->Childs,i);
        Entry_Branch_Delete(c);
    }
    Entry_Branch_Free(n);
    Branch_Free(n);
}
void Entry_Branch_Print(Branch* n,int Indent){
    Entry* e = (Entry*)n->Memory;
    
    printf("|");
    for(int i = 0;i<Indent;i++) printf("--");
    
    if(n->Parent==NULL)         printf("Root:   ");
    else if(n->Childs.size==0)  printf("Leaf:   ");
    else                        printf("Branch: ");
    
    if(e){
        if(e->Name)     printf("%s",e->Name);
        if(!e->Comment) printf(" = ");
        else            printf(": ");
        if(e->Value)    printf("%s",e->Value);
    }
    printf("\n");

    for(int i = 0;i<n->Childs.size;i++){
        Branch* c = *(Branch**)Vector_Get(&n->Childs,i);
        Entry_Branch_Print(c,Indent+1);
    }
}

typedef Tree Table;

Table Table_New(){
    Table t = Tree_New();
    return t;
}
Branch* Table_EndStatement(Branch* In,char* Name,char* Value,CMP_BOOL Comment,int* State,String* Buffer){
    Branch* b = Branch_New((Entry[]){Entry_Move(Name,Value,Comment)},sizeof(Entry));
    Branch_Add(In,b);

    //if(Value) free(Value);
    //if(Name) free(Name);
    return b;
}
char* Table_Correct(String* Path){
    if(Path->size==0)   return NULL;
    else{
        char* cstr = String_CStr(Path);
        String_Clear(Path);
        return cstr;
    }
}
void Table_Parse(Table* t,char* Path){
    FILE* f = fopen(Path,"r");
    
    if(!f){
        printf("[Table]: Parse -> File \"%s\" couldn't open!\n",Path);
        return;
    }

    fseek(f,0,SEEK_END);
    long Size = ftell(f);
    fseek(f,0,SEEK_SET);
    char* Text = malloc(Size+1);
    int read = fread(Text,1,Size,f);
    while(read<=0) read = fread(Text,1,Size,f);
    Text[Size] = '\0';
    fclose(f);

    Branch* BranchIn = NULL;
    Vector Branches = Vector_New(sizeof(Branch*));
    char* Name = NULL;

    Branch* Last = NULL;

    int StateBefore = STATE_NONE;
    int State = 0;
    String Buffer = String_New();
    for(int i = 0;i<Size;i++){
        char c = Text[i];
        if(State==STATE_COMMENT){
            if(c=='\n'){
                Last = Table_EndStatement(BranchIn,CStr_Cpy("#Comment"),Table_Correct(&Buffer),1,&State,&Buffer);
                Name = NULL;
                State = STATE_NONE;
            }else{
                String_AppendChar(&Buffer,c);
                continue;
            }
        }else if(State==STATE_CHAINED && c!='\"'){
            String_AppendChar(&Buffer,c);
        }else if(c=='{'){
            if(State==STATE_NONE){
                if(Name==NULL){
                    Name = Table_Correct(&Buffer);
                    State = STATE_VALUE;
                }
            }
            if(BranchIn==NULL){
                if(Name!=NULL){
                    Branch_Set(t->Root,(Entry[]){Entry_Move(Name,Table_Correct(&Buffer),0)},sizeof(Entry));
                    Name = NULL;
                }
                Last = t->Root;
            }else{
                Last = Table_EndStatement(BranchIn,Name,Table_Correct(&Buffer),0,&State,&Buffer);
                Name = NULL;
                Vector_Push(&Branches,&BranchIn);
            }
            
            BranchIn = Last;

            State = STATE_NONE;
        }else if(c=='}'){
            if(State==STATE_NONE){
                if(Name==NULL){
                    Name = Table_Correct(&Buffer);
                    State = STATE_VALUE;
                }
            }
            if(Name!=NULL){
                Last = Table_EndStatement(BranchIn,Name,Table_Correct(&Buffer),0,&State,&Buffer);
                Name = NULL;
            }
            if(Branches.size>0){
                BranchIn = *(Branch**)Vector_Get(&Branches,Branches.size-1);
                Vector_PopTop(&Branches);
            }else{
                BranchIn = NULL;
            }
            State = STATE_NONE;
        }else if(c=='#'){
            State = STATE_COMMENT;
        }else if(c=='\"'){
            if(State!=STATE_CHAINED){
                StateBefore = State;
                State = STATE_CHAINED;
            }else{
                State = StateBefore;
                StateBefore = STATE_NONE;
            }
        }else if(c=='\n' || c==';'){
            if(State==STATE_NONE){
                if(Name==NULL){
                    Name = Table_Correct(&Buffer);
                    State = STATE_VALUE;
                }
            }
            if(BranchIn==NULL){
                if(Name!=NULL){
                    Branch_Set(t->Root,(Entry[]){Entry_Move(Name,Table_Correct(&Buffer),0)},sizeof(Entry));
                    Name = NULL;
                    Last = t->Root;
                }
            }else{
                if(Name!=NULL){
                    Last = Table_EndStatement(BranchIn,Name,Table_Correct(&Buffer),0,&State,&Buffer);
                    Name = NULL;
                }
            }
            State = STATE_NONE;
        }else if(c=='='){
            Name = Table_Correct(&Buffer);
            State = STATE_VALUE;
        }else if(Char_StringPart(c)){
            String_AppendChar(&Buffer,c);
        }else if(Char_Num(c)){
            String_AppendChar(&Buffer,c);
        }
    }

    free(Text);
    String_Free(&Buffer);
}
void Table_Write(Table* t,char* Path){
    if(!Path){
        printf("No File appended.\n");
        return;
    }
    
    FILE* f = fopen(Path,"w");
    
    if(!f){
        printf("File couldn't open.\n");
        return;
    }
    
    Entry_Print(t->Root,f,0);

    fclose(f);
}
Branch* Table_Access_Branch(Table* t,char* Path,char Seperator){
    Branch* BranchIn = t->Root;
    
    Vector Parts = CStr_ChopDown(Path,Seperator);
    for(int i = 1;i<Parts.size;i++){
        char* cstr = *(char**)Vector_Get(&Parts,i);

        for(int j = 0;j<BranchIn->Childs.size;j++){
            Branch* b = *(Branch**)Vector_Get(&BranchIn->Childs,j);
            Entry* e = (Entry*)b->Memory;
            if(CStr_Cmp(e->Name,cstr)){
                BranchIn = b;
                break;
            }
            if(j==BranchIn->Childs.size-1){
                Number n = Number_Parse((i8*)cstr);
                if(n!=NUMBER_PARSE_ERROR){
                    if(n>=0 && n<BranchIn->Childs.size){
                        BranchIn = *(Branch**)Vector_Get(&BranchIn->Childs,n);
                        break;
                    }else{
                        printf("[ALXON]: Parse error: expexted dir or file, but _%s_ in path _%s_ is a index but not in bounce.\n",cstr,Path);
                    }
                }else{
                    printf("[ALXON]: Parse error: expexted dir or file, but _%s_ in path _%s_ is rather a file, dir or index.\n",cstr,Path);
                }
                Vector_Free(&Parts);
                return NULL;
            }
        }

        free(cstr);
    }

    Branch* Field = NULL;
    if(BranchIn!=t->Root){
        Field = BranchIn;
    }

    Vector_Free(&Parts);
    return Field;
}
Entry* Table_Access_Entry(Table* t,char* Path,char Seperator){
    Branch* b = Table_Access_Branch(t,Path,Seperator);
    if(b) return (Entry*)b->Memory;
    return NULL;
}

Branch* Table_Access_Branch_F(Table* t,char Seperator,char* Path,...){
    va_list args;
    va_start(args,Path);
    String str = String_FormatA(Path,args);
    va_end(args);
    
    CStr cstr = String_CStr(&str);
    String_Free(&str);
    
    Branch* b = Table_Access_Branch(t,cstr,Seperator);
    CStr_Free(&cstr);

    return b;
}
Entry* Table_Access_Entry_F(Table* t,char Seperator,char* Path,...){
    va_list args;
    va_start(args,Path);
    String str = String_FormatA(Path,args);
    va_end(args);
    
    CStr cstr = String_CStr(&str);
    String_Free(&str);
    
    Entry* e = Table_Access_Entry(t,cstr,Seperator);
    CStr_Free(&cstr);

    return e;
}

void Table_Print(Table* t){
    printf("Tree: --------\n");
    Entry_Branch_Print(t->Root,0);
    printf("--------------\n");
}
void Table_Free(Table* t){
    Entry_Branch_Delete(t->Root);
    t->Root = NULL;
    //printf("Freed Hole Tree.\n");
}

#endif