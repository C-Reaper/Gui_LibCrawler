#ifndef LIB3D_MATH_H
#define LIB3D_MATH_H

#include "../Container/Vector.h"
#include "Vector2.h"
#include "Pixel.h"

typedef struct Vec2D{
	float u;
	float v;
	float w;
} Vec2D;

typedef struct Vec3D{
	float x;
	float y;
	float z;
	float w;
} Vec3D;

Vec3D Vec3D_New(float x,float y,float z){
	return (Vec3D){ x,y,z,1.0f };
}
Vec3D Vec3D_Null(){
	return (Vec3D){ 0.0f,0.0f,0.0f,1.0f };
}
Vec3D Vec3D_Add(Vec3D v1, Vec3D v2){
	return Vec3D_New( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z );
}
Vec3D Vec3D_Sub(Vec3D v1, Vec3D v2){
	return Vec3D_New( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z );
}
Vec3D Vec3D_Mul(Vec3D v1, float k){
	return Vec3D_New( v1.x * k, v1.y * k, v1.z * k );
}
Vec3D Vec3D_Div(Vec3D v1, float k){
	if(k==0.0f) return Vec3D_New( 0.0f,0.0f,0.0f );
	return Vec3D_New( v1.x / k, v1.y / k, v1.z / k );
}
float Vec3D_DotProduct(Vec3D v1, Vec3D v2){
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
float Vec3D_Length(Vec3D v){
	return sqrtf(Vec3D_DotProduct(v, v));
}
Vec3D Vec3D_Normalise(Vec3D v){
	float l = Vec3D_Length(v);
	return Vec3D_New( v.x / l, v.y / l, v.z / l );
}
Vec3D Vec3D_Perp(Vec3D v){
	return Vec3D_New( -v.z, v.y, v.x );
}
Vec3D Vec3D_Round(Vec3D v){
	return Vec3D_New( roundf(v.x),roundf(v.y),roundf(v.z) );
}
Vec3D Vec3D_CrossProduct(Vec3D v1, Vec3D v2){
	Vec3D v = Vec3D_New(0.0f,0.0f,0.0f);
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}
Vec3D Vec3D_IntersectPlane(Vec3D plane_p, Vec3D* plane_n, Vec3D lineStart, Vec3D lineEnd){
	*plane_n = Vec3D_Normalise(*plane_n);
	float plane_d = -Vec3D_DotProduct(*plane_n, plane_p);
	float ad = Vec3D_DotProduct(lineStart, *plane_n);
	float bd = Vec3D_DotProduct(lineEnd, *plane_n);
	float t = (-plane_d - ad) / (bd - ad);
	Vec3D lineStartToEnd = Vec3D_Sub(lineEnd, lineStart);
	Vec3D lineToIntersect = Vec3D_Mul(lineStartToEnd, t);
	return Vec3D_Add(lineStart, lineToIntersect);
}
float Vec3D_Dist(Vec3D plane_p, Vec3D plane_n,Vec3D p){
	//Vec3D n = Vec3D_Normalise(p);
	return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vec3D_DotProduct(plane_n, plane_p));
}


typedef struct Tri3D{
	Vec3D p[3];
	Vec3D n;
	unsigned int c;
} Tri3D;

void Tri3D_CalcNorm(Tri3D* t){
	Vec3D line1 = Vec3D_Sub(t->p[1],t->p[0]);
	Vec3D line2 = Vec3D_Sub(t->p[2],t->p[0]);
	Vec3D normal = Vec3D_CrossProduct(line1,line2);

	t->n = Vec3D_Normalise(normal);
}
void Tri3D_ShadeNorm(Tri3D* t,Vec3D dirLight){
	float l = F32_Min(F32_Max(0.2f,Vec3D_DotProduct(t->n,dirLight)),1.0f);
	t->c = Pixel_Mulf(t->c,l);
}
void Tri3D_Scale(Tri3D* t,float s){
	t->p[0] = Vec3D_Mul(t->p[0],s);
	t->p[1] = Vec3D_Mul(t->p[1],s);
	t->p[2] = Vec3D_Mul(t->p[2],s);
}
void Tri3D_Mul(Tri3D* t,Vec3D scale){
	t->p[0] = (Vec3D){ t->p[0].x * scale.x,t->p[0].y * scale.y,t->p[0].z * scale.z };
	t->p[1] = (Vec3D){ t->p[1].x * scale.x,t->p[1].y * scale.y,t->p[1].z * scale.z };
	t->p[2] = (Vec3D){ t->p[2].x * scale.x,t->p[2].y * scale.y,t->p[2].z * scale.z };
}
void Tri3D_Offset(Tri3D* t,Vec3D offset){
	t->p[0] = Vec3D_Add(t->p[0],offset);
	t->p[1] = Vec3D_Add(t->p[1],offset);
	t->p[2] = Vec3D_Add(t->p[2],offset);
}
int Tri3D_ClipAgainstPlane(Vec3D plane_p, Vec3D plane_n, Tri3D in_tri, Tri3D* out_tri1, Tri3D* out_tri2){
	// Make sure plane normal is indeed normal
	plane_n = Vec3D_Normalise(plane_n);
	
	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	Vec3D* inCUBE_SIDE_points[3];  int nInsidePointCount = 0;
	Vec3D* outCUBE_SIDE_points[3]; int nOutsidePointCount = 0;
	// Get signed distance of each point in Tri3D to plane
	float d0 = Vec3D_Dist(plane_p,plane_n,in_tri.p[0]);
	float d1 = Vec3D_Dist(plane_p,plane_n,in_tri.p[1]);
	float d2 = Vec3D_Dist(plane_p,plane_n,in_tri.p[2]);
	if (d0 >= 0) { inCUBE_SIDE_points[nInsidePointCount++] = &in_tri.p[0]; }
	else { outCUBE_SIDE_points[nOutsidePointCount++] = &in_tri.p[0]; }
	if (d1 >= 0) { inCUBE_SIDE_points[nInsidePointCount++] = &in_tri.p[1]; }
	else { outCUBE_SIDE_points[nOutsidePointCount++] = &in_tri.p[1]; }
	if (d2 >= 0) { inCUBE_SIDE_points[nInsidePointCount++] = &in_tri.p[2]; }
	else { outCUBE_SIDE_points[nOutsidePointCount++] = &in_tri.p[2]; }
	// Now classify Tri3D points, and break the input Tri3D into 
	// smaller output Tri3Ds if required. There are four possible
	// outcomes...
	if (nInsidePointCount == 0)
	{
		// All points lie on the outside of plane, so clip whole Tri3D
		// It ceases to exist
		return 0; // No returned Tri3Ds are valid
	}
	if (nInsidePointCount == 3)
	{
		// All points lie on the inside of plane, so do nothing
		// and allow the Tri3D to simply pass through
		*out_tri1 = in_tri;
		return 1; // Just the one returned original Tri3D is valid
	}
	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		// Tri3D should be clipped. As two points lie outside
		// the plane, the Tri3D simply becomes a smaller Tri3D
		// Copy appearance info to new Tri3D
		out_tri1->c =  in_tri.c;
		// The inside point is valid, so keep that...
		out_tri1->p[0] = *inCUBE_SIDE_points[0];
		// but the two new points are at the locations where the 
		// original sides of the Tri3D (lines) intersect with the plane
		out_tri1->p[1] = Vec3D_IntersectPlane(plane_p, &plane_n, *inCUBE_SIDE_points[0], *outCUBE_SIDE_points[0]);
		out_tri1->p[2] = Vec3D_IntersectPlane(plane_p, &plane_n, *inCUBE_SIDE_points[0], *outCUBE_SIDE_points[1]);
		return 1; // Return the newly formed single Tri3D
	}
	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		// Tri3D should be clipped. As two points lie inside the plane,
		// the clipped Tri3D becomes a "quad". Fortunately, we can
		// represent a quad with two new Tri3Ds
		// Copy appearance info to new Tri3Ds
		out_tri1->c =  in_tri.c;
		out_tri2->c =  in_tri.c;
		// The first Tri3D consists of the two inside points and a new
		// point determined by the location where one side of the Tri3D
		// intersects with the plane
		out_tri1->p[0] = *inCUBE_SIDE_points[0];
		out_tri1->p[1] = *inCUBE_SIDE_points[1];
		out_tri1->p[2] = Vec3D_IntersectPlane(plane_p, &plane_n, *inCUBE_SIDE_points[0], *outCUBE_SIDE_points[0]);
		// The second Tri3D is composed of one of he inside points, a
		// new point determined by the intersection of the other side of the 
		// Tri3D and the plane, and the newly created point above
		out_tri2->p[0] = *inCUBE_SIDE_points[1];
		out_tri2->p[1] = out_tri1->p[2];
		out_tri2->p[2] = Vec3D_IntersectPlane(plane_p, &plane_n, *inCUBE_SIDE_points[1], *outCUBE_SIDE_points[0]);
		return 2; // Return two newly formed Tri3Ds which form a quad
	}
	return -1;
}


typedef struct M4x4D{
	float m[4][4];
} M4x4D;

M4x4D Matrix_Null(){
	return (M4x4D){{
		{ 0.0f,0.0f,0.0f,0.0f },
		{ 0.0f,0.0f,0.0f,0.0f },
		{ 0.0f,0.0f,0.0f,0.0f },
		{ 0.0f,0.0f,0.0f,0.0f }
	}};
}
Vec3D Matrix_MultiplyVector(M4x4D m, Vec3D i){
	Vec3D v;
	v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
	v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
	v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
	v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
	return v;
}
M4x4D Matrix_MultiplyMatrix(M4x4D m1, M4x4D m2){
	M4x4D matrix = Matrix_Null();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
	return matrix;
}
M4x4D Matrix_PointAt(Vec3D pos, Vec3D target, Vec3D up){
	Vec3D newForward = Vec3D_Sub(target, pos);
	newForward = Vec3D_Normalise(newForward);
	
	Vec3D a = Vec3D_Mul(newForward,Vec3D_DotProduct(up,newForward));
	Vec3D newUp = Vec3D_Sub(up, a);
	newUp = Vec3D_Normalise(newUp);

	Vec3D newRight = Vec3D_CrossProduct(newUp,newForward);

	return (M4x4D){{
		{ newRight.x,	newRight.y,		newRight.z,		0.0f },
		{ newUp.x,		newUp.y,		newUp.z,		0.0f },
		{ newForward.x,	newForward.y,	newForward.z,	0.0f },
		{ pos.x,		pos.y,			pos.z,			1.0f }
	}};
}
M4x4D Matrix_QuickInverse(M4x4D m){
	M4x4D matrix = Matrix_Null();
	matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}
M4x4D Matrix_MakeView(Vec3D pos,Vec3D target,Vec3D up){
	Vec3D newForward = Vec3D_Normalise(Vec3D_Sub(target, pos));
	Vec3D a = Vec3D_Mul(newForward,Vec3D_DotProduct(up,newForward));
	Vec3D newUp = Vec3D_Normalise(Vec3D_Sub(up, a));
	Vec3D newRight = Vec3D_CrossProduct(newUp,newForward);

	return (M4x4D){{
		{ newRight.x,						newUp.x,						newForward.x,						0.0f },
		{ newRight.y,						newUp.y,						newForward.y,						0.0f },
		{ newRight.z,						newUp.z,						newForward.z,						0.0f },
		{ -Vec3D_DotProduct(newRight,pos),	-Vec3D_DotProduct(newUp,pos),	-Vec3D_DotProduct(newForward,pos),	1.0f }
	}};
}

M4x4D Matrix_MakeIdentity(){
	return (M4x4D){{
		{ 1.0f,0.0f,0.0f,0.0f },
		{ 0.0f,1.0f,0.0f,0.0f },
		{ 0.0f,0.0f,1.0f,0.0f },
		{ 0.0f,0.0f,0.0f,1.0f }
	}};
}
M4x4D Matrix_MakeRotationX(float fAngleRad){
	return (M4x4D){{
		{ 1.0f,0.0f,			0.0f,			0.0f },
		{ 0.0f,cosf(fAngleRad),	sinf(fAngleRad),0.0f },
		{ 0.0f,-sinf(fAngleRad),cosf(fAngleRad),0.0f },
		{ 0.0f,0.0f,			0.0f,			1.0f }
	}};
}
M4x4D Matrix_MakeRotationY(float fAngleRad){
	return (M4x4D){{
		{ cosf(fAngleRad),	0.0f,sinf(fAngleRad),	0.0f },
		{ 0.0f,				1.0f,0.0f,				0.0f },
		{ -sinf(fAngleRad),	0.0f,cosf(fAngleRad),	0.0f },
		{ 0.0f,				0.0f,0.0f,				1.0f }
	}};
}
M4x4D Matrix_MakeRotationZ(float fAngleRad){
	return (M4x4D){{
		{ cosf(fAngleRad),	sinf(fAngleRad),0.0f,0.0f },
		{ -sinf(fAngleRad),	cosf(fAngleRad),0.0f,0.0f },
		{ 0.0f,				0.0f,			1.0f,0.0f },
		{ 0.0f,				0.0f,			0.0f,1.0f }
	}};
}
M4x4D Matrix_MakeTranslation(float x,float y,float z){
	return (M4x4D){{
		{ 1.0f,	0.0f,	0.0f,	0.0f },
		{ 0.0f,	1.0f,	0.0f,	0.0f },
		{ 0.0f,	0.0f,	1.0f,	0.0f },
		{ x,	y,		z,		1.0f }
	}};
}
M4x4D Matrix_MakeProjection(float fFovDegrees,float fAspectRatio,float fNear,float fFar){
	const float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	return (M4x4D){{
		{ fAspectRatio * fFovRad,	0.0f,	0.0f,								0.0f },
		{ 0.0f,						fFovRad,0.0f,								0.0f },
		{ 0.0f,						0.0f,	fFar / (fFar - fNear),				1.0f },
		{ 0.0f,						0.0f,	(-fFar * fNear) / (fFar - fNear),	0.0f }
	}};
}
M4x4D Matrix_MakeWorld(Vec3D origin,Vec3D angle){
	const M4x4D matRotX = Matrix_MakeRotationX(angle.x);
	const M4x4D matRotY = Matrix_MakeRotationY(angle.y);
	const M4x4D matRotZ = Matrix_MakeRotationZ(angle.z);
	const M4x4D matTrans = Matrix_MakeTranslation(origin.x,origin.y,origin.z);
	const M4x4D matWorld1 = Matrix_MultiplyMatrix(matTrans,matRotY);
	const M4x4D matWorld2 = Matrix_MultiplyMatrix(matRotX,matRotZ);
	const M4x4D matWorld = Matrix_MultiplyMatrix(matWorld1,matWorld2);
	return matWorld;
}
M4x4D Matrix_MakeWorldR(Vec3D origin,Vec3D angle){
	const M4x4D matRotX = Matrix_MakeRotationX(angle.x);
	const M4x4D matRotY = Matrix_MakeRotationY(angle.y);
	const M4x4D matRotZ = Matrix_MakeRotationZ(angle.z);
	const M4x4D matTrans = Matrix_MakeTranslation(origin.x,origin.y,origin.z);
	const M4x4D matWorld1 = Matrix_MultiplyMatrix(matRotY,matRotZ);
	const M4x4D matWorld2 = Matrix_MultiplyMatrix(matRotX,matTrans);
	const M4x4D matWorld = Matrix_MultiplyMatrix(matWorld1,matWorld2);
	return matWorld;
}
M4x4D Matrix_MakePerspektive(Vec3D pos,Vec3D up,Vec3D a){
	const M4x4D matCameraRotX = Matrix_MakeRotationX(a.x);
	const M4x4D matCameraRotY = Matrix_MakeRotationY(a.y);
	const M4x4D matCameraRotZ = Matrix_MakeRotationZ(a.z);
	
	Vec3D lookdir = Vec3D_New(0.0f,0.0f,1.0f);
	lookdir = Matrix_MultiplyVector(matCameraRotX,lookdir);
	lookdir = Matrix_MultiplyVector(matCameraRotY,lookdir);
	lookdir = Matrix_MultiplyVector(matCameraRotZ,lookdir);
	
	const Vec3D target = Vec3D_Add(pos,lookdir);
	//const M4x4D matCamera = Matrix_PointAt(pos,target,up);
	//const M4x4D matView = Matrix_QuickInverse(matCamera);
	const M4x4D matView = Matrix_MakeView(pos,target,up);
	return matView;
}


typedef struct Camera {
	Vec3D p;
	Vec3D up;
	Vec3D ld;
	Vec3D sd;
	Vec3D a;
	float fov;
} Camera;

Camera Camera_New(){
	Camera c;
	c.p = (Vec3D){ 0.0f,0.0f,0.0f,1.0f };
	c.up = (Vec3D){ 0.0f,1.0f,0.0f,1.0f };
	c.ld = (Vec3D){ 0.0f,0.0f,1.0f,1.0f };
	c.ld = (Vec3D){ 1.0f,0.0f,0.0f,1.0f };
	c.a = (Vec3D){ 0.0f,0.0f,0.0f,1.0f };
	c.fov = 90.0f;
	return c;
}
Camera Camera_Make(Vec3D p,Vec3D a,float fov){
	Camera c = Camera_New();
	c.p = p;
	c.a = a;
	c.fov = fov;
	return c;
}
void Camera_Update(Camera* c){
	if(c->a.x < -3.14159f * 0.5f + 0.01f) c->a.x = -3.14159f * 0.5f + 0.01f;
	if(c->a.x >  3.14159f * 0.5f - 0.01f) c->a.x =  3.14159f * 0.5f - 0.01f;
	
	M4x4D matCameraRotY = Matrix_MakeRotationY(c->a.y);
	M4x4D matCameraRotX = Matrix_MakeRotationX(c->a.x);
	
	Vec3D lookdir = Vec3D_New(0.0f,0.0f,1.0f);
	Vec3D sidedir = Vec3D_New(1.0f,0.0f,0.0f);

	lookdir = Matrix_MultiplyVector(matCameraRotX,lookdir);
	lookdir = Matrix_MultiplyVector(matCameraRotY,lookdir);
	//sidedir = Matrix_MultiplyVector(matCameraRotX,sidedir);
	sidedir = Matrix_MultiplyVector(matCameraRotY,sidedir);

	c->ld = lookdir;
	c->sd = sidedir;
}
void Camera_Focus(Camera* c,Vec2 before,Vec2 now,Vec2 screen){
	if(now.x!=before.x || now.y!=before.y){
		Vec2 d = Vec2_Sub(now,before);
		Vec2 a = Vec2_Mulf(Vec2_Div(d,(Vec2){ screen.x,screen.y }),8.0f * 3.141592654f);
		c->a.y += a.x;
		c->a.x += a.y;
	}
}


typedef struct Rect3D {
	Vec3D p;
	Vec3D d;
} Rect3D;

char Rect3D_Overlap(Rect3D r1,Rect3D r2){
	return !(r1.p.x<r2.p.x-r1.d.x || r1.p.y<r2.p.y-r1.d.y || r1.p.z<r2.p.z-r1.d.z || r1.p.x>r2.p.x+r2.d.x || r1.p.y>r2.p.y+r2.d.y || r1.p.z>r2.p.z+r2.d.z);
}
void Rect3D_Static(Rect3D* r1,Rect3D r2,void* Data,void (**Funcs)(void*)){
	if(Rect3D_Overlap(*r1,r2)){
        Vec3D m1 = Vec3D_Add(r1->p,Vec3D_Mul(r1->d,0.5f));
        Vec3D m2 = Vec3D_Add(r2.p, Vec3D_Mul(r2.d, 0.5f));

        Vec3D l = Vec3D_Add(r1->d,r2.d);
        Vec3D d = Vec3D_Sub(m2,m1);
        d = Vec3D_New(d.x / l.x,d.y / l.y,d.z / l.z);

        if(F32_Abs(d.x)>F32_Abs(d.y)){
            if(F32_Abs(d.x)>F32_Abs(d.z)){
				if(d.x>0.0f){
                	m1.x = m2.x - l.x * 0.5f;
                	r1->p.x = m1.x - r1->d.x * 0.5f;

                    if(Funcs[0]) Funcs[0](Data);
            	}else{
            	    m1.x = m2.x + l.x * 0.5f;
            	    r1->p.x = m1.x - r1->d.x * 0.5f;

                    if(Funcs[1]) Funcs[1](Data);
            	}
			}else{
				if(d.z>0.0f){
                	m1.z = m2.z - l.z * 0.5f;
                	r1->p.z = m1.z - r1->d.z * 0.5f;

                    if(Funcs[2]) Funcs[2](Data);
            	}else{
            	    m1.z = m2.z + l.z * 0.5f;
            	    r1->p.z = m1.z - r1->d.z * 0.5f;

                    if(Funcs[3]) Funcs[3](Data);
            	}
			}
        }else{
            if(F32_Abs(d.y)>F32_Abs(d.z)){
				if(d.y>0.0f){
                	m1.y = m2.y - l.y * 0.5f;
                	r1->p.y = m1.y - r1->d.y * 0.5f;

                    if(Funcs[4]) Funcs[4](Data);
            	}else{
            	    m1.y = m2.y + l.y * 0.5f;
            	    r1->p.y = m1.y - r1->d.y * 0.5f;

                    if(Funcs[5]) Funcs[5](Data);
            	}
			}else{
				if(d.z>0.0f){
                	m1.z = m2.z - l.z * 0.5f;
                	r1->p.z = m1.z - r1->d.z * 0.5f;

                    if(Funcs[2]) Funcs[2](Data);
            	}else{
            	    m1.z = m2.z + l.z * 0.5f;
            	    r1->p.z = m1.z - r1->d.z * 0.5f;

                    if(Funcs[3]) Funcs[3](Data);
            	}
			}
        }
    }
}

#endif