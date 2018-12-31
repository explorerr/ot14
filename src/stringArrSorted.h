#ifndef STRINGARRSORTED_H_
#define STRINGARRSORTED_H_
#include <memory>
#include <iostream>
#include "Project.h"
#include "math.h"
class stringArrSorted
{
public:

	__int64 length;
	__int64 incrementSize;

	stringArrSorted(){
		arr = NULL;
		incrementSize = 10;
		length = 0;
		size = 0;
	}

	stringArrSorted( __int64 initSize){
		arr = NULL;
		incrementSize = initSize;
		length = 0;
		size = 0;
	};
	virtual ~stringArrSorted(){
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
			printf("\nError in stringArrSorted assignment operator: subscript %lld out of range %lld\n", subscript, length);
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
			printf("\nError in stringArrSorted get operator: subscript %lld out of range %lld\n", subscript, length);
			throw StoppingException("\n");
		}
		return arr[subscript];
	};

	const stringArrSorted &operator=(const stringArrSorted &right){
		if(&right != this){
			if(size != right.size){
				free(arr);
				size =right.size;
				arr = (char**) malloc(sizeof(char*)*size);
			}
			for( __int64 i=0; i<size; i++)
				insert(right.arr[i]);
		}
		return *this;
	};

	 __int64 insert( char* string){

		if(length==0){
			length ++;
			tryResize(); //make sure there will be enough space
			copyString(string, 0);
			return 0;
		}
		float id = find(string);
		double pos;
		double frac = modf(id, &pos);
		if(frac==0) // already in the array
			return (__int64)pos;
		if(frac>0)
			pos++;
		else
			pos=0;

		length++;
		tryResize();
		for( __int64 i=length-1; i>pos; i--)
			arr[i] = arr[i-1];
		copyString(string, (__int64)pos);
		return (__int64)(pos);
		
	}

	void copyString(char* string, __int64 subscript){
		if(subscript <0 || subscript >=length){
			printf("\nError in stringArrSorted get operator: subscript %lld out of range %lld\n", subscript, length);
			throw StoppingException("\n");
		}
		arr[subscript] = (char*)malloc( sizeof(char)*(strlen(string)+1) );
		if(!arr[subscript]){
			printf("\nError in stringArrSorted memory allocation failed.\n");
			throw StoppingException("\n");
		}
		strcpy(arr[subscript], string);


	}

	float find( char* string){
	
		if(length==0)
			return -1;
		__int64 idx = length/2;
		float find;


		if( strcmp( string, arr[length-1]) > 0 )
			return length-0.5;
		else if ( strcmp(string, arr[0]) < 0 )
			return -0.5;
		int ret = strcmp(string, arr[idx]);
		if( ret > 0){
		    return find = findHelper(idx, length, string);
		}
		else if( ret < 0 ) {
			return find = findHelper(0, idx, string);
		}
		else {
			return idx;
		}
	}

    bool has( char* string){

        float f = find(string);
        if(f==-1)
        	return false;
        f *= 2;
        double myEPSILON = EPSILON;
        if ( fabs(fmod(f,2))<myEPSILON ) // in the array
            return true;
        else 
            return false;

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

    	for(int i=0; i<length; i++){
    		int len = strlen(arr[i]);
    		fwrite(&len, sizeof(int), 1, file );
    		fwrite(arr[i], sizeof(char), len, file );
    	}




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


		float findHelper( __int64 low,  __int64 high,  char* string){

		if ( low >= high )
			return (low+0.5);
		int ret;
		if( high-low == 1){
			ret = strcmp(string, arr[low]);
			if( ret == 0 )
				return low;
			ret = strcmp(string, arr[high]);
			if( ret == 0 )
				return high;
			return low+0.5;
		}

		 __int64 mid = ( low + high ) / 2;
		

		ret = strcmp(string, arr[mid]);
		if( ret == 0 )
			return mid;

		if( ret > 0)
			low = mid;
		else if ( ret < 0 )
			high = mid;
		
		return findHelper(low, high, string);


	};


};

#endif /*STRINGARRSORTED_H_*/
