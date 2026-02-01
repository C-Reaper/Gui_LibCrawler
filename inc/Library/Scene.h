#ifndef SCENE_H
#define SCENE_H

#include "../Container/List.h"
#include "../Container/PVector.h"

#include "Pixel.h"
#include "Rect.h"
#include "Math.h"
#include "AlxTime.h"
#include "String.h"
#include "Input.h"
#include "AlxFont.h"
#include "Geometry.h"

#include "CStr_G.h"
#include "String_G.h"
#include "TString.h"


typedef unsigned int EventId;

#define EVENT_NONE              0
#define EVENT_PRESSED           1
#define EVENT_INSIDE_RELEASED   2
#define EVENT_OUTSIDE_RELEASED  3
#define EVENT_DOWN              4
#define EVENT_INSIDE            5
#define EVENT_MOVED             6
#define EVENT_DRAGGED           7
#define EVENT_ENTERED           8
#define EVENT_EXITED            9
#define EVENT_EMTY              10
#define EVENT_FULL              11
#define EVENT_RESET             12
#define EVENT_START             13
#define EVENT_STOP              14
#define EVENT_FOCUSED           15
#define EVENT_UNFOCUSED         16
#define EVENT_ROTATE            17

#define EVENT_USE(e)            (1 << (e - 1))
#define EVENT_ISDOWN(e)         (e==EVENT_PRESSED || e==EVENT_DOWN || e==EVENT_DRAGGED)

#define EVENT_STD_LABEL         (EVENT_USE(EVENT_MOVED) | EVENT_USE(EVENT_DRAGGED) | EVENT_USE(EVENT_ENTERED) | EVENT_USE(EVENT_EXITED))
#define EVENT_STD_BUTTON        (EVENT_USE(EVENT_PRESSED) | EVENT_USE(EVENT_INSIDE_RELEASED) | EVENT_USE(EVENT_OUTSIDE_RELEASED) | EVENT_USE(EVENT_MOVED) | EVENT_USE(EVENT_DRAGGED) | EVENT_USE(EVENT_ENTERED) | EVENT_USE(EVENT_EXITED))
#define EVENT_STD_PROGRESSBAR   (EVENT_USE(EVENT_PRESSED) | EVENT_USE(EVENT_INSIDE_RELEASED) | EVENT_USE(EVENT_OUTSIDE_RELEASED) | EVENT_USE(EVENT_EMTY) | EVENT_USE(EVENT_FULL) | EVENT_USE(EVENT_RESET) | EVENT_USE(EVENT_START) | EVENT_USE(EVENT_STOP))
#define EVENT_STD_SCROLLBAR     (EVENT_USE(EVENT_PRESSED) | EVENT_USE(EVENT_DOWN) | EVENT_USE(EVENT_DRAGGED))
#define EVENT_STD_SLIDER        (EVENT_USE(EVENT_PRESSED) | EVENT_USE(EVENT_DOWN) | EVENT_USE(EVENT_DRAGGED))
#define EVENT_STD_TEXTBOX       (EVENT_USE(EVENT_PRESSED) | EVENT_USE(EVENT_DOWN) | EVENT_USE(EVENT_FOCUSED) | EVENT_USE(EVENT_UNFOCUSED))
#define EVENT_STD_SELECTION     (EVENT_USE(EVENT_PRESSED) | EVENT_USE(EVENT_DOWN))
#define EVENT_STD_ROTATEABLE    (EVENT_USE(EVENT_PRESSED) | EVENT_USE(EVENT_DOWN) | EVENT_USE(EVENT_ROTATE))
#define EVENT_STD_EDITOR        (EVENT_USE(EVENT_PRESSED) | EVENT_USE(EVENT_DOWN) | EVENT_USE(EVENT_FOCUSED) | EVENT_USE(EVENT_UNFOCUSED))


typedef unsigned int Alignment;
#define ALIGN_NO_ALIGN      0b00000000000000000000000000000000U
#define ALIGN_INVERT        0b10000000000000000000000000000000U
#define ALIGN_INVISIBLE     0b10000000000000000000000000000001U
#define ALIGN_VISIBLE       0b00000000000000000000000000000001U
#define ALIGN_VERT_CENTER   0b10000000000000000000000000000010U
#define ALIGN_HORI_CENTER   0b10000000000000000000000000000100U
#define ALIGN_VERT_TOP      0b00000000000000000000000000000010U
#define ALIGN_HORI_LEFT     0b00000000000000000000000000000100U
#define ALIGN_NOBORDER      0b10000000000000000000000000001000U
#define ALIGN_BORDER        0b00000000000000000000000000001000U

char Alignment_active(Alignment a,Alignment check){
    if(check & ALIGN_INVERT)    return (a & ~(check & ~ALIGN_INVERT)) == 0;
    else                        return (a & check) != 0;
}


typedef struct Renderable {
    unsigned int Bg;
    unsigned int Fg;
    unsigned int Bc;
    Alignment Align;
    int State;
    int UsedStates;
    int Remove;
    Rect rect;
    struct Renderable* Parent;
    void (*Render)(unsigned int*,int,int,void*,void*);
    void (*React)(void*,void*,EventId*);
    void (*Update)(void*,void*,States*,Vec2,Vec2);
    void (*Free)(void*,void*);
} Renderable;

Renderable Renderable_New(
    Renderable* Parent,
    void (*Renderable_Render)(unsigned int*,int,int,void*,void*),
    void (*Renderable_React)(void*,void*,EventId*),
    void (*Renderable_Update)(void*,void*,States*,Vec2,Vec2),
    void (*Renderable_Free)(void*,void*),
    Rect rect,
    EventId UsedStates,
    unsigned int Align,
    unsigned int Bg,
    unsigned int Fg,
    unsigned int Bc
){
    Renderable b;
    b.Bg = Bg;
    b.Fg = Fg;
    b.Bc = Bc;
    b.Align = ALIGN_VISIBLE | Align;
    b.State = EVENT_NONE;
    b.UsedStates = UsedStates;
    b.Remove = 0;
    b.rect = rect;
    b.Parent = Parent;
    b.Render = (void (*)(unsigned int*,int,int,void*,void*))Renderable_Render;
    b.React = (void (*)(void*,void*,EventId*))Renderable_React;
    b.Update = (void (*)(void*,void*,States*,Vec2,Vec2))Renderable_Update;
    b.Free = (void (*)(void*,void*))Renderable_Free;
    return b;
}
Renderable Renderable_Cpy(Renderable* r){
    Renderable b;
    b.Bg = r->Bg;
    b.Fg = r->Fg;
    b.Bc = r->Bc;
    b.Align = r->Align;
    b.State = r->State;
    b.UsedStates = r->UsedStates;
    b.Remove = 0;
    b.rect = r->rect;
    b.Parent = r->Parent;
    b.Render = r->Render;
    b.React = r->React;
    b.Update = r->Update;
    b.Free = r->Free;
    return b;
}
void Renderable_Set(Renderable* b,Alignment a){
    if(a & ALIGN_INVERT)    b->Align = b->Align & ~(a & ~ALIGN_INVERT);
    else                    b->Align = b->Align | a;
}
void Renderable_Free(void* parent,Renderable* b){
    if(b->Free) b->Free(parent,b);

    b->Bg = BLACK;
    b->Fg = BLACK;
    b->Bc = BLACK;
    b->Align = ALIGN_NO_ALIGN;
    b->State = EVENT_NONE;
    b->UsedStates = EVENT_NONE;
    b->Remove = 0;
    b->rect = (Rect){ { 0.0f,0.0f },{ 0.0f,0.0f } };
    b->Parent = NULL;
    b->Render = NULL;
    b->React = NULL;
    b->Update = NULL;
    b->Free = NULL;
}
void Renderable_Print_N(Renderable* b){
    printf("%p %p %p %p %p | ",b->Parent,b->Render,b->React,b->Update,b->Free);
    printf("%d %d %d %d | ",(int)b->rect.p.x,(int)b->rect.p.y,(int)b->rect.d.x,(int)b->rect.d.y);
    printf("%d %x %x %x %x",b->UsedStates,b->Align,b->Bg,b->Fg,b->Bc);
}
void Renderable_Print(Renderable* b){
    printf("--- Renderable ---\n");
    printf("%p %p %p %p %p\n",b->Parent,b->Render,b->React,b->Update,b->Free);
    printf("%d %d %d %d\n",(int)b->rect.p.x,(int)b->rect.p.y,(int)b->rect.d.x,(int)b->rect.d.y);
    printf("%d %x %x %x %x\n",b->UsedStates,b->Align,b->Bg,b->Fg,b->Bc);
    printf("------------------\n");
}




typedef struct WindowComponentEvent {
    EventId eid;
    int ButtonId;
    Vec2 pos;
    Vec2 posBefore;
} WindowComponentEvent;
typedef struct WindowComponent {
    Renderable renderable;
    String text;
    Vec2 PixelSize;
} WindowComponent;

void WindowComponent_Free(void* parent,WindowComponent* b){
    String_Free(&b->text);
}
WindowComponent WindowComponent_New(Renderable* Parent,char* text,Vec2 PixelSize,Rect rect,unsigned int Bg){
    WindowComponent b;
    b.text = String_Make(text);
    b.PixelSize = PixelSize;
    b.renderable = Renderable_New(Parent,NULL,NULL,NULL,(void(*)(void*,void*))WindowComponent_Free,rect,EVENT_STD_LABEL,1,Bg,WHITE,BLACK);
    return b;
}
WindowComponent WindowComponent_NewStd(Renderable* Parent,char* text,Vec2 PixelSize,Rect rect,unsigned int Bg){
    WindowComponent b = WindowComponent_New(
        Parent,
        text,
        PixelSize,
        rect,
        Bg
    );
    return b;
}



typedef struct LabelEvent {
    EventId eid;
    int ButtonId;
    Vec2 pos;
    Vec2 posBefore;
} LabelEvent;
typedef struct Label {
    Renderable renderable;
    String text;
    AlxFont AlxFont;
} Label;

void Label_Render(unsigned int *Target, int Target_Width, int Target_Height,void* parent,Label* b){
    int StrSizeX = b->AlxFont.CharSizeX * b->text.size;
    int StrSizeY = b->AlxFont.CharSizeY;
    Vec2 Pos = b->renderable.rect.p;
    if(Alignment_active(b->renderable.Align,ALIGN_HORI_CENTER)) Pos.x += b->renderable.rect.d.x / 2 - StrSizeX / 2;
    if(Alignment_active(b->renderable.Align,ALIGN_VERT_CENTER)) Pos.y += b->renderable.rect.d.y / 2 - StrSizeY / 2;
    unsigned int BgColor = b->renderable.Bg;
    
    Rect_Render(Target,Target_Width,Target_Height,b->renderable.rect,BgColor);

    if(Alignment_active(b->renderable.Align,ALIGN_BORDER))
        Rect_RenderWire(Target,Target_Width,Target_Height,b->renderable.rect,b->renderable.Bc,1.0f);
    
    CStr_RenderSizeAlxFont(Target,Target_Width,Target_Height,&b->AlxFont,b->text.Memory,b->text.size,Pos.x,Pos.y,b->renderable.Fg);
}
void Label_Event(void* parent,Label* b,LabelEvent be){
    if(be.eid!=EVENT_NONE){
        b->renderable.State = be.eid;
        if(b->renderable.UsedStates & EVENT_USE(be.eid)){
            if(b->renderable.React){
                b->renderable.React(parent,b,(EventId*)&be);
            }
        }
    }
}
void Label_Update(void* parent,Label* b,States* states,Vec2 Mouse,Vec2 MouseB){
    b->renderable.State = EVENT_NONE;

    if(!Overlap_Rect_Point(b->renderable.rect,Mouse) &&  Overlap_Rect_Point(b->renderable.rect,MouseB))
        Label_Event(parent,b,(LabelEvent){ .eid = EVENT_ENTERED });
    if( Overlap_Rect_Point(b->renderable.rect,Mouse) && !Overlap_Rect_Point(b->renderable.rect,MouseB))
        Label_Event(parent,b,(LabelEvent){ .eid = EVENT_EXITED });

    if(Overlap_Rect_Point(b->renderable.rect,Mouse)){
        if(Mouse.x==MouseB.x && Mouse.y==MouseB.y){
            if(states[ALX_MOUSE_L].DOWN)    Label_Event(parent,b,(LabelEvent){ .eid = EVENT_DOWN,      .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
            else                            Label_Event(parent,b,(LabelEvent){ .eid = EVENT_INSIDE,    .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }else{
            if(states[ALX_MOUSE_L].DOWN)    Label_Event(parent,b,(LabelEvent){ .eid = EVENT_DRAGGED,   .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
            else                            Label_Event(parent,b,(LabelEvent){ .eid = EVENT_MOVED,     .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }

        if(states[ALX_MOUSE_L].PRESSED)     Label_Event(parent,b,(LabelEvent){ .eid = EVENT_PRESSED,   .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        if(states[ALX_MOUSE_M].PRESSED)     Label_Event(parent,b,(LabelEvent){ .eid = EVENT_PRESSED,   .ButtonId = ALX_MOUSE_M,.pos = Mouse,.posBefore = MouseB });
        if(states[ALX_MOUSE_R].PRESSED)     Label_Event(parent,b,(LabelEvent){ .eid = EVENT_PRESSED,   .ButtonId = ALX_MOUSE_R,.pos = Mouse,.posBefore = MouseB });

        if(states[ALX_MOUSE_L].RELEASED)    Label_Event(parent,b,(LabelEvent){ .eid = EVENT_INSIDE_RELEASED,    .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        if(states[ALX_MOUSE_M].RELEASED)    Label_Event(parent,b,(LabelEvent){ .eid = EVENT_INSIDE_RELEASED,    .ButtonId = ALX_MOUSE_M,.pos = Mouse,.posBefore = MouseB });
        if(states[ALX_MOUSE_R].RELEASED)    Label_Event(parent,b,(LabelEvent){ .eid = EVENT_INSIDE_RELEASED,    .ButtonId = ALX_MOUSE_R,.pos = Mouse,.posBefore = MouseB });
    }else{
        if(states[ALX_MOUSE_L].RELEASED)    Label_Event(parent,b,(LabelEvent){ .eid = EVENT_OUTSIDE_RELEASED,.ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        if(states[ALX_MOUSE_M].RELEASED)    Label_Event(parent,b,(LabelEvent){ .eid = EVENT_OUTSIDE_RELEASED,.ButtonId = ALX_MOUSE_M,.pos = Mouse,.posBefore = MouseB });
        if(states[ALX_MOUSE_R].RELEASED)    Label_Event(parent,b,(LabelEvent){ .eid = EVENT_OUTSIDE_RELEASED,.ButtonId = ALX_MOUSE_R,.pos = Mouse,.posBefore = MouseB });
    }
}
void Label_Free(void* parent,Label* b){
    AlxFont_Free(&b->AlxFont);
    String_Free(&b->text);
}
Label Label_New(Renderable* Parent,char* text,void (*Label_React)(void*,Label*,LabelEvent*),AlxFont AlxFont,Vec2 AlxFontSize,Rect rect,unsigned int Align,unsigned int Bg,unsigned int Fg){
    Label b;
    b.text = String_Make(text);
    b.AlxFont = AlxFont;
    AlxFont_Resize(&b.AlxFont,(int)AlxFontSize.x,(int)AlxFontSize.y);
    b.renderable = Renderable_New(Parent,(void(*)(unsigned int*,int,int,void*,void*))Label_Render,(void(*)(void*,void*,EventId*))Label_React,(void(*)(void*,void*,States*,Vec2,Vec2))Label_Update,(void(*)(void*,void*))Label_Free,rect,EVENT_STD_LABEL,Align,Bg,Fg,BLACK);
    return b;
}
Label Label_NewStd(Renderable* Parent,char* text,void (*Label_React)(void*,Label*,LabelEvent*),Vec2 AlxFontSize,Rect rect,unsigned int Bg,unsigned int Fg){
    Label b = Label_New(
        Parent,
        text,
        Label_React,
        AlxFont_New(ALXFONT_BLOCKY),
        AlxFontSize,
        rect,
        ALIGN_BORDER,
        Bg,
        Fg
    );
    return b;
}


typedef LabelEvent ButtonEvent;
typedef Label Button;

void Button_Render(unsigned int *Target, int Target_Width, int Target_Height,void* parent,Button* b){
    int StrSizeX = b->AlxFont.CharSizeX * b->text.size;
    int StrSizeY = b->AlxFont.CharSizeY;
    
    Vec2 Pos = b->renderable.rect.p;
    Vec2 PosText = b->renderable.rect.p;
    if(Alignment_active(b->renderable.Align,ALIGN_HORI_CENTER)) PosText.x += b->renderable.rect.d.x / 2 - StrSizeX / 2;
    if(Alignment_active(b->renderable.Align,ALIGN_VERT_CENTER)) PosText.y += b->renderable.rect.d.y / 2 - StrSizeY / 2;
    
    unsigned int BgColor = b->renderable.Bg;
    if(b->renderable.State==EVENT_DOWN || b->renderable.State==EVENT_DRAGGED){
        BgColor = Pixel_Mul(BgColor,Pixel_toRGBA(0.8f,0.8f,0.8f,1.0f));
        Pos.y += b->renderable.rect.d.y * 0.05f;
        PosText.y += b->renderable.rect.d.y * 0.05f;
    }
    
    Rect_RenderXX(Target,Target_Width,Target_Height,Pos.x,Pos.y,b->renderable.rect.d.x,b->renderable.rect.d.y,BgColor);
    if(Alignment_active(b->renderable.Align,ALIGN_BORDER))
        Rect_RenderXXWire(Target,Target_Width,Target_Height,Pos.x,Pos.y,b->renderable.rect.d.x,b->renderable.rect.d.y,b->renderable.Bc,1.0f);
    CStr_RenderSizeAlxFont(Target,Target_Width,Target_Height,&b->AlxFont,b->text.Memory,b->text.size,PosText.x,PosText.y,b->renderable.Fg);
}
void Button_Event(void* parent,Button* b,ButtonEvent be){
    Label_Event(parent,(Label*)b,*(LabelEvent*)&be);
}
void Button_Update(void* parent,Button* b,States* states,Vec2 Mouse,Vec2 MouseB){
    b->renderable.State = EVENT_NONE;
    Label_Update(parent,(Label*)b,states,Mouse,MouseB);
}
void Button_Free(void* parent,Button* b){
    AlxFont_Free(&b->AlxFont);
    String_Free(&b->text);
}
Button Button_New(Renderable* Parent,char* text,void (*Button_React)(void* parent,Button* b,ButtonEvent* be),AlxFont AlxFont,Vec2 AlxFontSize,Rect rect,int Align,unsigned int Bg,unsigned int Fg){
    Button b;
    b.text = String_Make(text);
    b.AlxFont = AlxFont;
    AlxFont_Resize(&b.AlxFont,(int)AlxFontSize.x,(int)AlxFontSize.y);
    b.renderable = Renderable_New(Parent,(void(*)(unsigned int*,int,int,void*,void*))Button_Render,(void(*)(void*,void*,EventId*))Button_React,(void(*)(void*,void*,States*,Vec2,Vec2))Button_Update,(void(*)(void*,void*))Button_Free,rect,EVENT_STD_BUTTON,Align,Bg,Fg,BLACK);
    return b;
}
Button Button_NewStd(Renderable* Parent,char* text,void (*Button_React)(void* parent,Button* b,ButtonEvent* be),Vec2 AlxFontSize,Rect rect,unsigned int Bg,unsigned int Fg){
    return Button_New(
        Parent,
        text,
        Button_React,
        AlxFont_New(ALXFONT_BLOCKY),
        AlxFontSize,
        rect,
        ALIGN_BORDER,
        Bg,
        Fg
    );
}
Button Button_Cpy(Button* b){
    Button ret;
    ret.AlxFont = AlxFont_Make(
        Sprite_Cpy(&b->AlxFont.Atlas),
        b->AlxFont.Columns,
        b->AlxFont.Rows,
        b->AlxFont.CharSizeX,
        b->AlxFont.CharSizeY,
        b->AlxFont.CharSizeX,
        b->AlxFont.CharSizeY
    );
    ret.renderable = Renderable_Cpy(&b->renderable);
    ret.text = String_Cpy(&b->text);
    return ret;
}
Button Button_Null(){
    Button h;
    memset(&h,0,sizeof(Button));
    return h;
}
CStr Button_CStr(Button* h){
    String builder = String_Format("{  }");
    CStr resstr = String_CStr(&builder);
    String_Free(&builder);
    return resstr;
}


#define PROGRESSBAR_STOP        0
#define PROGRESSBAR_RUNNING     1

typedef LabelEvent ProgressBarEvent;
typedef struct ProgressBar {
    Label label;
    Timepoint LastTime;
    unsigned int BarColor;
    float Speed;
    float Progress;
    char status;
} ProgressBar;

void ProgressBar_Render(unsigned int *Target, int Target_Width, int Target_Height,void* parent,ProgressBar* b){
    int StrSizeX = b->label.AlxFont.CharSizeX * b->label.text.size;
    int StrSizeY = b->label.AlxFont.CharSizeY;
    Vec2 Pos = b->label.renderable.rect.p;
    Vec2 PosText = b->label.renderable.rect.p;
    if(Alignment_active(b->label.renderable.Align,ALIGN_HORI_CENTER)) PosText.x += b->label.renderable.rect.d.x / 2 - StrSizeX / 2;
    if(Alignment_active(b->label.renderable.Align,ALIGN_VERT_CENTER)) PosText.y += b->label.renderable.rect.d.y / 2 - StrSizeY / 2;
    unsigned int BgColor = b->label.renderable.Bg;
    
    Rect_RenderXX(Target,Target_Width,Target_Height,Pos.x,Pos.y,b->label.renderable.rect.d.x,b->label.renderable.rect.d.y,BgColor);
    CStr_RenderSizeAlxFont(Target,Target_Width,Target_Height,&b->label.AlxFont,b->label.text.Memory,b->label.text.size,PosText.x,PosText.y,b->label.renderable.Fg);

    Rect_RenderXXAlpha(Target,Target_Width,Target_Height,Pos.x,Pos.y,b->label.renderable.rect.d.x * b->Progress,b->label.renderable.rect.d.y,b->BarColor);
    if(Alignment_active(b->label.renderable.Align,ALIGN_BORDER))
        Rect_RenderXXWire(Target,Target_Width,Target_Height,Pos.x,Pos.y,b->label.renderable.rect.d.x,b->label.renderable.rect.d.y,b->label.renderable.Bc,1.0f);
}
void ProgressBar_Event(void* parent,ProgressBar* b,ProgressBarEvent be){
    Label_Event(parent,(Label*)b,*(LabelEvent*)&be);
}
void ProgressBar_Update(void* parent,ProgressBar* b,States* states,Vec2 Mouse,Vec2 MouseB){
    Label_Update(parent,(Label*)b,states,Mouse,MouseB);
    
    if(b->status==PROGRESSBAR_RUNNING){
        if(b->Progress==0.0f) ProgressBar_Event(parent,b,(ProgressBarEvent){ .eid = EVENT_EMTY });
        if(b->Progress==1.0f){
            b->status = PROGRESSBAR_STOP;
            ProgressBar_Event(parent,b,(ProgressBarEvent){ .eid = EVENT_FULL });
        }

        double AlxTime = (double)(Time_Nano()-b->LastTime) / (double)TIME_NANOTOSEC;
        b->LastTime = Time_Nano();
        b->Progress = F32_Min(b->Progress + b->Speed * (float)AlxTime,1.0f);
    }
}
void ProgressBar_Free(void* parent,ProgressBar* b){
    AlxFont_Free(&b->label.AlxFont);
    String_Free(&b->label.text);
}
ProgressBar ProgressBar_New(Renderable* Parent,char* text,void (*ProgressBar_React)(void* parent,ProgressBar* b,ProgressBarEvent* be),AlxFont AlxFont,Vec2 AlxFontSize,Rect rect,unsigned int Align,unsigned int Bg,unsigned int Fg,unsigned int BarColor,Timepoint LastTime,float Speed){
    ProgressBar b;
    b.label.text = String_Make(text);
    b.label.AlxFont = AlxFont;
    AlxFont_Resize(&b.label.AlxFont,(int)AlxFontSize.x,(int)AlxFontSize.y);
    b.label.renderable = Renderable_New(Parent,(void(*)(unsigned int*,int,int,void*,void*))ProgressBar_Render,(void(*)(void*,void*,EventId*))ProgressBar_React,(void(*)(void*,void*,States*,Vec2,Vec2))ProgressBar_Update,(void(*)(void*,void*))ProgressBar_Free,rect,EVENT_STD_PROGRESSBAR,Align,Bg,Fg,BLACK);

    b.LastTime = LastTime;
    b.BarColor = BarColor;
    b.Speed = Speed;
    b.Progress = 0.0f;
    b.status = PROGRESSBAR_STOP;
    return b;
}
ProgressBar ProgressBar_NewStd(Renderable* Parent,char* text,void (*ProgressBar_React)(void* parent,ProgressBar* b,ProgressBarEvent* be),Vec2 AlxFontSize,Rect rect,unsigned int Bg,unsigned int Fg,unsigned int BarColor,float Speed){
    ProgressBar b = ProgressBar_New(
        Parent,text,
        ProgressBar_React,
        AlxFont_New(ALXFONT_BLOCKY),
        AlxFontSize,
        rect,
        ALIGN_BORDER,
        Bg,
        Fg,
        BarColor,
        Time_Nano(),
        0.1f
    );
    return b;
}
void ProgressBar_Reset(void* parent,ProgressBar* b){
    b->status = PROGRESSBAR_RUNNING;
    b->Progress = 0.0f;
    b->LastTime = Time_Nano();

    ProgressBar_Event(parent,b,(ProgressBarEvent){ .eid = EVENT_RESET });
}
void ProgressBar_Start(void* parent,ProgressBar* b){
    b->status = PROGRESSBAR_RUNNING;
    b->Progress = 0.0f;
    b->LastTime = Time_Nano();

    ProgressBar_Event(parent,b,(ProgressBarEvent){ .eid = EVENT_START });
}
void ProgressBar_Stop(void* parent,ProgressBar* b){
    b->status = PROGRESSBAR_STOP;
    ProgressBar_Event(parent,b,(ProgressBarEvent){ .eid = EVENT_STOP });
}


typedef LabelEvent ScrollbarEvent;
typedef struct Scrollbar {
    Renderable renderable;
    float scrollpart;
    float scrolled;
} Scrollbar;

void Scrollbar_Render(unsigned int *Target, int Target_Width, int Target_Height,void* parent,Scrollbar* b){
    unsigned int FgColor = b->renderable.Fg;
    if(b->renderable.State==EVENT_DOWN || b->renderable.State==EVENT_DRAGGED){
        FgColor = Pixel_Mul(FgColor,Pixel_toRGBA(0.8f,0.8f,0.8f,1.0f));
    }
    
    Rect_RenderXX(Target,Target_Width,Target_Height,b->renderable.rect.p.x,b->renderable.rect.p.y,b->renderable.rect.d.x,b->renderable.rect.d.y,b->renderable.Bg);
    if(Alignment_active(b->renderable.Align,ALIGN_BORDER))
        Rect_RenderXXWire(Target,Target_Width,Target_Height,b->renderable.rect.p.x,b->renderable.rect.p.y,b->renderable.rect.d.x,b->renderable.rect.d.y,b->renderable.Bc,1.0f);

    float bw = b->renderable.rect.d.x;
    float bh = b->renderable.rect.d.y * b->scrollpart;
    float bx = b->renderable.rect.p.x;
    float by = b->renderable.rect.p.y + (b->renderable.rect.d.y - bh) * b->scrolled;

    Rect_RenderXX(Target,Target_Width,Target_Height,bx,by,bw,bh,FgColor);
}
void Scrollbar_Event(void* parent,Scrollbar* b,ScrollbarEvent be){
    if(be.eid!=EVENT_NONE){
        b->renderable.State = be.eid;
        if(b->renderable.UsedStates & EVENT_USE(be.eid)){
            if(b->renderable.React){
                b->renderable.React(parent,b,(EventId*)&be);
            }
        }
    }
}
void Scrollbar_Update(void* parent,Scrollbar* b,States* states,Vec2 Mouse,Vec2 MouseB){
    b->renderable.State = EVENT_NONE;

    if(Overlap_Rect_Point(b->renderable.rect,Mouse)){
        if(states[ALX_MOUSE_L].DOWN){
            b->scrolled = (Mouse.y - b->renderable.rect.p.y) / b->renderable.rect.d.y;
            Scrollbar_Event(parent,b,(LabelEvent){ .eid = EVENT_DOWN,      .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }
        
        if(Mouse.x!=MouseB.x || Mouse.y!=MouseB.y){
            if(states[ALX_MOUSE_L].DOWN)    Scrollbar_Event(parent,b,(LabelEvent){ .eid = EVENT_DRAGGED,   .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
            else                            Scrollbar_Event(parent,b,(LabelEvent){ .eid = EVENT_MOVED,     .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }

        if(states[ALX_MOUSE_L].PRESSED)     Scrollbar_Event(parent,b,(LabelEvent){ .eid = EVENT_PRESSED,           .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        if(states[ALX_MOUSE_L].RELEASED)    Scrollbar_Event(parent,b,(LabelEvent){ .eid = EVENT_INSIDE_RELEASED,   .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
    }else{
        if(states[ALX_MOUSE_L].RELEASED)
            Scrollbar_Event(parent,b,(LabelEvent){ .eid = EVENT_OUTSIDE_RELEASED,.ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
    }
}
void Scrollbar_Free(void* parent,Scrollbar* b){

}
Scrollbar Scrollbar_New(Renderable* Parent,void (*Scrollbar_React)(void*,Scrollbar*,ScrollbarEvent*),Rect rect,unsigned int Bg,unsigned int Fg){
    Scrollbar b;
    b.scrollpart = 0.1f;
    b.scrolled = 0.0f;
    b.renderable = Renderable_New(Parent,(void(*)(unsigned int*,int,int,void*,void*))Scrollbar_Render,(void(*)(void*,void*,EventId*))Scrollbar_React,(void(*)(void*,void*,States*,Vec2,Vec2))Scrollbar_Update,(void(*)(void*,void*))Scrollbar_Free,rect,EVENT_STD_SCROLLBAR,ALIGN_HORI_LEFT | ALIGN_VERT_TOP | ALIGN_BORDER,Bg,Fg,BLACK);
    return b;
}


typedef LabelEvent SliderEvent;
typedef struct Slider {
    Renderable renderable;
    float scrollpart;
    float scrolled;
} Slider;

void Slider_Render(unsigned int *Target, int Target_Width, int Target_Height,void* parent,Slider* b){
    unsigned int FgColor = b->renderable.Fg;
    if(b->renderable.State==EVENT_DOWN || b->renderable.State==EVENT_DRAGGED){
        FgColor = Pixel_Mul(FgColor,Pixel_toRGBA(0.8f,0.8f,0.8f,1.0f));
    }
    
    Rect_RenderXX(Target,Target_Width,Target_Height,b->renderable.rect.p.x,b->renderable.rect.p.y,b->renderable.rect.d.x,b->renderable.rect.d.y,b->renderable.Bg);
    if(Alignment_active(b->renderable.Align,ALIGN_BORDER))
        Rect_RenderXXWire(Target,Target_Width,Target_Height,b->renderable.rect.p.x,b->renderable.rect.p.y,b->renderable.rect.d.x,b->renderable.rect.d.y,b->renderable.Bc,1.0f);

    float bw = b->renderable.rect.d.x * b->scrollpart;
    float bh = b->renderable.rect.d.y;
    float bx = b->renderable.rect.p.x + (b->renderable.rect.d.x - bw) * b->scrolled;
    float by = b->renderable.rect.p.y;

    Rect_RenderXX(Target,Target_Width,Target_Height,bx,by,bw,bh,FgColor);
}
void Slider_Event(void* parent,Slider* b,SliderEvent be){
    if(be.eid!=EVENT_NONE){
        b->renderable.State = be.eid;
        if(b->renderable.UsedStates & EVENT_USE(be.eid)){
            if(b->renderable.React){
                b->renderable.React(parent,b,(EventId*)&be);
            }
        }
    }
}
void Slider_Update(void* parent,Slider* b,States* states,Vec2 Mouse,Vec2 MouseB){
    b->renderable.State = EVENT_NONE;

    if(Overlap_Rect_Point(b->renderable.rect,Mouse)){
        if(states[ALX_MOUSE_L].DOWN){
            b->scrolled = (Mouse.x - b->renderable.rect.p.x) / b->renderable.rect.d.x;

            Slider_Event(parent,b,(LabelEvent){ .eid = EVENT_DOWN,      .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }
        
        if(Mouse.x!=MouseB.x || Mouse.y!=MouseB.y){
            if(states[ALX_MOUSE_L].DOWN)    Slider_Event(parent,b,(LabelEvent){ .eid = EVENT_DRAGGED,   .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
            else                            Slider_Event(parent,b,(LabelEvent){ .eid = EVENT_MOVED,     .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }

        if(states[ALX_MOUSE_L].PRESSED)     Slider_Event(parent,b,(LabelEvent){ .eid = EVENT_PRESSED,           .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        if(states[ALX_MOUSE_L].RELEASED)    Slider_Event(parent,b,(LabelEvent){ .eid = EVENT_INSIDE_RELEASED,   .ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
    }else{
        if(states[ALX_MOUSE_L].RELEASED)
            Slider_Event(parent,b,(LabelEvent){ .eid = EVENT_OUTSIDE_RELEASED,.ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
    }
}
void Slider_Free(void* parent,Slider* b){

}
Slider Slider_New(Renderable* Parent,void (*Slider_React)(void*,Slider*,SliderEvent*),Rect rect,unsigned int Bg,unsigned int Fg){
    Slider b;
    b.renderable = Renderable_New(Parent,(void(*)(unsigned int*,int,int,void*,void*))Slider_Render,(void(*)(void*,void*,EventId*))Slider_React,(void(*)(void*,void*,States*,Vec2,Vec2))Slider_Update,(void(*)(void*,void*))Slider_Free,rect,EVENT_STD_SLIDER,ALIGN_HORI_LEFT | ALIGN_VERT_TOP | ALIGN_BORDER,Bg,Fg,BLACK);
    b.scrollpart = 0.1f;
    b.scrolled = 0.0f;
    return b;
}


typedef LabelEvent TextboxEvent;
typedef struct Textbox {
    Renderable renderable;
    AlxFont font;
    int ScrollX;
    int ScrollY;
    Input In;
} Textbox;

void Textbox_Render(unsigned int *Target, int Target_Width, int Target_Height,void* parent,Textbox* tb){
    Rect_Render(Target,Target_Width,Target_Height,tb->renderable.rect,tb->renderable.Bg);
    if(Alignment_active(tb->renderable.Align,ALIGN_BORDER))
        Rect_RenderWire(Target,Target_Width,Target_Height,tb->renderable.rect,tb->renderable.Bc,1.0f);

    int CurserX = Input_CurserX(&tb->In,tb->In.Curser);
    int CurserY = Input_CurserY(&tb->In,tb->In.Curser);

    const float text_width = (tb->renderable.rect.d.x);
    const float text_height = tb->renderable.rect.d.y;
    const float text_x = (tb->renderable.rect.p.x);
    const float text_y = tb->renderable.rect.p.y;

    float xEnd = text_width / (float)tb->font.CharSizeX * 0.8f;
    float yEnd = text_height / (float)tb->font.CharSizeY * 0.8f;

    if(tb->In.Enabled){
        if(tb->ScrollX + CurserX < 0 || tb->ScrollX + CurserX > xEnd){
            tb->ScrollX = F32_Clamp(tb->ScrollX,-CurserX,-CurserX + (int)xEnd);
            //tb->ScrollX = -CurserX;
            //tb->ScrollX = (int)(F32_Abs(tb->ScrollX) <= F32_Abs(text_width / tb->font.CharSizeX - 5)?0: tb->ScrollX + text_width / tb->AlxFont.CharSizeX - 5);
        }
        if(tb->ScrollY + CurserY < 0 || tb->ScrollY + CurserY > yEnd){
            tb->ScrollY = F32_Clamp(tb->ScrollY,-CurserY,-CurserY + (int)yEnd);
            if(tb->In.MaxLine==1) tb->ScrollY = 0;
            //tb->ScrollY = -CurserY;
            //tb->ScrollY = (int)(F32_Abs(tb->ScrollY) <= F32_Abs(text_height / tb->font.CharSizeY - 5)?0: tb->ScrollY + text_height / tb->AlxFont.CharSizeY - 5);
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
        float x = tb->ScrollX * tb->font.CharSizeX + text_x;
        float y = text_y + (tb->ScrollY + i) * (tb->font.CharSizeY * INPUT_GAP_FAKTOR);

        x = F32_Clamp(x,text_x,text_x+text_width);
        y = F32_Clamp(y,text_y,text_y+text_height);
        Size = (int)F32_Clamp(Size,0,(text_width - (x - text_x)) / tb->font.CharSizeX);

        CStr_RenderSizeAlxFont(Target,Target_Width,Target_Height,&tb->font,(unsigned char*)(tb->In.Buffer.Memory + FirstChar),Size,x,y,tb->renderable.Fg);

        int TempF = String_FirstCharOfLineLast(&tb->In.Buffer,i+1,FirstChar,LastChar,i);
        int TempL = String_LastCharOfLineLast(&tb->In.Buffer,i+1,FirstChar,LastChar,i);
        FirstChar = TempF;
        LastChar = TempL;

        if(LastChar>tb->In.Buffer.size){
            break;
        }
    }

    if(tb->In.Enabled){
        Char_RenderAlxFont(Target,Target_Width,Target_Height,&tb->font,'_',text_x+(CurserX + tb->ScrollX) * tb->font.CharSizeX,text_y+(CurserY + tb->ScrollY) * (tb->font.CharSizeY * INPUT_GAP_FAKTOR),BLACK);

        if(tb->In.Curser!=tb->In.CurserEnd && tb->In.CurserEnd>=0){
            int Up = tb->In.Curser<tb->In.CurserEnd?tb->In.Curser:tb->In.CurserEnd;
            int Down = tb->In.Curser>tb->In.CurserEnd?tb->In.Curser:tb->In.CurserEnd;

            int Before = Up;
            for(int i = Up;;i++){
                if(String_Get(&tb->In.Buffer,i)=='\n'){
                    int X = (i+1<Down?i+1:Down+1) - Before;

                    float x = text_x+(Input_CurserX(&tb->In,Before) + tb->ScrollX) * tb->font.CharSizeX;
                    float y = text_y+(Input_CurserY(&tb->In,Before) + tb->ScrollY) * (tb->font.CharSizeY * INPUT_GAP_FAKTOR);
                    float w = X * tb->font.CharSizeX;
                    float h = (tb->font.CharSizeY * INPUT_GAP_FAKTOR);
                    
                    x = F32_Clamp(x,text_x,text_x+text_width);
                    y = F32_Clamp(y,text_y,text_y+text_height);
                    w = F32_Clamp(w,0.0f,text_width - (x - text_x));
                    h = F32_Clamp(h,0.0f,text_height - (y - text_y));
                    
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
void Textbox_Event(void* parent,Textbox* b,TextboxEvent be){
    if(be.eid!=EVENT_NONE){
        b->renderable.State = be.eid;
        if(b->renderable.UsedStates & EVENT_USE(be.eid)){
            if(b->renderable.React){
                b->renderable.React(parent,b,(EventId*)&be);
            }
        }
    }
}
void Textbox_Update(void* parent,Textbox* tb,States* states,Vec2 Mouse,Vec2 MouseB){
    tb->renderable.State = EVENT_NONE;
    Input_Update(&tb->In,states);
    
    if(Input_Stroke(&tb->In,ALX_MOUSE_L).PRESSED){
        if(Overlap_Rect_Point(tb->renderable.rect,Mouse)){
            tb->In.Enabled = 1;
            Textbox_Event(parent,tb,(LabelEvent){ .eid = EVENT_FOCUSED,.ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }else{
            tb->In.Enabled = 0;
            Textbox_Event(parent,tb,(LabelEvent){ .eid = EVENT_UNFOCUSED,.ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }
    }
    if(Input_Stroke(&tb->In,ALX_MOUSE_L).PRESSED){
        tb->In.Curser = Input_GetCurser(&tb->In,(Mouse.x - tb->renderable.rect.p.x) / tb->font.CharSizeX - tb->ScrollX,(Mouse.y - tb->renderable.rect.p.y) / (tb->font.CharSizeY * INPUT_GAP_FAKTOR) - tb->ScrollY);
    }
    if(Input_Stroke(&tb->In,ALX_MOUSE_L).DOWN){
        tb->In.CurserEnd = Input_GetCurser(&tb->In,(Mouse.x - tb->renderable.rect.p.x) / tb->font.CharSizeX - tb->ScrollX,(Mouse.y - tb->renderable.rect.p.y) / (tb->font.CharSizeY * INPUT_GAP_FAKTOR) - tb->ScrollY);
    }
    
    Input_DefaultReact(&tb->In,tb);
}
void Textbox_Free(void* parent,Textbox* b){
    AlxFont_Free(&b->font);
    Input_Free(&b->In);
}
Textbox Textbox_New(Renderable* Parent,char *text,void (*Textbox_React)(void*,Textbox*,TextboxEvent*),AlxFont font,Vec2 AlxFontSize,Rect rect,unsigned int Lines,unsigned int Bg,unsigned int Fg){
    Textbox b;
    b.font = font;
    AlxFont_Resize(&b.font,(int)AlxFontSize.x,(int)AlxFontSize.y);
    b.ScrollX = 0;
    b.ScrollY = 0;
    
    b.renderable = Renderable_New(Parent,(void(*)(unsigned int*,int,int,void*,void*))Textbox_Render,(void(*)(void*,void*,EventId*))Textbox_React,(void(*)(void*,void*,States*,Vec2,Vec2))Textbox_Update,(void(*)(void*,void*))Textbox_Free,rect,EVENT_STD_TEXTBOX,ALIGN_HORI_LEFT | ALIGN_VERT_TOP | ALIGN_BORDER,Bg,Fg,BLACK);

    b.In = Input_New(10,Lines);
    Input_SetText(&b.In,text);
    return b;
}
Textbox Textbox_NewStd(Renderable* Parent,char *text,void (*Textbox_React)(void*,Textbox*,TextboxEvent*),Vec2 AlxFontSize,Rect rect,unsigned int Lines,unsigned int Bg,unsigned int Fg){
    Textbox b = Textbox_New(
        Parent,
        text,
        Textbox_React,
        AlxFont_New(ALXFONT_BLOCKY),
        AlxFontSize,
        rect,
        Lines,
        Bg,
        Fg
    );
    return b;
}


typedef LabelEvent SelectionEvent;
typedef struct Selection {
    Renderable renderable;
} Selection;

void Selection_Render(unsigned int *Target, int Target_Width, int Target_Height,void* parent,Selection* b){
    //unsigned int FgColor = b->renderable.Fg;
    //if(Alignment_active(b->renderable.Align,ALIGN_BORDER))
}
void Selection_Event(void* parent,Selection* b,SelectionEvent be){
    if(be.eid!=EVENT_NONE){
        b->renderable.State = be.eid;
        if(b->renderable.UsedStates & EVENT_USE(be.eid)){
            if(b->renderable.React){
                b->renderable.React(parent,b,(EventId*)&be);
            }
        }
    }
}
void Selection_Update(void* parent,Selection* b,States* states,Vec2 Mouse,Vec2 MouseB){
    b->renderable.State = EVENT_NONE;
    
}
void Selection_Free(void* parent,Selection* b){

}
Selection Selection_New(Renderable* Parent,void (*Selection_React)(void*,Selection*,SelectionEvent*),Rect rect,unsigned int Bg,unsigned int Fg){
    Selection b;
    b.renderable = Renderable_New(Parent,(void(*)(unsigned int*,int,int,void*,void*))Selection_Render,(void(*)(void*,void*,EventId*))Selection_React,(void(*)(void*,void*,States*,Vec2,Vec2))Selection_Update,(void(*)(void*,void*))Selection_Free,rect,EVENT_STD_SELECTION,ALIGN_HORI_LEFT | ALIGN_VERT_TOP | ALIGN_BORDER,Bg,Fg,BLACK);
    return b;
}


typedef LabelEvent RotatableEvent;
typedef struct Rotatable {
    Renderable renderable;
    float angle;
} Rotatable;

Circle Rotatable_Circle(Rotatable* b){
    Vec2 p = Vec2_Add(b->renderable.rect.p,Vec2_Mulf(b->renderable.rect.d,0.5f));
    float r;
    if(b->renderable.rect.d.x < b->renderable.rect.d.y)     r = b->renderable.rect.d.x * 0.5f;
    else                                                    r = b->renderable.rect.d.y * 0.5f;
    return Circle_New(p,r);
}
void Rotatable_Render(unsigned int *Target, int Target_Width, int Target_Height,void* parent,Rotatable* b){
    unsigned int BgColor = b->renderable.Bg;
    Rect_Render(Target,Target_Width,Target_Height,b->renderable.rect,BgColor);

    if(Alignment_active(b->renderable.Align,ALIGN_BORDER))
        Rect_RenderWire(Target,Target_Width,Target_Height,b->renderable.rect,b->renderable.Bc,1.0f);


    Circle c = Rotatable_Circle(b);
    Vec2 t = Vec2_Add(c.p,Vec2_Mulf(Vec2_OfAngle(b->angle),c.r));

    Circle_Render(Target,Target_Width,Target_Height,c,b->renderable.Fg);
    Line_RenderX(Target,Target_Width,Target_Height,c.p,t,BLACK,1.0f);

    Circle oc = c;
    oc.r *= 0.7f;
    Circle_Render(Target,Target_Width,Target_Height,oc,Pixel_Mul(b->renderable.Fg,Pixel_toRGBA(0.8f,0.8f,0.8f,1.0f)));
}
void Rotatable_Event(void* parent,Rotatable* b,RotatableEvent be){
    if(be.eid!=EVENT_NONE){
        b->renderable.State = be.eid;
        if(b->renderable.UsedStates & EVENT_USE(be.eid)){
            if(b->renderable.React){
                b->renderable.React(parent,b,(EventId*)&be);
            }
        }
    }
}
void Rotatable_Update(void* parent,Rotatable* b,States* states,Vec2 Mouse,Vec2 MouseB){
    b->renderable.State = EVENT_NONE;
    
    Circle circle = Rotatable_Circle(b);
    if(Overlap_Circle_Point(circle,Mouse)){
        if(states[ALX_MOUSE_L].PRESSED || states[ALX_MOUSE_L].DOWN){
            b->angle = Vec2_AngleOf(Vec2_Sub(Mouse,circle.p));
            Rotatable_Event(parent,b,(RotatableEvent){ .eid = EVENT_ROTATE,.ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }
    }
}
void Rotatable_Free(void* parent,Rotatable* b){
    
}
Rotatable Rotatable_New(Renderable* Parent,void (*Rotatable_React)(void*,Rotatable*,RotatableEvent*),Rect rect,unsigned int Bg,unsigned int Fg){
    Rotatable b;
    b.renderable = Renderable_New(Parent,(void(*)(unsigned int*,int,int,void*,void*))Rotatable_Render,(void(*)(void*,void*,EventId*))Rotatable_React,(void(*)(void*,void*,States*,Vec2,Vec2))Rotatable_Update,(void(*)(void*,void*))Rotatable_Free,rect,EVENT_STD_ROTATEABLE,ALIGN_HORI_LEFT | ALIGN_VERT_TOP | ALIGN_BORDER,Bg,Fg,BLACK);
    return b;
}


#define EDITOR_LINENUMBERR     4.5

typedef LabelEvent EditorEvent;
typedef struct Editor {
    Renderable renderable;
    Tex t;
    HighLight Syntax;
    AlxFont font;
    int ScrollX;
    int ScrollY;
    int ShowLines;
    Input In;
} Editor;

void Editor_Syntax(Editor* b,char* Path){
    if(Path)    HighLight_Set(&b->Syntax,Path);
    else        HighLight_Clear(&b->Syntax);
}
void Editor_Render(unsigned int *Target, int Target_Width, int Target_Height,void* parent,Editor* tb){
    Tex_SyncString(&tb->t,&tb->In.Buffer);
    HighLight_Tex_String(&tb->Syntax,&tb->t,&tb->In.Buffer);

    Rect_Render(Target,Target_Width,Target_Height,tb->renderable.rect,tb->renderable.Bg);
    if(Alignment_active(tb->renderable.Align,ALIGN_BORDER))
        Rect_RenderWire(Target,Target_Width,Target_Height,tb->renderable.rect,tb->renderable.Bc,1.0f);

    int CurserX = Input_CurserX(&tb->In,tb->In.Curser);
    int CurserY = Input_CurserY(&tb->In,tb->In.Curser);

    const float LineNumber = (tb->ShowLines ? EDITOR_LINENUMBERR : 0.0f) * tb->font.CharSizeX;
    const float text_width = (tb->renderable.rect.d.x - LineNumber);
    const float text_height = tb->renderable.rect.d.y;
    const float text_x = (tb->renderable.rect.p.x + LineNumber);
    const float text_y = tb->renderable.rect.p.y;

    float xEnd = text_width / (float)tb->font.CharSizeX * 0.8f;
    float yEnd = text_height / (float)tb->font.CharSizeY * 0.8f;

    if(tb->In.Enabled){
        if(tb->ScrollX + CurserX < 0 || tb->ScrollX + CurserX > xEnd){
            tb->ScrollX = F32_Clamp(tb->ScrollX,-CurserX,-CurserX + (int)xEnd);
            //tb->ScrollX = -CurserX;
            //tb->ScrollX = (int)(F32_Abs(tb->ScrollX) <= F32_Abs(text_width / tb->font.CharSizeX - 5)?0: tb->ScrollX + text_width / tb->AlxFont.CharSizeX - 5);
        }
        if(tb->ScrollY + CurserY < 0 || tb->ScrollY + CurserY > yEnd){
            tb->ScrollY = F32_Clamp(tb->ScrollY,-CurserY,-CurserY + (int)yEnd);
            if(tb->In.MaxLine==1) tb->ScrollY = 0;
            //tb->ScrollY = -CurserY;
            //tb->ScrollY = (int)(F32_Abs(tb->ScrollY) <= F32_Abs(text_height / tb->font.CharSizeY - 5)?0: tb->ScrollY + text_height / tb->AlxFont.CharSizeY - 5);
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
        float x = tb->ScrollX * tb->font.CharSizeX + text_x;
        float y = text_y + (tb->ScrollY + i) * (tb->font.CharSizeY * INPUT_GAP_FAKTOR);
        
        x = F32_Clamp(x,text_x,text_x+text_width);
        y = F32_Clamp(y,text_y,text_y+text_height);
        Size = (int)F32_Clamp(Size,0,(text_width - (x - text_x)) / tb->font.CharSizeX);

        void* texture = Vector_Get(&tb->t,FirstChar);
        if(texture) TCStr_RenderSizeAlxFont(Target,Target_Width,Target_Height,&tb->font,texture,(unsigned char*)(tb->In.Buffer.Memory + FirstChar),Size,x,y);
        else        CStr_RenderSizeAlxFont(Target,Target_Width,Target_Height,&tb->font,(unsigned char*)(tb->In.Buffer.Memory + FirstChar),Size,x,y,tb->renderable.Fg);

        int TempF = String_FirstCharOfLineLast(&tb->In.Buffer,i+1,FirstChar,LastChar,i);
        int TempL = String_LastCharOfLineLast(&tb->In.Buffer,i+1,FirstChar,LastChar,i);
        FirstChar = TempF;
        LastChar = TempL;

        if(tb->ShowLines){
            CStr linetext = (CStr)Number_Get(i + 1);
            const int size = CStr_Size(linetext);
            const int posx = tb->renderable.rect.p.x + ((int)EDITOR_LINENUMBERR - size) * tb->font.CharSizeX;
            const char cursorline = tb->ScrollY + i == CurserY + tb->ScrollY; 
            CStr_RenderSizeAlxFont(Target,Target_Width,Target_Height,&tb->font,linetext,size,posx,y,cursorline ? 0xAACCCCCC : 0xAA777777);
            CStr_Free(&linetext);
        }
        if(LastChar>tb->In.Buffer.size || FirstChar>=tb->In.Buffer.size){
            break;
        }
    }

    if(tb->In.Enabled){
        Char_RenderAlxFont(Target,Target_Width,Target_Height,&tb->font,'_',text_x+(CurserX + tb->ScrollX) * tb->font.CharSizeX,text_y+(CurserY + tb->ScrollY) * (tb->font.CharSizeY * INPUT_GAP_FAKTOR),BLACK);

        if(tb->In.Curser!=tb->In.CurserEnd && tb->In.CurserEnd>=0){
            const int Up = I32_Min(tb->In.Curser,tb->In.CurserEnd);
            const int Down = I32_Max(tb->In.Curser,tb->In.CurserEnd) + 1;

            int Before = Up;
            for(int i = Up;i<tb->In.Buffer.size;i++){
                if(String_Get(&tb->In.Buffer,i)=='\n' || i == tb->In.Buffer.size - 1){
                    int X = (i+1<Down?i+1:Down) - Before;

                    float x = text_x+(Input_CurserX(&tb->In,Before) + tb->ScrollX) * tb->font.CharSizeX;
                    float y = text_y+(Input_CurserY(&tb->In,Before) + tb->ScrollY) * (tb->font.CharSizeY * INPUT_GAP_FAKTOR);
                    float w = X * tb->font.CharSizeX;
                    float h = (tb->font.CharSizeY * INPUT_GAP_FAKTOR);
                    
                    x = F32_Clamp(x,text_x,text_x+text_width);
                    y = F32_Clamp(y,text_y,text_y+text_height);
                    w = F32_Clamp(w,0.0f,text_width - (x - text_x));
                    h = F32_Clamp(h,0.0f,text_height - (y - text_y));
                    
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
void Editor_Event(void* parent,Editor* b,EditorEvent be){
    if(be.eid!=EVENT_NONE){
        b->renderable.State = be.eid;
        if(b->renderable.UsedStates & EVENT_USE(be.eid)){
            if(b->renderable.React){
                b->renderable.React(parent,b,(EventId*)&be);
            }
        }
    }
}
void Editor_Update(void* parent,Editor* tb,States* states,Vec2 Mouse,Vec2 MouseB){
    tb->renderable.State = EVENT_NONE;
    Input_Update(&tb->In,states);
    
    const float LineNumber = (tb->ShowLines ? EDITOR_LINENUMBERR : 0.0f) * tb->font.CharSizeX;
    const float text_width = (tb->renderable.rect.d.x - LineNumber);
    const float text_height = tb->renderable.rect.d.y;
    const float text_x = (tb->renderable.rect.p.x + LineNumber);
    const float text_y = tb->renderable.rect.p.y;

    if(Input_Stroke(&tb->In,ALX_MOUSE_L).PRESSED){
        if(Overlap_Rect_Point(tb->renderable.rect,Mouse)){
            tb->In.Enabled = 1;
            Editor_Event(parent,tb,(LabelEvent){ .eid = EVENT_FOCUSED,.ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }else{
            tb->In.Enabled = 0;
            Editor_Event(parent,tb,(LabelEvent){ .eid = EVENT_UNFOCUSED,.ButtonId = ALX_MOUSE_L,.pos = Mouse,.posBefore = MouseB });
        }
    }
    if(Input_Stroke(&tb->In,ALX_MOUSE_L).PRESSED){
        tb->In.Curser = Input_GetCurser(&tb->In,(Mouse.x - text_x) / tb->font.CharSizeX - tb->ScrollX,(Mouse.y - text_y) / (tb->font.CharSizeY * INPUT_GAP_FAKTOR) - tb->ScrollY);
    }
    if(Input_Stroke(&tb->In,ALX_MOUSE_L).DOWN){
        if(Mouse.x - LineNumber >= 0.0f && Mouse.y >= 0.0f)
            tb->In.CurserEnd = Input_GetCurser(&tb->In,(Mouse.x - text_x) / tb->font.CharSizeX - tb->ScrollX,(Mouse.y - text_y) / (tb->font.CharSizeY * INPUT_GAP_FAKTOR) - tb->ScrollY);
    }
    
    Input_DefaultReact(&tb->In,tb);
}
void Editor_Free(void* parent,Editor* b){
    AlxFont_Free(&b->font);
    Input_Free(&b->In);
}
Editor Editor_New(Renderable* Parent,char *text,void (*Editor_React)(void*,Editor*,EditorEvent*),AlxFont font,Vec2 AlxFontSize,Rect rect,unsigned int Lines,unsigned int Bg,unsigned int Fg){
    Editor b;
    b.font = font;
    AlxFont_Resize(&b.font,(int)AlxFontSize.x,(int)AlxFontSize.y);
    
    b.t = Vector_New(sizeof(SymbolInfo));
    b.Syntax = Vector_New(sizeof(Pair));
    
    b.ScrollX = 0;
    b.ScrollY = 0;
    b.ShowLines = 0;
    
    b.renderable = Renderable_New(Parent,(void(*)(unsigned int*,int,int,void*,void*))Editor_Render,(void(*)(void*,void*,EventId*))Editor_React,(void(*)(void*,void*,States*,Vec2,Vec2))Editor_Update,(void(*)(void*,void*))Editor_Free,rect,EVENT_STD_EDITOR,ALIGN_HORI_LEFT | ALIGN_VERT_TOP | ALIGN_BORDER,Bg,Fg,BLACK);

    b.In = Input_New(10,Lines);
    Input_SetText(&b.In,text);
    return b;
}
Editor Editor_NewStd(Renderable* Parent,char *text,void (*Editor_React)(void*,Editor*,EditorEvent*),Vec2 AlxFontSize,Rect rect,unsigned int Lines,unsigned int Bg,unsigned int Fg){
    Editor b = Editor_New(
        Parent,
        text,
        Editor_React,
        AlxFont_New(ALXFONT_BLOCKY),
        AlxFontSize,
        rect,
        Lines,
        Bg,
        Fg
    );
    return b;
}



/*
typedef PVector Scene;

Scene Scene_New(){
    Scene b = PVector_New();
    return b;
}
void Scene_Add(Scene* scene,void* r,unsigned int size){// Renderable
    PVector_Push(scene,r,size);
}
void* Scene_Add_R(Scene* scene,void* r,unsigned int size){// Renderable
    PVector_Push(scene,r,size);
    return PVector_Get(scene,scene->size-1);
}
void Scene_Update(Scene* scene,States* states,Vec2 Mouse,Vec2 MouseB){
    for(int i = 0;i<scene->size;i++){
        Renderable* r = (Renderable*)PVector_Get(scene,i);
        if(Alignment_active(r->Align,ALIGN_VISIBLE))
            if(r->Update) r->Update(r,states,Mouse,MouseB);
    }
}
void Scene_Render(unsigned int *Target, int Target_Width, int Target_Height,Scene* scene){
    for(int i = 0;i<scene->size;i++){
        Renderable* r = (Renderable*)PVector_Get(scene,i);

        Renderable_Print(r);

        if(Alignment_active(r->Align,ALIGN_VISIBLE))
            if(r->Render) r->Render(Target,Target_Width,Target_Height,scene);
    }
}
char Scene_Contains(Scene* scene,void* t){
    for(int i = 0;i<scene->size;i++){
        Renderable* r = (Renderable*)PVector_Get(scene,i);
        if(r==t && t!=NULL) return 1;
    }
    return 0;
}
void Scene_Free(Scene* scene){
    for(int i = 0;i<scene->size;i++){
        Renderable* r = (Renderable*)PVector_Get(scene,i);
        Renderable_Free(r);
    }
    PVector_Free(scene);
}
*/

typedef List Scene;

Scene Scene_New(){
    Scene b = List_New();
    return b;
}
void Scene_Add(Scene* scene,void* r,unsigned int size){// Renderable
    List_Push(scene,size,r);
}
void* Scene_Add_R(Scene* scene,void* r,unsigned int size){// Renderable
    List_Push(scene,size,r);
    return List_Get(scene,scene->size-1);
}
void Scene_RemoveI(Scene* scene,int i){
    Node* n = List_FindNode(scene,i);
    
    if(n){
        Renderable* rend = (Renderable*)n->Memory;
        Renderable_Free(scene,rend);
        
        Node_Delete(n);
        Node_Free(n);
        scene->size--;
        return;
    }
}
void Scene_Remove(Scene* scene,void* r){
    Node* n = scene->First;
    for(int i = 0;i<scene->size;i++){
        if(n->Memory == r){
            Renderable* rend = (Renderable*)r;
            if(rend->Free) rend->Free(scene,rend);

            Node_Delete(n);
            Node_Free(n);
            scene->size--;
            return;
        }
        n = n->Next;
    }
}
void Scene_RemoveQueue(Scene* scene,void* r){
    Renderable* rend = (Renderable*)r;
    if(rend) rend->Remove = 1;
}
void Scene_Update(Scene* scene,States* states,Vec2 Mouse,Vec2 MouseB){
    Node* n = scene->First;
    for(int i = 0;i<scene->size;i++){
        Renderable* r = (Renderable*)n->Memory;
        if(Alignment_active(r->Align,ALIGN_VISIBLE))
            if(r->Update) r->Update(scene,n->Memory,states,Mouse,MouseB);
        
        Node* rem = n;
        n = n->Next;

        if(r->Remove && rem){
            Scene_RemoveI(scene,i);
            i--;
        }
    }
}
void Scene_Render(unsigned int *Target, int Target_Width, int Target_Height,Scene* scene){
    Node* n = scene->First;
    for(int i = 0;i<scene->size;i++){
        Renderable* r = (Renderable*)n->Memory;
        if(Alignment_active(r->Align,ALIGN_VISIBLE))
            if(r->Render) r->Render(Target,Target_Width,Target_Height,scene,n->Memory);
        n = n->Next;
    }
}
char Scene_Contains(Scene* scene,void* t){
    Node* n = scene->First;
    for(int i = 0;i<scene->size;i++){
        Renderable* r = (Renderable*)n->Memory;
        if(r==t && t!=NULL) return 1;
        n = n->Next;
    }
    return 0;
}
void Scene_Free(Scene* scene){
    Node* n = scene->First;
    for(int i = 0;i<scene->size;i++){
        Renderable* r = (Renderable*)n->Memory;
        Renderable_Free(scene,r);
        n = n->Next;
    }
    List_Free(scene);
}
void Scene_Print(Scene* scene){
    printf("--- Scene ---\n");
    
    Node* n = scene->First;
    for(int i = 0;i<scene->size;i++){
        Renderable* r = (Renderable*)n->Memory;
        if(r){
            printf("%d: %p -> ",i,r);
            Renderable_Print_N(r);
            printf("\n");
        }
        n = n->Next;
    }
    
    printf("-------------\n");
}

#endif // !SCENE_H