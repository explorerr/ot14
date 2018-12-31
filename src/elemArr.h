#ifndef ELEMARR_H_
#define ELEMARR_H_

#include <memory>
#include <iostream>

#include "stdlib.h"




#include "Exceptions.h"



class elemArr
{
public:
	__int64 length;

	elemArr(__int64 initSize){
		incrementSize = 100;
		length = 0;
		size = 0;
		arr = NULL;
		zId = NULL;
	}
	virtual ~elemArr(){
        if(arr != NULL)
            free(arr);
        if(zId != NULL)
            free(zId);
    };

	void setIncreMentSize(__int64 size){
		incrementSize = size;
	}
	__int64 &operator()(__int64 i, __int64 j ){

		if(i < 0 || i >= (CAP-incrementSize)){
			cerr << "\nError: Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		if(j < 0 || j >= dim ){
			cerr << "\nError: Dimension " << j << " out of range" << endl;
			throw StoppingException("\n");
		}

		if( i >= size){
            tryResize();
		}
		i > length ? length = i : length;
		return arr[i][j];
	}

	__int64 operator()(__int64 i, __int64 j ) const {
		if(i < 0 || i >= (CAP-incrementSize)){
			cerr << "\nError: Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		if(j < 0 || j >= dim ){
			cerr << "\nError: Dimension " << j << " out of range" << endl;
			throw StoppingException("\n");
		}
		return arr[i][j];
	}
	const elemArr &operator=(const elemArr &right){
		if(&right != this){
			if(size != right.size){
				delete [] arr;
				size =right.size;
				arr = (__int64(*)[dim]) realloc(arr, sizeof(__int64)*dim*size);
			}
			for(__int64 i=0; i<size; i++)
				for(__int64 j=0; j<dim; j++)
					arr[i][j] =right.arr[i][j];
		}
		return *this;
	}

	void add(__int64 ids[8], __int64 z){
		length++;
		if(length < 0 || length >= (CAP-incrementSize)){
			cerr << "\nError: Subscript " << length << " out of range" << endl;
			throw StoppingException("\n");
		}
        tryResize();
		for(__int64 i=0; i<dim; i++)
			arr[length-1][i] = ids[i];
		zId[length-1] = z;

	}
	void add(__int64 ids[8]){
		length++;
		if(length < 0 || length >= (CAP-incrementSize)){
			cerr << "\nError: Subscript " << length << " out of range" << endl;
			throw StoppingException("\n");
		}
        tryResize();
		for(__int64 i=0; i<dim; i++)
			arr[length-1][i] = ids[i];
	}
	void setZId(__int64 i, __int64 z){
		if(i < 0 || i >= length){
			cerr << "\nError: Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		tryResize();
		zId[i] = z;
	}

	__int64 * getElem(__int64 i){
		if(i < 0 || i >= length){
			cerr << "\nError: Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
        return arr[i];
    }
    __int64 getZId(__int64 i){
		if(i < 0 || i >= length){
			cerr << "\nError: Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
        return zId[i];
    }

private:
	__int64 incrementSize;
	__int64 size;
	__int64 (*arr)[8];
	static const __int64 dim =8;
	__int64 *zId;


	void tryResize() {

		if( length >= size){
			size = size + incrementSize;
			printf("In elemArr, Increasing size of the element array to: %d\n", size);
			try{
				arr = (__int64(*)[dim]) realloc(arr, sizeof(__int64)*dim*size);
				zId = (__int64*) realloc(zId, sizeof(__int64)*size);
			}
			catch (...){
				throw StoppingException("Error: Memory allocation failed in elemArr.\n");
			}
			for(__int64 i=size-incrementSize; i<size; i++){
				for(__int64 j=0; j<dim; j++)
					arr[i][j] = -1;

				zId[i]    = -1;
			}
		}
		else
			return;
	}
};

#endif /*ELEMARR_H_*/
