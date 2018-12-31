/*
 * pointerArr.h
 *
 *  Created on: Oct 7, 2010
 *      Author: Rui Zhang
 */

#ifndef POINTERARR_H_
#define POINTERARR_H_

#include <memory>
#include <iostream>
#include "Project.h"
class pointerArr
{
public:
	int length;
	int incrementSize;

	pointerArr(){
		arr = NULL;
		incrementSize = 10;
		length = 0;
		size = 0;
		arr = NULL;
	}

	pointerArr(int initSize){
		arr = NULL;
		incrementSize = initSize;
		length = 0;
		size = 0;
	};
	virtual ~pointerArr(){
		if(arr != NULL)
			free(arr);
	};

	void setIncreMentSize(int size){
		incrementSize = size;
	};


	/**
	 * This is the assignment operator for this class
	 * This function will support "out of range" assignment
	 * The index must be within the CAP of the array
	 */

	void* &operator[](int subscript ){
		if(subscript < 0 || subscript >= CAP){
			throw StoppingException("\n");
		}

		if(subscript >= length){
			length = subscript + 1;
			tryResize();
		}
		return arr[subscript];
	};

	void* operator[](int subscript)const{
		if(subscript <0 || length){
			throw StoppingException("\n");
		}
		return arr[subscript];
	};

	const pointerArr &operator=(const pointerArr &right){
		if(&right != this){
			if(size != right.size){
				delete [] arr;
			}
			for(int i=0; i<size; i++)
				arr[i] =right.arr[i];
		}
		return *this;
	};
	
	//shallow copy
	void add(void* data){
		length ++;
		tryResize();
		arr[length-1] = data;
	}

	void deleteData(int id){
		if( id>length || id<0) {
			printf("Error in pointerArr deleteData: index out of range %d.\n", id);
			throw StoppingException("\n");
		}
		for(int i=id; i<length-1; i++)
			arr[i] = arr[i+1];
		length--;

	};


	void shallowDelete(int id){
		if(id>length-1){
			printf("Error in pointerArr, shallowDelete, id larger than total number of data in array, [%d]\n", id);
			throw StoppingException("\n");
		}
		length --;
		for(int i=id; i<length; i++){
			arr[i] = arr[i+1];
		}
	}

private:
	int size;
	void** arr;
	static const int CAP =ARRLONGCAP;


	/**
	 * This function will compare the current length of the array and its size
	 *
	 * The current length is supposed to be updated in the calling function
	 * The calling function need to change the length first, and call the tryResize()
	 * before the function tries to change the content of the array
	 *
	 * This function will check if the length is larger than the CAP, if larger will print error message
	 */
	void tryResize() {
//		if(length>CAP){
//			throw StoppingException("\n");
//		}
		while(length >= size){
			size = size + incrementSize;
			arr = (void**) realloc(arr, sizeof(void*)*size);
			if(!arr)
				throw StoppingException("Error in pointerArr: memory allocation failed in intArr.\n");
			for(int i=size-incrementSize; i<size; i++){
				arr[i] = NULL;
			}
		}
	};
};

#endif /* POINTERARR_H_ */
