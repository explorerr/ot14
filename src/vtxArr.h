#ifndef VTXARR_H_
#define VTXARR_H_
#include <memory>
#include <iostream>
const  __int64 CAP=500000000; // maximum capacity 500M
using namespace std;
#include "Exceptions.h"
#include "intArr.h"
class vtxArr
{
public:
	 __int64 length;

	vtxArr( __int64 initSize){
		if(initSize < 0){
			fprintf(stderr, "Error: not valid initial size for vtx array: %d\n", initSize);
			return;
		}
		size   = 0;
		length = 0;
		arr    = NULL;
		zone   = NULL;
//		ocId   = NULL;
		bnd    = NULL;
		incrementSize = initSize;
	}
	vtxArr(){
		size   = 0;
		length = 0;
		arr    = NULL;
		zone   = NULL;
//		ocId   = NULL;
		bnd    = NULL;
		incrementSize = 1000;
	}

	virtual ~vtxArr(){
		if(arr != NULL)
			free(arr);
		if(zone != NULL)
			free(zone);
		if(bnd != NULL)
			free(bnd);
//		if(ocId != NULL)
//			free(ocId);

	}

	void setIncrementSize( __int64 size){
        if(size<=0)
            throw StoppingException("Error: in vtxArr, increment size specified non-positive.\n");
		incrementSize = size;
	}


	double &operator()( __int64 i,  __int64 j ){

		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		if(j < 0 || j >= dim ){
			cerr << "\nError: In vtxArr Dimension " << j << " out of range" << endl;
			throw StoppingException("\n");
		}

		return arr[i][j];
	}

	double operator()( __int64 i,  __int64 j ) const {
		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		if(j < 0 || j >= dim ){
			cerr << "\nError: In vtxArr Dimension " << j << " out of range" << endl;
			throw StoppingException("\n");
		}
		return arr[i][j];
	}

	double *getCoord( __int64 i){
		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr getCoord, Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		return arr[i];
	}

	const vtxArr &operator=(const vtxArr &right){
		if(&right != this){
			if(size != right.size){
				delete [] arr;
				size =right.size;
				arr = (double(*)[dim]) realloc(arr, sizeof(double)*dim*size);
			}
			for( __int64 i=0; i<size; i++)
				for( __int64 j=0; j<dim; j++)
					arr[i][j] =right.arr[i][j];
		}
		return *this;
	}

	/**
	 * deep copy
	 */
	void add(double x[3]){

		length++;
		tryResize();

		for( __int64 i=0; i<dim; i++)
            arr[length-1][i] = x[i];
	}

	/*	__int64 getOcId( __int64 i){
		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr getOcId, Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		return ocId[i];

	}

	/*	void setOcId( __int64 i, __int64 oc){
		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr setOcId, Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		ocId[i]=oc;
	}
*/
	__int64 getZone( __int64 i){
		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr getZone, Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		return zone[i];
	}

	void setZone( __int64 i,  __int64 z){
		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr setZone, Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		zone[i]=z;
	}


	 intArr* getBnd( __int64 i){
		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr getBnd, Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		return bnd[i];
	}

	void setBnd( __int64 i,  intArr* b){
		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr setBnd, Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		intArr * bb = new intArr(1);
		bnd[i] = bb;
		for(__int64 j=0; j<b->length; j++)
			bnd[i]->add((*b)[j]);
	}




	void updateVtx(__int64 i, double x[3]){
		if(i < 0 || i >= length){
			cerr << "\nError: In vtxArr updateVtx, Subscript " << i << " out of range" << endl;
			throw StoppingException("\n");
		}
		arr[i][0] = x[0];
		arr[i][1] = x[1];
		arr[i][2] = x[2];
	}



private:
	 __int64 incrementSize;
	 __int64 size;
	double (*arr)[3]; // the first three item are the coordinates, and the last one is
                      // the zone identification that the vertex belongs to
	static const  __int64 dim=3;
	 __int64 *zone;   // -2: on boundary
	                  // -1: exterior of the domain
			          // >=0: in the domain and index of the zone
//	__int64 *ocId;
	intArr **bnd;   // this bnd array will store all the faces that the vertex belongs to
					//code for bnd:
	                // -10 -11 -12 -13 -14 -15 is reserved for the six surface for domain
	                // -1: vtx not on the boundary
	                //>=0: vtx on the boundary, bnd id of the boundary
	                //        bnd id should be non-negative


	void tryResize() {
		if(length>= size){
			size = size + incrementSize;
			try{
				arr =  (double(*)[dim]) realloc(arr, sizeof(double)*dim*size);
				zone = (__int64*)realloc(zone, sizeof(__int64)*size);
//				ocId = (__int64*)realloc(ocId, sizeof(__int64)*size);
				bnd =  (intArr** )realloc(bnd, sizeof(intArr*)*size);
			}
			catch (...){
				throw StoppingException("Error: Memory allocation failed in vtxArr.\n");
			}
			for( __int64 i=size-incrementSize; i<size; i++){
				for( __int64 j=0; j<dim; j++)
					arr[i][j] = -1;
					zone[i] = -2;
//					ocId[i] = -1;
					bnd[i] = NULL;

			}
		}
	}

};

#endif /*VTXARR_H_*/
