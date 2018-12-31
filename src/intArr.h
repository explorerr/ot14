#ifndef INTARR_H_
#define INTARR_H_
#include <memory>
#include <iostream>
//#include "Project.h"
#include "stdlib.h"
#include "Exceptions.h"
class intArr
{
public:
	 __int64 length;
	 __int64 incrementSize;

	intArr(){
		arr = NULL;
		numArr = 0;
		incrementSize = 1;
		length = 0;
		size = 0;
		arr = NULL;
	}

	intArr( __int64 initSize){
		arr = NULL;
		numArr = 0;
		incrementSize = initSize;
		length = 0;
		size = 0;
	};
	virtual ~intArr(){
		if(arr != NULL){
			for(int i=0; i<numArr; i++)
				free(arr[i]);
			free(arr);
		}
	};

	void setIncreMentSize( __int64 size){
		incrementSize = size;
	};


	/**
	 * This is the assignment operator for this class
	 * This function will support "out of range" assignment
	 * The index must be within the CAP of the array
	 */

	 __int64 &operator[]( __int64 subscript ){
		if(subscript < 0 || subscript >= CAP){
			printf("\nError in intArr assignment operator: subscript %lld out of range %lld", subscript, length);
			throw StoppingException("\n");
		}
				
		int arrId = ( subscript )/BLOCKSIZE;
		int inArrId = (subscript) %BLOCKSIZE;

		
		if(subscript >= length){
			length = subscript + 1;
			tryResize();
		}
		return arr[arrId][inArrId];
	};

	__int64 operator[]( __int64 subscript)const{
		if(subscript <0 || length){
			printf("\nError in intArr get operator: subscript %lld out of range %lld", subscript, length);
			throw StoppingException("\n");
		}
			
		int arrId = ( subscript )/BLOCKSIZE;
		int inArrId = (subscript)%BLOCKSIZE;
		return arr[arrId][inArrId];
	};

/*	const intArr &operator=(const intArr &right){
		if(&right != this){
			if(size != right.size){
				delete [] arr;
				size =right.size;
				arr = new __int64[size];
			}
			for( __int64 i=0; i<size; i++)
				arr[i] =right.arr[i];
		}
		return *this;
	};*/

	
	
	
	void add( __int64 data ){


		length ++;
		tryResize();
		int arrId = ( length-1 )/BLOCKSIZE;
		int inArrId = (length-1) %BLOCKSIZE;
		arr[arrId][inArrId] = data;


	};




	bool has( __int64 data ){

		for(int a = 0; a<numArr-1; a++){

			for(int i=0; i<BLOCKSIZE; i++){
				if(arr[a][i]==data)
					return true;
			}

		}

		for(int i=0; i<length%BLOCKSIZE; i++)
			if(arr[numArr-1][i]==data)
				return true;
		return false;

	};

	void clear(){

		for(int i=0; i<numArr; i++)
			free(arr[i]);
		if(arr)
			free(arr);
		numArr = 0;
		arr = NULL;
		length = 0;
		size = 0;

	};

	/**
	 * This fucntion will clear the current data, and copy the data in the given array
	 */

	void copy(intArr & cpy){
		clear();

		for(__int64 i=0; i<cpy.length; i++){
			add(cpy[i]);
		}

	}
	
	
	void copy(intArr * cpy){
		clear();
		if(!cpy)
			return;
		for(__int64 i=0; i<cpy->length; i++){
			add( (*cpy)[i]);
		}

	}

	void appendArr(intArr * app){

		for(__int64 i=0; i<app->length; i++){
			add( (*app)[i] );
		}

	}






	void deepDelete(int d){
		if(d<0 || d>=length){
			printf("Error in deepDelete in inArr, subscript out of range: %d.\n", d);
			throw StoppingException("\n");
		}
		int arrId = d<BLOCKSIZE ? 0: d/BLOCKSIZE;
		int inArrId = d%BLOCKSIZE;

		if(arrId<numArr-1){ //not the last array
			for(int i=inArrId; i<BLOCKSIZE-1; i++){
				arr[arrId][i] = arr[arrId][i+1];
			}
			arr[arrId][BLOCKSIZE-1] = arr[arrId+1][0];
			for(int a=arrId+1; a<numArr; a++){

				for(int i=0; i<BLOCKSIZE-1; i++)
					arr[a][i] = arr[a][i+1];
				
				if(a!=numArr-1)
					arr[a][BLOCKSIZE-1] = arr[a+1][0];
			}

		}

		else{
			for(int i=inArrId; i<length%BLOCKSIZE-1; i++){
				arr[arrId][i] = arr[arrId][i+1];
			}


		}
		
		
		length--;
	}

		
	void print(){


		for(int a=0; a<numArr; a++){
			int end;
			if(a==numArr-1){
				end = length%BLOCKSIZE;
			}
			else
				end= BLOCKSIZE;

			for(int i=0; i<end; i++){
				std::cout<<arr[a][i]<<std::endl;
			}

		}

	}


private:
	__int64 size;
	__int64** arr;
	int numArr;
	static const __int64 CAP =ARRLONGCAP;

	static const __int64 BLOCKSIZE = 1000;

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

		while(length > size){
			
			if(size==0){
				numArr=1;
				arr = (__int64 **) realloc(arr, sizeof(__int64*)*numArr);
				arr[0] = NULL;
			}

			size = size + incrementSize;
			
			
			if(size>BLOCKSIZE*numArr){
				numArr++;
				size=numArr*BLOCKSIZE;
				arr = (__int64 **) realloc(arr, sizeof(__int64*)*numArr);
				arr[numArr-1] = (__int64*) malloc(sizeof(__int64)*BLOCKSIZE);
							
				if(!arr[numArr-1])
					throw StoppingException("Error in intArr: memory allocation failed in intArr.\n");
			}

			else{
				
				arr[0] = (__int64 *) realloc(arr[0], sizeof(__int64)*size);
				if(!arr[0])
					throw StoppingException("Error in intArr: memory allocation failed in intArr.\n");

			}


		}
	};
};

#endif /*INTARR_H_*/
