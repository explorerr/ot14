#ifndef STRINGARRNONDUP_H_
#define STRINGARRNONDUP_H_
#include <memory>
#include <iostream>
#include "Project.h"
#include "math.h"
class stringArrNonDup
{
public:

	__int64 length;
	__int64 incrementSize;

	stringArrNonDup(){
		arr = NULL;
		incrementSize = 10;
		length = 0;
		size = 0;
	}

	stringArrNonDup( __int64 initSize){
		arr = NULL;
		incrementSize = initSize;
		length = 0;
		size = 0;
	};
	virtual ~stringArrNonDup(){
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
			printf("\nError in stringArrNonDup assignment operator: subscript %lld out of range %lld\n", subscript, length);
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
			printf("\nError in stringArrNonDup get operator: subscript %lld out of range %lld\n", subscript, length);
			throw StoppingException("\n");
		}
		return arr[subscript];
	};

	const stringArrNonDup &operator=(const stringArrNonDup &right){
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


		__int64 pos = find(string);

		if(pos!=-1){//already in the array
			return pos;
		}
		//no dups in the array then add this string into the array
		length ++;
		tryResize();
		copyString (string, length-1);
		return length -1;
		
	}

	void copyString(char* string, __int64 subscript){
		if(subscript <0 || subscript >=length){
			printf("\nError in stringArrNonDup get operator: subscript %lld out of range %lld\n", subscript, length);
			throw StoppingException("\n");
		}
		arr[subscript] = (char*)malloc( sizeof(char)*(strlen(string)+1) );
		if(!arr[subscript]){
			printf("\nError in stringArrNonDup memory allocation failed.\n");
			throw StoppingException("\n");
		}
		strcpy(arr[subscript], string);


	}


    bool has( char* string){

    	__int64 ret = find(string);

    	if(ret==-1)
    		return false;
    	else
    		return true;

    }

    __int64 find( char* string){

    	__int64 ret = -1;
    	for(__int64 i=0; i<length; i++){
			if(strcmp(string, arr[i])==0){
				ret = i;
			}
    	}
		return ret;

    }

    void clear(){
    	length = 0;
    	size = 0;
    	free(arr);
    	arr= NULL;

    }

	

	void toFile(char* fName, char* ext){

    	char* fullFilePath = NULL;
        getFullFilePath(&fullFilePath, fName, ext);
        FILE * file = NULL;
 /*       FILE* file = fopen(fullFilePath, "w");

    	for(int i=0; i<length; i++){
            fprintf(file, "%d %s\n", i, arr[i]);
    	}
    	free(fullFilePath);
    	fclose(file);

*/
    	//output binary file

//    	fullFilePath = NULL;
//   	getFullFilePath(&fullFilePath, fName, ext);
    	file = fopen(fullFilePath, "wb");

        if( !file ){
            fprintf(stderr, "error opening bstr file for output.\n");
            throw StoppingException("\n");
        }
    	for(int i=0; i<length; i++){
    		int len = strlen(arr[i])+1;
    		fwrite(&len, sizeof(int), 1, file );
    		fwrite(arr[i], sizeof(char), len, file );
    	}
		//for now will assume all patches are co-planar

    	fclose(file);



    }


private:
	char** arr;
	__int64 size;
	static const  __int64 CAP =ARRLONGCAP;


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

#endif /*STRINGARRNONDUP_H_*/
