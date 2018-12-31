/*
 * geoNodeArr.h
 *
 *  Created on: Jul 2, 2010
 *      Author: zrui
 */

#ifndef GEOPLANEARR_H_
#define GEOPLANEARR_H_

#include "math.h"

class geoPlaneArr {
public:
	int length;

	geoPlaneArr(){
		incrementSize = 100;
		length = 0;
		size = 0;
		arr = NULL;
	}
	geoPlaneArr(int initSize){
		incrementSize = initSize;
		length = 0;
		size = 0;
		arr = NULL;
	}


	virtual ~geoPlaneArr(){
        if(arr != NULL)
            free(arr);
    };

	void setIncreMentSize(int size){
		incrementSize = size;
	}

	/**
	 * This is the assignment operator for this class
	 * This function will not support "out of range" assignment
	 * The index must be within the length of the array
	 */

	double &operator()(int i, int j ){

		if(i < 0 || i >= length){
			printf("\nError in geoNodeArr assignment operator: first subscript %d out of range %d", i, length);
			throw StoppingException("\n");
		}
		if(j < 0 || j >= dim ){
			printf("\nError in geoNodeArr assignment operator: second subscript %d out of range %d", j, dim);
			throw StoppingException("\n");
		}

		return arr[i][j];
	}

	double operator()(int i, int j ) const {
		if(i < 0 || i >= length){
			printf("\nError in geoNodeArr get operator: first subscript %d out of range %d", i, length);
			throw StoppingException("\n");
		}
		if(j < 0 || j >= dim ){
			printf("\nError in geoNodeArr get operator: second subscript %d out of range %d", j, dim);
			throw StoppingException("\n");
		}
		return arr[i][j];
	}


	const geoPlaneArr &operator=(const geoPlaneArr &right){
		if(&right != this){
			if(size != right.size){
		        if(arr != NULL)
		            free(arr);
				length =right.size;
				tryResize();
			}
			for(int i=0; i<size; i++)
				for(int j=0; j<dim; j++)
					arr[i][j] =right.arr[i][j];
		}
		return *this;
	}



	void add(double ids[3]){
		length++;
        tryResize();
		for(int i=0; i<dim; i++)
			arr[length-1][i] = ids[i];

	}

	int addNonDup(double ids[3]){

		double myEPSILON = EPSILON;

		int id= -1;
		double sum = 0;
		for(int i=0; i<length; i++){
			sum = (arr[i][0] - ids[0]) + (arr[i][1] - ids[1]) + (arr[i][2] - ids[2]);
			if( fabs(sum) < myEPSILON ){
				id = i;
				break;
			}
		}
		if(id == -1){
			id = length++;
			tryResize();
			arr[length-1][0] = ids[0];
			arr[length-1][1] = ids[1];
			arr[length-1][2] = ids[2];
		}
		return id;
	}



	double * get(int i){

		if(i < 0 || i >= length){
			printf("\nError in geoNodeArr getNode: Subscript %d out of range %d", i, length);
			throw StoppingException("\n");
		}
        return arr[i];
    }




private:
	int incrementSize;
	int size;
	double (*arr)[4];
	static const int dim =4;
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
		if(length>CAP){
			printf("Error in faceArr: array length larger than allowed capacity: %d > %d", length, CAP);
			throw StoppingException("\n");
		}

		while(length >= size){
			size = size + incrementSize;
			try{
				arr = (double(*)[dim]) realloc(arr, sizeof(double)*dim*size);
			}
			catch (...){
				throw StoppingException("Error in faceArr: Memory allocation failed in elemArr.\n");
			}

			for(int i=size-incrementSize; i<size; i++){
				for(int j=0; j<dim; j++)
					arr[i][j] = -1;
			}
		}
	}

};
#endif /* GEOPLANEARR_H_ */
