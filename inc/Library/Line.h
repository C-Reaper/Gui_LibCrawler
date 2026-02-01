#ifndef LINE_H
#define LINE_H

#include "Intrinsics.h"
#include "Point.h"
#include "Math.h"

typedef struct Line{
    Vec2 s;
    Vec2 e;
} Line;

Line Line_New(Vec2 s,Vec2 e){
    Line l;
    l.s = s;
    l.e = e;
    return l;
}
float Line_Y(Line l,float x){
    Vec2 d = Vec2_Sub(l.e,l.s);
    float m = INFINITY;
    if(d.x!=0.0f) m = d.y / d.x;
    float t = -m * l.s.x + l.s.y;
    return m * x + t;
}
float Line_X(Line l,float y){
    Vec2 d = Vec2_Sub(l.e,l.s);
    float m = INFINITY;
    if(d.x!=0.0f) m = d.y / d.x;
    float t = -m * l.s.x + l.s.y;
    return (y - t) / m;
}

Vec2 Line_Clamp(Line l,Vec2 p,Vec2 min,Vec2 max){
    if(p.x<min.x){
        p.x = min.x;
        p.y = Line_Y(l,p.x);
    }
    if(p.y<min.y){
        p.y = min.y;
        p.x = Line_X(l,p.y);
    }
    if(p.x>max.x){
        p.x = max.x;
        p.y = Line_Y(l,p.x);
    }
    if(p.y>max.y){
        p.y = max.y;
        p.x = Line_X(l,p.y);
    }
    return p;
}

void Line_Render(unsigned int* Target,int Target_Width,int Target_Height,Line l,unsigned int c,F32 StrokeSize){
	if(l.s.x < 0.0f && l.e.x < 0.0f)                        return;
    if(l.s.x >= Target_Width && l.e.x >= Target_Width)      return;
    if(l.s.y < 0.0f && l.e.y < 0.0f)                        return;
    if(l.s.y >= Target_Height && l.e.y >= Target_Height)    return;
    
    Vec2 d = Vec2_Norm(Vec2_Sub(l.e,l.s));

    F32 m = 1.0f;
    if(d.x!=0.0f){
        m = d.y / d.x;
        F32 t = -m * l.s.x + l.s.y;
        if(l.s.y<1.0f || l.s.y>=Target_Height-1.0f){
            l.s.y = l.s.y<1.0f?1.0f:(l.s.y>Target_Height-1.0f?Target_Height-1.0f:l.s.y);
            l.s.x = (l.s.y - t) / m;
        }
        if(l.e.y<1.0f || l.e.y>=Target_Height-1.0f){
            l.e.y = l.e.y<1.0f?1.0f:(l.e.y>Target_Height-1.0f?Target_Height-1.0f:l.e.y);
            l.e.x = (l.e.y - t) / m;
        }
        if(l.s.x<1.0f || l.s.x>=Target_Width-1.0f){
            l.s.x = l.s.x<1.0f?1.0f:(l.s.x>Target_Width-1.0f?Target_Width-1.0f:l.s.x);
            l.s.y = m * l.s.x + t;
        }
        if(l.e.x<1.0f || l.e.x>=Target_Width-1.0f){
            l.e.x = l.e.x<1.0f?1.0f:(l.e.x>Target_Width-1.0f?Target_Width-1.0f:l.e.x);
            l.e.y = m * l.e.x + t;
        }
    }else{
        if(l.s.y<1.0f || l.s.y>=Target_Height-1.0f){
            l.s.y = l.s.y<1.0f?1.0f:(l.s.y>Target_Height-1.0f?Target_Height-1.0f:l.s.y);
        }
        if(l.e.y<1.0f || l.e.y>=Target_Height-1.0f){
            l.e.y = l.e.y<1.0f?1.0f:(l.e.y>Target_Height-1.0f?Target_Height-1.0f:l.e.y);
        }
    }

    F32 w = 0.0f;
    F32 L = Vec2_Mag(Vec2_Sub(l.e,l.s));
    while(w<L){
        l.s = Vec2_Add(l.s,d);
        w += 1.0f;

        float y = l.s.y;
        if(StrokeSize == 1.0f) {
            if(l.s.x<1.0f||l.s.x>=Target_Width-1||y<1.0f||y>=Target_Height-1)
                continue;
            
			if(l.s.x>=0.0f && l.s.x<Target_Width && y>=0.0f && y<Target_Height)
                Target[(int)y * Target_Width + (int)l.s.x] = c;
        }else{
            for(float i = -StrokeSize;i<StrokeSize;i+=1.0f){
                float x = l.s.x + Vec2_Perp(d).x*i;
                y = l.s.y + Vec2_Perp(d).y*i;
                if(x<1.0f||x>=Target_Width-1||y<1.0f||y>=Target_Height-1)
                    continue;
                
                if(l.s.x>=0.0f && l.s.x<Target_Width && y>=0.0f && y<Target_Height)
                    Target[(int)y * Target_Width + (int)l.s.x] = c;
            }
        }
    }
}
void Line_RenderX(unsigned int* Target,int Target_Width,int Target_Height,Vec2 s,Vec2 e,unsigned int c,F32 StrokeSize){
	Line_Render(Target,Target_Width,Target_Height,Line_New(s,e),c,StrokeSize);
}

#endif // !LINE_H