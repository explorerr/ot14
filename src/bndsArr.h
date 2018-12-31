#ifndef BNDSARR_H_
#define BNDSARR_H_
#include <memory>
#include <iostream>
#include "Project.h"
class bndsArr
{
public:
	 __int64 length;
	 __int64 incrementSize;

	 bndsArr(){
		arr = NULL;
		incrementSize = 100;
		length = 0;
		size = 0;
		arr = NULL;
	}

	 bndsArr( __int64 initSize){
		arr = NULL;
		incrementSize = initSize;
		length = 0;
		size = 0;
	};
	virtual ~bndsArr(){
		if(arr != NULL)
			free(arr);
	};

	void setIncreMentSize( __int64 size){
		incrementSize = size;
	};

	 intArr* operator[]( __int64 subscript)const{
		if(subscript <0 || length){
			printf("\nError in intArr get operator: subscript %lld out of range %lld", subscript, length);
			throw StoppingException("\n");
		}
		return arr[subscript];
	};

	void add( intArr* data ){
		length ++;
		tryResize();
		arr[length-1]= new intArr(3);
		for(__int64 i=0; i<data->length; i++){
			arr[length-1]->add((*data)[i]);
		}

	}

	bool has( __int64 data ){

		for(__int64 i=0; i<length; i++){
			if( arr[i]->has(data) )
				return true;
		}
		return false;

	}

	void clear(){
		if(arr){
			for(__int64 i=0; i<length; i++){
				delete(arr[i]);
			}
			free(arr);
		}

		arr = NULL;
		incrementSize = 1;
		length = 0;
		size = 0;
		arr = NULL;

	}


	void copy(bndsArr & cpy){

		for(__int64 i=0; i<cpy.length; i++){
			add(cpy[i]);
		}

	}










private:
	__int64 size;
	intArr** arr;
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
//		if(length>CAP){
//			printf("Error in intArr: array length larger than allowed capacity: %lld > %lld", length, CAP);
//			throw StoppingException("\n");
	//	}
		while(length >= size){
			size = size + incrementSize;
			arr = (intArr**) realloc(arr, sizeof(intArr*)*size);
			if(!arr)
				throw StoppingException("Error in intArr: memory allocation failed in intArr.\n");
			for( __int64 i=size-incrementSize; i<size; i++){
				arr[i] = NULL;
			}
		}
	};
};

#endif /*BNDSARR_H_*/
