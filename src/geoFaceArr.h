/*
 * geoFaceArr.h
 *
 *  Created on: Jul 2, 2010
 *      Author: zrui
 */

#ifndef GEOFACEARR_H_
#define GEOFACEARR_H_


#include <memory>
#include <iostream>
#include "Face.h"
#include "Polygon.h"
#include "PolygonPatch.h"
#include "stringArrSorted.h"
using namespace std;

#include "Exceptions.h"



class geoFaceArr {

public:
	int length;

	geoFaceArr(){
		incrementSize = 100;
		length = 0;
		size = 0;
		arr = NULL;

	}

	geoFaceArr(int initSize){
		incrementSize = initSize;
		length = 0;
		size = 0;
		arr = NULL;

	}


	virtual ~geoFaceArr(){
        if(arr != NULL){
        	for(int i=0; i<length; i++)
        		if(arr[i])
        			free(arr[i]);
        	free(arr);
        }
   //     delete(bndStrLookUp);
	};

	void setIncreMentSize(int size){
		incrementSize = size;
	}

	Face* &operator[](int i ){

		if(i < 0 || i >= length){
			cerr << "\nError in geoFaceArr: Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		return arr[i];
	}

	Face* operator[](int i ) const {
		if(i < 0 || i >= length){
			cerr << "\nError in geoFaceArr: Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		return arr[i];
	}

	const geoFaceArr &operator=(const geoFaceArr &right){
		if(&right != this){
			if(size != right.size){
		        if(arr != NULL){
		        	for(int i=0; i<length; i++)
		        		if(arr[i])
		        			free(arr[i]);
		        	free(arr);
		        }
				length =right.size;
				tryResize();
			}
			for(int i=0; i<size; i++)
				arr[i] =right.arr[i];
		}
		return *this;
	}

	void add(Face* data){
		length++;
        tryResize();
		arr[length-1] = data;


	}

    Face * getFace(int i){

		if(i < 0 || i >= length){
			printf("\nError in geoFaceArr: Subscript %d out of range %d", i, length);
			throw StoppingException("\n");
		}
        return arr[i];
    }

    
	void print(){

        char* fullFilePath = NULL;
        char* fName = "face";
        getFullFilePath(&fullFilePath, fName, ".info");
        FILE* file = fopen(fullFilePath, "w");
        double scale = 39.3700787;
		scale = 1;

    	for(int i=0; i<length; i++){
            fprintf(file, "Face [%d]: \n", i);
    		if( arr[i]->type > 2 && arr[i]->type <6 ){
    			Polygon* pp = (Polygon*) arr[i]->f;
				for(int j=0; j<pp->nv; j++) {
						//    			cout<<"\t"<<pp->v[j][0]<<"\t"<<pp->v[j][1]<<"\t"<<pp->v[j][2]<<endl;
					fprintf(file, "\t\t%f \t %f \t %f \n", pp->v[j][0]*scale,
																pp->v[j][1]*scale,
																	pp->v[j][2]*scale);
					
				}
			//	fprintf(file,"norm: (%f, %f, %f) \n", pp->n[0], pp->n[1], pp->n[2]);
    		}
    		else if (arr[i]->type == 10){
    			PolygonPatch * pp = (PolygonPatch*) arr[i]->f;
    			fprintf(file, "\tboolean operation: %d\n", pp->bo);
    			for(int k=0; k<pp->np; k++){
        			fprintf(file, "\tpolys[%d]:\n", k);

    				for(int j=0; j<pp->polys[k]->nv; j++) {
    						//    			cout<<"\t"<<pp->v[j][0]<<"\t"<<pp->v[j][1]<<"\t"<<pp->v[j][2]<<endl;
    					fprintf(file, "\t\t%f \t %f \t %f \n", pp->polys[k]->v[j][0]*scale,
    							                               	   pp->polys[k]->v[j][1]*scale,
    							                               	   	   pp->polys[k]->v[j][2]*scale);
						
    				}
				//	fprintf(file,"norm: (%f, %f, %f) \n", pp->polys[k]->n[0], pp->polys[k]->n[1], pp->polys[k]->n[2]);

    			}
   			}
    	}

		fclose(file);
    }

	void printNormal(){

        char* fullFilePath = NULL;
        char* fName = "faceNormal";
        getFullFilePath(&fullFilePath, fName, ".info");
        FILE* file = fopen(fullFilePath, "wb");
		double norm[3];
    	for(int i=0; i<length; i++){
        
    		if( arr[i]->type > 2 && arr[i]->type <6 ){

				Polygon* pp;
				pp = (Polygon*) arr[i]->f;

				fwrite (norm , sizeof(double) , 3 , file );
					
			}
    		
    		else if (arr[i]->type == 10){
    			PolygonPatch * pp = (PolygonPatch*) arr[i]->f;
    			fprintf(file, "\tboolean operation: %d\n", pp->bo);
    			for(int k=0; k<pp->np; k++){
        			fprintf(file, "\tpolys[%d]:\n", k);

    				for(int j=0; j<pp->polys[k]->nv; j++) {
 						
    				}
    			}
   			}
    	}

		fclose(file);

	}



private:
	int incrementSize;
	int size;
	Face **arr; //an array of pointer to the Face objects

	stringArrSorted* bndStrLookUp;


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
			printf("Error in geoFaceArr: array length larger than allowed capacity: %d > %lld", length, CAP);
			throw StoppingException("\n");
		}

		while(length >= size){
			size = size + incrementSize;
			try{
				arr = (Face**)realloc(arr, sizeof(Face*)*size);
			}
			catch (...){
				throw StoppingException("Error in geoFaceArr: Memory allocation failed in elemArr.\n");
			}

			for(int i=size-incrementSize; i<size; i++)
					arr[i] = NULL;
		}
	}

};

#endif /* GEOFACEARR_H_ */
