#ifndef DBLARR_H_
#define DBLARR_H_
#include <memory>
#include <iostream>
#include "Project.h"
class dblArr
{
public:
	 __int64 length;
	 __int64 incrementSize;

	dblArr(){
		arr = NULL;
		incrementSize = 100;
		length = 0;
		size = 0;
		arr = NULL;
	}

	dblArr( __int64 initSize){
		arr = NULL;
		incrementSize = initSize;
		length = 0;
		size = 0;
	};
	virtual ~dblArr(){
		if(arr != NULL)
			free(arr);
	};

	void setIncreMentSize( __int64 size){
		incrementSize = size;
	};


	/**
	 * This is the assignment operator for this class
	 * This function will support "out of range" assignment
	 * The index must be within the CAP of the array
	 */

	 double &operator[]( __int64 subscript ){
		if(subscript < 0 || subscript >= CAP){
			printf("\nError in intArr assignment operator: subscript %lld out of range %lld", subscript, length);
			throw StoppingException("\n");
		}

		if(subscript >= length){
			length = subscript + 1;
			tryResize();
		}
		return arr[subscript];
	};

	 double operator[]( __int64 subscript)const{
		if(subscript <0 || length){
			printf("\nError in intArr get operator: subscript %lld out of range %lld", subscript, length);
			throw StoppingException("\n");
		}
		return arr[subscript];
	};

	const dblArr &operator=(const dblArr &right){
		if(&right != this){
			if(size != right.size){
				free(arr);
				size =right.size;
				arr = (double*) malloc(sizeof(double), size);
			}
			for( __int64 i=0; i<size; i++)
				arr[i] =right.arr[i];
		}
		return *this;
	};

	void add( double data ){
		length ++;
		tryResize();
		arr[length-1] = data;

	}

	bool has( double data ){

		for(__int64 i=0; i<length; i++){
			if(arr[i]==data)
				return true;
		}
		return false;

	}

	void clear(){
		if(arr)
			free(arr);
		arr = NULL;
		incrementSize = 1;
		length = 0;
		size = 0;
		arr = NULL;

	}

	/**
	 * This fucntion will clear the current data, and copy the data in the given array
	 */

	void copy(dblArr & cpy){
		clear();
		for(int i=0; i<cpy.length; i++){
			add(cpy[i]);
		}

	}
	void copy(dblArr * cpy){
		clear();
		for(int i=0; i<cpy->length; i++){
			add( (*cpy)[i]);
		}

	}

	void appendArr(dblArr * app){

		for(int i=0; i<app->length; i++){
			add( (*app)[i] );
		}

	}


	void print(){
		for(__int64 i=0; i<length; i++){
			cout<<arr[i]<<endl;
		}
	}

	bool equal(dblArr & aa){

		if(aa.length != length)
			return false;
		for(int i=0; i<length; i++){
			if( !aa.has(arr[i]) )
				return false;
		}
		return true;

	}








private:
	__int64 size;
	double* arr;
	static const __int64 CAP =ARRLONGCAP;


	/**
	 * This function will compare the current length of the array and its size
	 *
	 * The current length is supposed to be updated in the calling function
	 * The calling function need to change the length first, and call the tryResize()
	 * before the function tries to change the content of the array
	 *
	 * This function will check if the length is larger than the CAP, if larger will pr __int64 error message
	 */
	void tryResize() {
		while(length >= size){
			size = size + incrementSize;
			arr = (double*) realloc(arr, sizeof(double)*size);
			if(!arr)
				throw StoppingException("Error in intArr: memory allocation failed in intArr.\n");
			for( __int64 i=size-incrementSize; i<size; i++){
				arr[i] = -1;
			}
		}
	};
};

#endif /*DBLARR_H_*/
