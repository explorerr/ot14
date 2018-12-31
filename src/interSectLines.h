/*
 * interSectLines.h
 *
 *  Created on: Nov 14, 2009
 *      Author: zrui
 */

#ifndef INTERSECTLINES_H_
#define INTERSECTLINES_H_
#include "Exceptions.h"
#include "Face.h"
#include <vector>

class interSectLines {
public:
	interSectLines(){
		arr = NULL;
		interLines = NULL;
		length = 0;
	}
	virtual ~interSectLines(){
		if(!arr)
			free(arr);
		for(int i=0; i<length; i++){
			for(int j=0; j<arr[i][2]; j++)
				free(interLines[i][j]);
			free(interLines[i]);
		}
		free(interLines);
	}


	void addInterLine(int a, int b, vector<double> interLineInfo, int type){
		int aa = (a<b)? a: b;
		int bb = (a>b)? a: b;
		int found = -1;
		for(int i=0; i<length; i++){
			if(arr[i][0] == aa && arr[i][1] == bb){
				found = i;
				break;
			}
		}
		if(found != -1){
			arr[found][2] ++;
		}
		else{
			tryResize();
			arr[length-1][0] = aa;
			arr[length-1][1] = bb;
			arr[length-1][2] = 1;
		}
		interLines[found] = (Face**) realloc(interLines[found], sizeof(Face*)*arr[found][2]);
		interLines[found][arr[found][2]-1] = new Face(type, interLineInfo);

	}

	void addInterLine(int a, int b, vector<Face*>& lines){

		int aa = (a<b)? a: b;
		int bb = (a>b)? a: b;
		int found = -1;
		for(int i=0; i<length; i++){
			if(arr[i][0] == aa && arr[i][1] == bb){
				found = i;
				break;
			}
		}
		int oldn;
		if(found != -1){
			oldn = arr[found][2];
			arr[found][2] += lines.size();
		}
		else{
			tryResize();
			oldn = 0;
			found = length-1;
			arr[found][0] = aa;
			arr[found][1] = bb;
			arr[found][2] = lines.size();
		}
		interLines[found] = (Face**) realloc(interLines[found], sizeof(Face*)*arr[found][2]);
		for(unsigned int i=0; i<lines.size(); i++){
			interLines[found][i+oldn] = lines[i];
		}

	}

	/**
	 * for faces with no interSectline
	 * will set arr[i][2]=0
	 */
	void addInterLine(int a, int b){
		int aa = (a<b)? a: b;
		int bb = (a>b)? a: b;
		int found = -1;
		for(int i=0; i<length; i++){
			if(arr[i][0] == a ){
				found = i;
				break;
			}
		}
		if(found != -1){
			return;
		}
		else{
			tryResize();
			arr[length-1][0] = aa;
			arr[length-1][1] = bb;
			arr[length-1][2] = 0;
		}
	}

	/**
	 * Will return number of interlines
	 * -1 == this interline is not calculated yet
	 * 0  == there is no interLine
	 * n  == number of interlines
	 */

	int getNumberOfInterLine(int a, int b ){
		int aa = (a<b)? a: b;
		int bb = (a>b)? a: b;
		int found = -1;
		for(int i=0; i<length; i++){
			if(arr[i][0] == aa && arr[i][1] == bb){
				found = i;
				break;
			}
		}
		if(found == -1)
			return -1;
		else
			return arr[found][2];
	}

	Face* getInterLine(int a, int b, int n){
		int aa = (a<b)? a: b;
		int bb = (a>b)? a: b;
		int found = -1;
		for(int i=0; i<length; i++){
			if(arr[i][0] == aa && arr[i][1] == bb){
				found = i;
				break;
			}
		}
		if(found == -1){
			return NULL;
		}
		else if(n>arr[found][2]){
				throw StoppingException("Error: in interSectLines->getInerLine, interLine index out bound.\n");
		}
		else{
			return interLines[found][n];
		}
	}


private:
	int (*arr)[3];
	Face ***interLines;
	int length;


	void tryResize(){
		length++;
		try{
			arr = (int (*)[3]) realloc(arr, sizeof(int)*3*length);
			interLines = (Face***) realloc(interLines, sizeof(Face**)*length);
			interLines[length-1] = NULL;
		}
		catch(...){
			throw StoppingException("Error: in interSecLines object, memory allocation failed.\n");
		}
	}
};

#endif /* INTERSECTLINES_H_ */
