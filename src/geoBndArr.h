/*
 * geoBndArr.h
 *
 *  Created on: Jul 6, 2010
 *      Author: zrui
 */

#ifndef GEOBNDARR_H_
#define GEOBNDARR_H_

class geoBndArr {
public:
	__int64 length;

	geoBndArr(){
		incrementSize = 100;
		length = 0;
		size = 0;
		arr = NULL;
	}
	geoBndArr(__int64 initSize){
		incrementSize = initSize;
		arr = NULL;
		initializedLength = initSize;
		length = initSize;
		size = 0;
		tryResize();
		length = 0;
	}


	virtual ~geoBndArr(){
        if(arr != NULL)
            free(arr);
    };

	void setIncreMentSize(__int64 size){
		incrementSize = size;
	}

	/**
	 * This is the assignment operator for this class
	 * This function will not support "out of range" assignment
	 * The index must be within the length of the array
	 */

	__int64 &operator()(__int64 i, __int64 j ){

		if(i < 0 || i >= initializedLength){
			printf("\nError in geoNodeArr assignment operator: first subscript %lld out of initialized range %lld", i, length);
			throw StoppingException("\n");
		}
		if(j < 0 || j >= dim ){
			printf("\nError in geoNodeArr assignment operator: second subscript %lld out of range %lld", j, dim);
			throw StoppingException("\n");
		}

		return arr[i][j];
	}

	__int64 operator()(__int64 i, __int64 j ) const {
		if(i < 0 || i >= length){
			printf("\nError in geoNodeArr get operator: first subscript %lld out of range %lld", i, length);
			throw StoppingException("\n");
		}
		if(j < 0 || j >= dim ){
			printf("\nError in geoNodeArr get operator: second subscript %lld out of range %lld", j, dim);
			throw StoppingException("\n");
		}
		return arr[i][j];
	}


	const geoBndArr &operator=(const geoBndArr &right){
		if(&right != this){
			if(size != right.size){
		        if(arr != NULL)
		            free(arr);
				length =right.size;
				tryResize();
			}
			for(__int64 i=0; i<size; i++)
				for(__int64 j=0; j<dim; j++)
					arr[i][j] =right.arr[i][j];
		}
		return *this;
	}



	void add(__int64 ids[3]){
		length++;
        tryResize();
		for(__int64 i=0; i<dim; i++)
			arr[length-1][i] = ids[i];

	}



	__int64 getBnd(__int64 i){

		if(i < 0 || i >= length){
			printf("\nError in geoNodeArr: Subscript %lld out of range %lld", i, length);
			throw StoppingException("\n");
		}
        return arr[i][0];
    }

	__int64 *getBndZone(__int64 i){
		if(i < 0 || i >= length){
			printf("\nError in geoNodeArr: Subscript %lld out of range %lld", i, length);
			throw StoppingException("\n");
		}
        return (__int64*) arr[i]+sizeof(__int64);

	}


	void initializeWithDefault(__int64 length){
		if(length < 0 || length > CAP){
			printf("\nError in geoNodeArr initializeWithDefault: length  %lld out of capacity %lld", length, CAP);
			throw StoppingException("\n");
		}
		this->length=length;
		tryResize();
		this->length = 0;
		initializedLength = length;


	}

private:
	__int64 incrementSize;
	__int64 size;
	__int64 (*arr)[3]; //[0]=bnd, [1]=zone+, [2]=zone-
	static const __int64 dim =3;
	static const __int64 CAP =ARRLONGCAP;
	__int64 initializedLength;


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
		if(length>CAP){
			printf("Error in faceArr: array length larger than allowed capacity: %lld > %lld", length, CAP);
			throw StoppingException("\n");
		}

		while(length >= size){
			size = size + incrementSize;
			try{
				arr = (__int64(*)[dim]) realloc(arr, sizeof(__int64)*dim*size);
			}
			catch (...){
				throw StoppingException("Error in faceArr: Memory allocation failed in elemArr.\n");
			}

			for(__int64 i=size-incrementSize; i<size; i++)
				for(__int64 j=0; j<dim; j++)
					arr[i][j] = -1;
		}
	}

};
#endif /* GEOBNDARR_H_ */
