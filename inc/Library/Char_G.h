#ifndef CHAR_G_H
#define CHAR_G_H

#include "Char.h"
#include "Sprite.h"
#include "AlxFont.h"

void Char_RenderAlxFont(unsigned int* Target,int Target_Width,int Target_Height,AlxFont* f,unsigned char c,float x,float y,unsigned int Color){
    float ox = (float)(c % f->Columns)  * f->CharSizeX;
    float oy = (float)(c / f->Rows)     * f->CharSizeY;
    Sprite_RenderSubAlphaTint(Target,Target_Width,Target_Height,&f->Atlas,x,y,ox,oy,f->CharSizeX,f->CharSizeY,Color);
}

#endif // !CHAR_G_H