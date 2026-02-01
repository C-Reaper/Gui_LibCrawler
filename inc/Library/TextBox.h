#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "Math.h"
#include "Rect.h"
#include "Intrinsics.h"
#include "String.h"
#include "String_G.h"
#include "TString.h"
#include "Input.h"
#include "AlxFont.h"

typedef struct TextBox {
    Input In;
    Tex t;
    Rect r;
    char* Path;
    AlxFont AlxFont;
    int ScrollX;
    int ScrollY;
    HighLight Syntax;
    unsigned int Fg;
    unsigned int Bg;
} TextBox;

void TextBox_AlxFontSetSize(TextBox* tb,int CharSizeX,int CharSizeY){
    AlxFont_Free(&tb->AlxFont);
    tb->AlxFont = AlxFont_New(Sprite_Load(tb->Path),tb->AlxFont.Columns,tb->AlxFont.Rows,tb->AlxFont.CharSizeX,tb->AlxFont.CharSizeY);
    AlxFont_Resize(&tb->AlxFont,CharSizeX,CharSizeY);
}
void TextBox_AlxFontResize(TextBox* tb,int CharSizeX,int CharSizeY){
    AlxFont_Resize(&tb->AlxFont,CharSizeX,CharSizeY);
}
void TextBox_ZoomIn(TextBox* tb){
    TextBox_AlxFontSetSize(tb,tb->AlxFont.CharSizeX+1,tb->AlxFont.CharSizeY+1);
}
void TextBox_ZoomOut(TextBox* tb){
    TextBox_AlxFontSetSize(tb,tb->AlxFont.CharSizeX-1,tb->AlxFont.CharSizeY-1);
}

TextBox TextBox_New(Input In,Rect r,char* Path,int Columns,int Rows,int CharSizeX,int CharSizeY,int AlxFontSizeX,int AlxFontSizeY,unsigned int Bg){
    TextBox tb;
    tb.In = In;
    tb.t = Vector_New(sizeof(SymbolInfo));
    tb.r = r;
    tb.Path = CStr_Cpy(Path);
    tb.AlxFont = AlxFont_Make(Sprite_Load(Path),Columns,Rows,CharSizeX,CharSizeY,AlxFontSizeX,AlxFontSizeY);
    tb.ScrollX = 0;
    tb.ScrollY = 0;
    tb.Syntax = Vector_New(sizeof(Pair));
    tb.Bg = Bg;
    tb.Fg = 0xFF666666;

    Vector_Push(&tb.In.Handlers,(InputHandler[]){
        InputHandler_New(
            (InputCombination[]){
                InputCombination_New((States){ .DOWN = 1 }, ALX_KEY_CTRL),
                InputCombination_New((States){ .DOWN = 1 }, ALX_KEY_PLUS),
                InputCombination_Null()
            },
            (void*)TextBox_ZoomIn
        )
    });
    Vector_Push(&tb.In.Handlers,(InputHandler[]){
        InputHandler_New(
            (InputCombination[]){
                InputCombination_New((States){ .DOWN = 1 }, ALX_KEY_CTRL),
                InputCombination_New((States){ .DOWN = 1 }, ALX_KEY_DASH),
                InputCombination_Null()
            }, 
            (void*)TextBox_ZoomOut
        )
    });
    
    return tb;
}
void TextBox_SetBox(TextBox* tb,Rect r){
    tb->r = r;
}
void TextBox_SetSyntax(TextBox* tb,char* Path){
    HighLight_Set(&tb->Syntax,Path);
}

void TextBox_Update(TextBox* tb,States* Strokes,Vec2 Mouse){
    Input_Update(&tb->In,Strokes);
    
    if(Input_Stroke(&tb->In,ALX_MOUSE_L).PRESSED){
        if(Mouse.x>=tb->r.p.x && Mouse.x<tb->r.p.x+tb->r.d.x && Mouse.y>=tb->r.p.y && Mouse.y<tb->r.p.y+tb->r.d.y){
            tb->In.Enabled = 1;
        }else{
            tb->In.Enabled = 0;
        }
    }
    if(Input_Stroke(&tb->In,ALX_MOUSE_L).PRESSED){
        tb->In.Curser = Input_FindChar(
            &tb->In,
            tb->In.Curser,
            Input_GetCurser(
                &tb->In,
                (Mouse.x - tb->r.p.x) / tb->AlxFont.CharSizeX - tb->ScrollX,
                (Mouse.y - tb->r.p.y) / (tb->AlxFont.CharSizeY * INPUT_GAP_FAKTOR) - tb->ScrollY
            ),
            INPUT_BLOCKCHAR
        );
    }
    if(Input_Stroke(&tb->In,ALX_MOUSE_L).DOWN){
        tb->In.CurserEnd = Input_FindChar(
            &tb->In,
            tb->In.CurserEnd,
            Input_GetCurser(
                &tb->In,
                (Mouse.x - tb->r.p.x) / tb->AlxFont.CharSizeX - tb->ScrollX,
                (Mouse.y - tb->r.p.y) / (tb->AlxFont.CharSizeY * INPUT_GAP_FAKTOR) - tb->ScrollY
            ),
            INPUT_BLOCKCHAR
        );
    }

    Input_DefaultReact(&tb->In,tb);
}
void TextBox_Render(unsigned int* Target,int Target_Width,int Target_Height,TextBox* tb){
    Tex_SyncString(&tb->t,&tb->In.Buffer);
    HighLight_Tex_String(&tb->Syntax,&tb->t,&tb->In.Buffer);

    Rect_Render(Target,Target_Width,Target_Height,tb->r,tb->Bg);
    Rect_RenderWire(Target,Target_Width,Target_Height,tb->r,tb->Fg,1.0f);

    int CurserX = Input_CurserX(&tb->In,tb->In.Curser);
    int CurserY = Input_CurserY(&tb->In,tb->In.Curser);

    float xEnd = tb->r.d.x / (float)tb->AlxFont.CharSizeX * 0.8f;
    float yEnd = tb->r.d.y / (float)tb->AlxFont.CharSizeY * 0.8f;

    if(tb->In.Enabled){
        if(tb->ScrollX + CurserX < 0 || tb->ScrollX + CurserX > xEnd){
            tb->ScrollX = F32_Clamp(tb->ScrollX,-CurserX,-CurserX + (int)xEnd);
            //tb->ScrollX = -CurserX;
            //tb->ScrollX = (int)(F32_Abs(tb->ScrollX) <= F32_Abs(tb->r.d.x / tb->AlxFont.CharSizeX - 5)?0: tb->ScrollX + tb->r.d.x / tb->AlxFont.CharSizeX - 5);
        }
        if(tb->ScrollY + CurserY < 0 || tb->ScrollY + CurserY > yEnd){
            tb->ScrollY = F32_Clamp(tb->ScrollY,-CurserY,-CurserY + (int)yEnd);
            if(tb->In.MaxLine==1) tb->ScrollY = 0;
            //tb->ScrollY = -CurserY;
            //tb->ScrollY = (int)(F32_Abs(tb->ScrollY) <= F32_Abs(tb->r.d.y / tb->AlxFont.CharSizeY - 5)?0: tb->ScrollY + tb->r.d.y / tb->AlxFont.CharSizeY - 5);
        }
    }

    int Lines = -tb->ScrollY;
    int FirstChar = String_FirstCharOfLine(&tb->In.Buffer,Lines);
    int LastChar = String_LastCharOfLine(&tb->In.Buffer,Lines);
    int MaxLine = -tb->ScrollY + (int)yEnd;

    MaxLine = I64_Max(1,MaxLine);

    for(int i = Lines;i<=MaxLine;i++){
        FirstChar -= tb->ScrollX;
        int Size = LastChar - FirstChar;
        float x = tb->ScrollX * tb->AlxFont.CharSizeX + tb->r.p.x;
        float y = tb->r.p.y + (tb->ScrollY + i) * (tb->AlxFont.CharSizeY * INPUT_GAP_FAKTOR);

        x = F32_Clamp(x,tb->r.p.x,tb->r.p.x+tb->r.d.x);
        y = F32_Clamp(y,tb->r.p.y,tb->r.p.y+tb->r.d.y);
        Size = (int)F32_Clamp(Size,0,(tb->r.d.x - (x - tb->r.p.x)) / tb->AlxFont.CharSizeX);

        TCStr_RenderSizeAlxFont(Target,Target_Width,Target_Height,&tb->AlxFont,Vector_Get(&tb->t,FirstChar),(unsigned char*)(tb->In.Buffer.Memory + FirstChar),Size,x,y);

        int TempF = String_FirstCharOfLineLast(&tb->In.Buffer,i+1,FirstChar,LastChar,i);
        int TempL = String_LastCharOfLineLast(&tb->In.Buffer,i+1,FirstChar,LastChar,i);
        FirstChar = TempF;
        LastChar = TempL;

        if(LastChar>tb->In.Buffer.size){
            break;
        }
    }

    if(tb->In.Enabled){
        Char_RenderAlxFont(Target,Target_Width,Target_Height,&tb->AlxFont,'_',tb->r.p.x+(CurserX + tb->ScrollX) * tb->AlxFont.CharSizeX,tb->r.p.y+(CurserY + tb->ScrollY) * (tb->AlxFont.CharSizeY * INPUT_GAP_FAKTOR),0xFFFF0000);

        if(tb->In.Curser!=tb->In.CurserEnd && tb->In.CurserEnd>=0){
            int Up = tb->In.Curser<tb->In.CurserEnd?tb->In.Curser:tb->In.CurserEnd;
            int Down = tb->In.Curser>tb->In.CurserEnd?tb->In.Curser:tb->In.CurserEnd;

            int Before = Up;
            for(int i = Up;;i++){
                if(String_Get(&tb->In.Buffer,i)=='\n'){
                    int X = (i+1<Down?i+1:Down+1) - Before;

                    float x = tb->r.p.x+(Input_CurserX(&tb->In,Before) + tb->ScrollX) * tb->AlxFont.CharSizeX;
                    float y = tb->r.p.y+(Input_CurserY(&tb->In,Before) + tb->ScrollY) * (tb->AlxFont.CharSizeY * INPUT_GAP_FAKTOR);
                    float w = X * tb->AlxFont.CharSizeX;
                    float h = (tb->AlxFont.CharSizeY * INPUT_GAP_FAKTOR);
                    
                    x = F32_Clamp(x,tb->r.p.x,tb->r.p.x+tb->r.d.x);
                    y = F32_Clamp(y,tb->r.p.y,tb->r.p.y+tb->r.d.y);
                    w = F32_Clamp(w,0.0f,tb->r.d.x - (x - tb->r.p.x));
                    h = F32_Clamp(h,0.0f,tb->r.d.y - (y - tb->r.p.y));
                    
                    Rect_RenderXXAlpha(Target,Target_Width,Target_Height,x,y,w,h,0x99BBBBBB);
                    Before = i+1;
                }
                if(i>=Down && String_Get(&tb->In.Buffer,i)=='\n'){
                    break;
                }
            }
        }
    }
}
void TextBox_Free(TextBox* tb){
    Input_Free(&tb->In);
    
    for(int i = 0;i<tb->Syntax.size;i++){
        Pair* p = (Pair*)Vector_Get(&tb->Syntax,i);
        Pair_Free(p);
    }

    Vector_Free(&tb->t);
    Vector_Free(&tb->Syntax);
    free(tb->Path);
    tb->Path = NULL;
    AlxFont_Free(&tb->AlxFont);
}

#endif // !TEXTBOX_H