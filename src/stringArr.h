#ifndef STRINGARR_H_
#define STRINGARR_H_
#include <memory>
#include <iostream>
//#include "Project.h"
#include "Exceptions.h"
#include "math.h"
class stringArr
{
public:

	__int64 length;
	__int64 incrementSize;

	stringArr(){
		arr = NULL;
		incrementSize = 10;
		length = 0;
		size = 0;
	}

	stringArr( __int64 initSize){
		arr = NULL;
		incrementSize = initSize;
		length = 0;
		size = 0;
	};
	virtual ~stringArr(){
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

	 char* &operator[]( __int64 subscript ){
		if(subscript < 0 || subscript >= CAP){
			printf("\nError in stringArr assignment operator: subscript %lld out of range %lld\n", subscript, length);
			throw StoppingException("\n");
		}

		if(subscript >= length){
			length = subscript + 1;
			tryResize();
		}
		return arr[subscript];
	};

	 char* operator[]( __int64 subscript)const{
		if(subscript <0 || subscript >= length){
			printf("\nError in stringArr get operator: subscript %lld out of range %lld\n", subscript, length);
			throw StoppingException("\n");
		}
		return arr[subscript];
	};

	const stringArr &operator=(const stringArr &right){
		if(&right != this){
			if(size != right.size){
				free(arr);
				size =right.size;
				arr = (char**) malloc(sizeof(char*)*size);
			}
			for( __int64 i=0; i<size; i++)
				add(right.arr[i]);
		}
		return *this;
	};

	 __int64 add( char* string){

		length ++;
		tryResize();
		copyString (string, length-1);
		return length -1;
		
	}

	void copyString(char* string, __int64 subscript){
		if(subscript <0 || subscript >=length){
			printf("\nError in stringArr get operator: subscript %lld out of range %lld\n", subscript, length);
			throw StoppingException("\n");
		}
		arr[subscript] = (char*)malloc( sizeof(char)*(strlen(string)+1) );
		if(!arr[subscript]){
			printf("\nError in stringArr memory allocation failed.\n");
			throw StoppingException("\n");
		}
		strcpy(arr[subscript], string);


	}


    bool has( char* string){

		for(__int64 i=0; i<length; i++)
			if(strcmp(string, arr[i])==0)
				return true;
		return false;

    }

    void clear(){
    	length = 0;
    	size = 0;
    	free(arr);
    	arr= NULL;

    }




private:
	char** arr;
	__int64 size;
	static const  __int64 CAP =1000000000;


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
//		}
		while(length >= size){
			size = size + incrementSize;
			arr = (char**) realloc(arr, sizeof(char*)*size);
			if(!arr)
				throw StoppingException("Error in intArr: memory allocation failed in intArr.\n");
			for( __int64 i=size-incrementSize; i<size; i++){
				arr[i] = NULL;
			}
		}
	};




};

#endif /*STRINGARR_H_*/
