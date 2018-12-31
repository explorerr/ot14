/*
 * ocCell.h
 *
 *  Created on: May 31, 2010
 *      Author: zrui
 */

#ifndef OCCELLARR_H_
#define OCCELLARR_H_

#include "intArr.h"
#include "Exceptions.h"

class ocCellArr {
public:


   	typedef struct {
		
		intArr 				*fc;
		intArr 				*nd;
        intArr				*bnd;
	 	__int64 			vtx;
//		__int64				coll;
		__int64				zone;
        __int64 			ocId;

	} ocCell;


	ocCellArr(){
		arr = NULL;
		incrementSize = 1000;
		length = 0;
		size = 0;
		numArr = 0;

	}
	ocCellArr(__int64 initSize){
		arr = NULL;
		incrementSize = initSize;
		length = 0;
		size = 0;
		numArr = 0;

	}

	virtual ~ocCellArr(){


		for(int i=0; i<numArr; i++){
			if(arr[i]->bnd)
				free(arr[i]->bnd);
			if(arr[i]->fc)
				free(arr[i]->fc);
			if(arr[i]->nd)
				free(arr[i]->nd);
			free(arr[i]);

		}
		free(arr);


	}

	__int64 length;
	__int64 incrementSize;




	void setIncreMentSize(__int64 size){
		incrementSize = size;
	};


	void add(intArr *fc, intArr *nd, __int64 z, __int64 v, __int64 coll, intArr* b){

		length ++;
		tryResize();
		int curId = length-1;
		int arrId = curId/incrementSize;
		int inArrId = curId - arrId*incrementSize;
		arr[arrId][inArrId].fc    = fc;
		arr[arrId][inArrId].nd    = nd;
		arr[arrId][inArrId].zone  = z;
		arr[arrId][inArrId].bnd   = b;
		arr[arrId][inArrId].vtx   = v;
//		arr[arrId][inArrId].coll  = coll;
		arr[arrId][inArrId].ocId = 0;
	}

	void add(){
		length ++;
//		cout<<"adding cell: "<<length<<endl;
		tryResize();
		int curId = length-1;
		int arrId = curId/incrementSize;
		int inArrId = curId - arrId*incrementSize;
		arr[arrId][inArrId].fc = NULL;
		arr[arrId][inArrId].nd = NULL;
		arr[arrId][inArrId].zone  = -2;
		arr[arrId][inArrId].bnd = NULL;
		arr[arrId][inArrId].vtx  = -1;
//		arr[arrId][inArrId].coll = -1;
		arr[arrId][inArrId].ocId = 0;
	}

	//shallow return
    ocCell * getCell(__int64 i){
		if(i<0 || i>=length){
			printf("Error: in getCell in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;

        return &arr[arrId][inArrId];

    }



	intArr* getFc(__int64 i){
		if(i<0 || i>=length){
			printf("Error: in getFc in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;

		return arr[arrId][inArrId].fc;
	}

	intArr* getNd(__int64 i){
		if(i<0 || i>=length){
			printf("Error: in getNd in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;
		return arr[arrId][inArrId].nd;
	}

	__int64 getZone(__int64 i){
		if(i<0 || i>=length){
			printf("Error: in getZone in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;
		return arr[arrId][inArrId].zone;
	}

   	intArr* getBnd(__int64 i){
		if(i<0 || i>=length){
			printf("Error: in getBnd in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;
		return arr[arrId][inArrId].bnd;
	}

	__int64 getVtx(__int64 i){
		if(i<0 || i>=length){
			printf("Error: in getVtx in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;
		return arr[arrId][inArrId].vtx;
	}

/*	__int64 getColl(__int64 i){
		if(i<0 || i>=length){
			printf("Error: in getColl in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		return arr[i].coll;
	}
*/
	void setFc(__int64 i, intArr *fc){
		if(i<0 || i>=length){
			printf("Error: in getFc in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;
		if(arr[arrId][inArrId].fc){
			delete(arr[arrId][inArrId].fc);
		}
		arr[arrId][inArrId].fc = fc;
	}

	void setNd(__int64 i, intArr *nd){
		if(i<0 || i>=length){
			printf("Error: in setNd in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;
		if(arr[arrId][inArrId].nd)
			delete(arr[arrId][inArrId].nd);
		arr[arrId][inArrId].nd = nd;
	}

	void setZone(__int64 i, __int64 z){
		if(i<0 || i>=length){
			printf("Error: in setZone in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;

		arr[arrId][inArrId].zone = z;
	}

	void setBnd(__int64 i, intArr* b){
		if(i<0 || i>=length){
			printf("Error: in setBnd in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;
		intArr * bb = new intArr(1);
		arr[arrId][inArrId].bnd = bb;
		for(__int64 j=0; j<b->length; j++)
			arr[arrId][inArrId].bnd->add((*b)[j]);

	}


    void setVtx(__int64 i, __int64 v){
		if(i<0 || i>=length){
			printf("Error: in setVtx in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;
		arr[arrId][inArrId].vtx = v;
	}

/*	void setColl(__int64 i, __int64 c){
		if(i<0 || i>=length){
			printf("Error: in setColl in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}

		arr[i].coll = c;
	}*/
    void setOcId(__int64 i, __int64 o){
		if(i<0 || i>=length){
			printf("Error: in setOcId in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;

		arr[arrId][inArrId].ocId = o;

		if(o==641463173)
			printf("KKKKKKKKKK");
	}

    __int64  getOcId(__int64 i){
		if(i<0 || i>=length){
			printf("Error: in getOcId in OcCell, i=%lld out of range, expected [0, %lld].\n",
					                   i, length-1);
			throw StoppingException("\n");
		}
		int arrId = i/incrementSize;
		int inArrId = i - arrId*incrementSize;
		return arr[arrId][inArrId].ocId;
	}

	void toString(){

	}


private:


	ocCell **arr;

	int numArr;

	__int64 size;
	static const __int64 CAP =500000000;

	void tryResize() {

		if(length >= size){
			size = size + incrementSize;
			numArr++;
			arr = (ocCell**)realloc(arr, sizeof(ocCell*)*numArr);
			arr[numArr-1] = (ocCell*)malloc(sizeof(ocCell)*incrementSize);
			if(!arr[numArr-1])
				cout<<"Memory allocation failed in ocCellArr."<<endl;
			for(__int64 i=0; i<incrementSize; i++){
				arr[numArr-1][i].fc = NULL;
				arr[numArr-1][i].nd = NULL;
				arr[numArr-1][i].zone  = -2;
				arr[numArr-1][i].bnd = NULL;
				arr[numArr-1][i].vtx  = -1;
	//			arr[i].coll = -1;
				arr[numArr-1][i].ocId = 0;
			}
		}
	};


};

#endif /* OCCELLARR_H_ */
