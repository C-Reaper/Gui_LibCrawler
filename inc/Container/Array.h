#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../Library/Compare.h"

typedef int Index_t;

#define INDEX_INVALID   -1

typedef struct Array {
    unsigned int SIZE;
    unsigned int ELEMENT_SIZE;
    void* Memory;
} Array;

Array Array_New(unsigned int ELEMENT_SIZE,unsigned int SIZE) {
    Array a;
    a.ELEMENT_SIZE = ELEMENT_SIZE;
    a.SIZE = SIZE;
    a.Memory = calloc(SIZE,ELEMENT_SIZE);
    memset(a.Memory,0,a.SIZE * a.ELEMENT_SIZE);
    return a;
}

void* Array_Get(Array* v, int Index) {
    return (v->Memory + Index * v->ELEMENT_SIZE);
}

void* Array_GetCpy(Array* v, int Index) {
    void* Ptr = (v->Memory + Index * v->ELEMENT_SIZE);
    void* Out = malloc(v->ELEMENT_SIZE);
    memcpy(Out,Ptr,v->ELEMENT_SIZE);
    return Out;
}

void Array_Set(Array* v, void* Item, int Index) {
    memcpy(v->Memory + Index * v->ELEMENT_SIZE,Item,v->ELEMENT_SIZE);
}

void Array_Print(Array* a){
    printf("Array: -----------\n");
    printf(" Elementsize: %d\n",a->ELEMENT_SIZE);
    printf(" Size: %d\n",a->SIZE);
    for(int i = 0;i<a->SIZE;i++){
        printf("  Element %d: ",i);
        char* Item = (char*)Array_Get(a,i);
        for(int j = 0;j<a->ELEMENT_SIZE;j++){
            printf("%c",Item[j]);
        }
        printf("\n");
    }
    printf("------------------\n");
}

void Array_Free(Array* a) {
    free(a->Memory);
}


void Array_Swap(Array* a,int Index1,int Index2){
    void* Item1 = Array_GetCpy(a,Index1);
    void* Item2 = Array_Get(a,Index2);
    Array_Set(a,Item2,Index1);
    Array_Set(a,Item1,Index2);
    free(Item1);
}

Index_t Array_Search_Linear(Array* a,void* Item,CMP_BOOL (*Cmp)(size_t ElementSize,void* e1,void* e2)){
    for(Index_t i = 0;i<a->SIZE;i++){
        if(Cmp(a->ELEMENT_SIZE,Item,Array_Get(a,i)) & CMP_EQUAL){
            return i;
        }
    }
    return INDEX_INVALID;
}

Index_t Array_Search_Binary(Array* a,void* Item,CMP_BOOL (*Cmp)(size_t ElementSize,void* e1,void* e2)){
    int SizeLeft = a->SIZE / 2;
    int Index = SizeLeft;
    while(SizeLeft>=1){
        CMP_BOOL b = Cmp(a->ELEMENT_SIZE,Item,Array_Get(a,Index));
        if(b & CMP_LESS){
            SizeLeft >>= 1;
            Index -= SizeLeft;
        }else if(b & CMP_GREATER){
            SizeLeft >>= 1;
            Index += SizeLeft;
        }else if(b & CMP_EQUAL){
            return Index;
        }
    }
    if(Cmp(a->ELEMENT_SIZE,Item,Array_Get(a,Index-1)) & CMP_EQUAL) return Index-1;
    if(Cmp(a->ELEMENT_SIZE,Item,Array_Get(a,Index+1)) & CMP_EQUAL) return Index+1;
    return INDEX_INVALID;
}

Index_t Array_Search_Interpolation(Array* a,void* Item,CMP_BOOL (*Cmp)(size_t ElementSize,void* e1,void* e2)){
    int High = a->SIZE - 1;
	int Low = 0;
	
	while(Cmp(a->ELEMENT_SIZE,Item,Array_Get(a,Low)) & (CMP_GREATER | CMP_EQUAL) && Cmp(a->ELEMENT_SIZE,Item,Array_Get(a,High)) & (CMP_LESS | CMP_EQUAL) && Low <= High) {
		Index_t Probe = Low + (High - Low) * Cmp_NumberRepSub(a->ELEMENT_SIZE,Item,Array_Get(a,Low)) / Cmp_NumberRepSub(a->ELEMENT_SIZE,Array_Get(a,High),Array_Get(a,Low));
		
        CMP_BOOL b = Cmp(a->ELEMENT_SIZE,Item,Array_Get(a,Probe));
		if(b & CMP_EQUAL)           return Probe;
		else if(b & CMP_GREATER)    Low = Probe + 1;
		else                        High = Probe - 1;
	}
    return INDEX_INVALID;
}

void Array_Sort_Bubble(Array* a,CMP_BOOL (*Cmp)(size_t ElementSize,void* e1,void* e2)){
    for(Index_t i = 0;i<a->SIZE;i++){
        int Swaps = 0;
        for(Index_t j = 0;j<a->SIZE-1;j++){
            CMP_BOOL b = Cmp(a->ELEMENT_SIZE,Array_Get(a,j),Array_Get(a,j+1));
            if(b & CMP_GREATER){
                Array_Swap(a,j,j+1);
                Swaps++;
            }
        }
        if(Swaps==0) break;
    }
}

void Array_Sort_Selection(Array* a,CMP_BOOL (*Cmp)(size_t ElementSize,void* e1,void* e2)){
    for(Index_t i = 0;i<a->SIZE;i++){
        Index_t Min = i;
        for(Index_t j = i;j<a->SIZE;j++){
            CMP_BOOL b = Cmp(a->ELEMENT_SIZE,Array_Get(a,Min),Array_Get(a,j));
            if(b & CMP_GREATER) Min = j;
        }
        Array_Swap(a,i,Min);
    }
}

void Array_Sort_Insertion(Array* a,CMP_BOOL (*Cmp)(size_t ElementSize,void* e1,void* e2)){
    for(Index_t i = 1;i<a->SIZE;i++){
        for(Index_t j = i;j>0;j--){
            CMP_BOOL b = Cmp(a->ELEMENT_SIZE,Array_Get(a,j-1),Array_Get(a,j));
            if(b & CMP_GREATER) Array_Swap(a,j-1,j);
            else                break;
        }
    }
}

void Array_Merge(int Elements,int* a,int* b,CMP_BOOL (*Cmp)(size_t ElementSize,void* e1,void* e2)){
    //int leftSize = array.length / 2;
	//int rightSize = array.length - leftSize;
	//int i = 0, l = 0, r = 0; //indices
	//
	////check the conditions for merging
	//while(l < leftSize && r < rightSize) {
	//	if(leftArray[l] < rightArray[r]) {
	//		array[i] = leftArray[l];
	//		i++;
	//		l++;
	//	}
	//	else {
	//		array[i] = rightArray[r];
	//		i++;
	//		r++;
	//	}
	//}
	//while(l < leftSize) {
	//	array[i] = leftArray[l];
	//	i++;
	//	l++;
	//}
	//while(r < rightSize) {
	//	array[i] = rightArray[r];
	//	i++;
	//	r++;
	//}
}

void Array_Sort_Merge(Array* a,CMP_BOOL (*Cmp)(size_t ElementSize,void* e1,void* e2)){
    //int length = array.length;
	//if (length <= 1) return; //base case
	//
	//int middle = length / 2;
	//int[] leftArray = new int[middle];
	//int[] rightArray = new int[length - middle];
	//
	//int i = 0; //left array
	//int j = 0; //right array
	//
	//for(; i < length; i++) {
	//	if(i < middle) {
	//		leftArray[i] = array[i];
	//	}
	//	else {
	//		rightArray[j] = array[i];
	//		j++;
	//	}
	//}
	//Array_Sort_Merge(leftArray);
	//Array_Sort_Merge(rightArray);
	//Array_Merge(leftArray, rightArray, array);
}

void Array_Sort_Quick(Array* a,CMP_BOOL (*Cmp)(size_t ElementSize,void* e1,void* e2)){
    
}

#endif