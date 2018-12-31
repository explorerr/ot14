/*
 * geoNodeArr.h
 *
 *  Created on: Jul 2, 2010
 *      Author: zrui
 */

#ifndef GEONODEARR_H_
#define GEONODEARR_H_

#include "math.h"

class geoNodeArr {
public:
	int length;

	geoNodeArr(){
		incrementSize = 10;
		length = 0;
		size = 0;
		arr = NULL;
		z = NULL;
	}
	geoNodeArr(int initSize){
		incrementSize = initSize;
		length = 0;
		size = 0;
		arr = NULL;
		z = NULL;
	}


	virtual ~geoNodeArr(){
        if(arr != NULL)
            free(arr);

        if(z)
        	free(z);
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


	const geoNodeArr &operator=(const geoNodeArr &right){
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

	void deleteNode(int id){
		if(id>length-1){
			printf("Error in geoNodeArr deleteNode, id larger than total number of nodes in the array, [%d] \n", id);
			throw StoppingException("\n");
		}
		length --;
		for(int i=id; i<length; i++){
			arr[i][0]= arr[i+1][0];
			arr[i][1]= arr[i+1][1];
			arr[i][2]= arr[i+1][2];
		}


	}

	int addNonDup(double ids[3]){

		double myEPSILON = 0.0001;

		int id= -1;
		double sum = 0;
		for(int i=0; i<length; i++){
			sum = fabs(arr[i][0] - ids[0]) + fabs(arr[i][1] - ids[1]) + fabs(arr[i][2] - ids[2]);
			if( sum < myEPSILON ){
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



	double * getNode(int i){

		if(i < 0 || i >= length){
			printf("\nError in geoNodeArr getNode: Subscript %d out of range %d", i, length);
			throw StoppingException("\n");
		}
        return arr[i];
    }

	bool featureNodeCheck(int i){
		if(i < 0 || i >= length){
			printf("\nError in geoNodeArr getNode: Subscript %d out of range %d", i, length);
			throw StoppingException("\n");
		}

       	return z[i];

	}
	void add(double ids[3], bool z){

		add(ids);
		this->z[length-1] = z;
	}

	int addNonDup(double ids[3], bool z){
		int i = addNonDup(ids);
		this->z[i] = z;
		return i;
	}


    void print(){
//    	cout<<"nodes in the geoModel are: "<<endl;
        char* fullFilePath = NULL;
        char* fName = "node";
        getFullFilePath(&fullFilePath, fName, ".info");
        FILE* file = fopen(fullFilePath, "w");


    	for(int i=0; i<length; i++){
            fprintf(file, "Node [%d]: (%f, %f, %f)\n", i, arr[i][0], arr[i][1], arr[i][2]);
    	}

		fclose(file);
   	}

	/**
	  * This function will update the given node id in the array
	  *
	  * Deep copy of the data
	  */
	void updateNode(int i, double *nn){

		if(i<0 || i>length){
			printf("\nError in geoNodeArr updateNode: Subscript %d out of range %d", i, length);
			throw StoppingException("\n");
		}
		if( !nn ){
			printf("\nError in geoNodeArr updateNode: new position pointer is NULL.");
			throw StoppingException("\n");
		}
		arr[i][0] = nn[0];
		arr[i][1] = nn[1];
		arr[i][2] = nn[2];
	}







private:
	int incrementSize;
	int size;
	double (*arr)[3];
	static const int dim =3;
	static const int CAP =ARRLONGCAP;
	bool *z;  //default set as true


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
				z   = (bool*)realloc(z, sizeof(bool)*size);
			}
			catch (...){
				throw StoppingException("Error in faceArr: Memory allocation failed in elemArr.\n");
			}

			for(int i=size-incrementSize; i<size; i++){
				for(int j=0; j<dim; j++)
					arr[i][j] = -1;
				z[i] = true;
			}
		}
	}

};
#endif /* GEONODEARR_H_ */
