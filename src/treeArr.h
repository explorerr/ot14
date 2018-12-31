/*
 * treeArr.h
 *
 *  Created on: Apr 18, 2011
 *      Author: zrui
 */

#ifndef TREEARR_H_
#define TREEARR_H_

#include "intArr.h"
#include "Exceptions.h"
#include "OcTree.h"




class treeArr {
    

public:


    treeArr(){

    };
	
	treeArr(OcTree *ot){

        this->ot = ot;

        incrementSize = 1000;
        size = 0;
        numArr = 0;

        length = ot->numCellAtLvl[ot->strLvl];
        tryResize();

	};

	treeArr(OcTree* ot, __int64 initSize){

		this->ot = ot;
        incrementSize = initSize;
        size = 0;
        numArr = 0;

        length = ot->numCellAtLvl[ot->strLvl];
        tryResize();


	};

	virtual ~treeArr(){


		for(int i=0; i<numArr; i++)
			free(arr[i]);
		free(arr);


	};

	__int64 length;
	__int64 incrementSize;




	void setIncreMentSize(__int64 size){
		incrementSize = size;
	};


    /** this function will add the root to the tree structure
      * and then create slots for the cld, and return the first cld position
      * other child will be following the first cld
      */
	__int64 createChild(__int64 root){

        
		length += 8;
		tryResize();
        int curId, arrId, inArrId;
        
        curId = root;
        arrId = curId/incrementSize;
        inArrId = curId - arrId*incrementSize;
        arr[arrId][inArrId].r = length-8;

        for(int i=0; i<8; i++){
        
            curId = length -8 +i;
            arrId = curId/incrementSize;
            inArrId = curId - arrId*incrementSize;

            arr[arrId][inArrId].r = -1;
            arr[arrId][inArrId].cld = -1;

        }

        return length-8;

	};



    __int64 getChild(__int64 i, __int64 cldId){

        if(i>=length){
            printf("Error in treeArr getNode, id out of range: %lld, excepting(0, %lld).", 
                    i, length);
            throw StoppingException("\n");
        
        }

        int curId, arrId, inArrId;
        
        curId = i;
        arrId = curId/incrementSize;
        inArrId = curId - arrId*incrementSize;

        int firstChild = arr[arrId][inArrId].cld ;
        if( firstChild == -1)
            return -1;
        else
            return firstChild + cldId;



    };


    void setCellId( __int64 i, __int64 cellId ) {

        if(i>=length){
            printf("Error in treeArr getNode, id out of range: %lld, excepting(0, %lld).", 
                    i, length);
            throw StoppingException("\n");
        
        }


        int curId, arrId, inArrId;
        
        curId = i;
        arrId = curId/incrementSize;
        inArrId = curId - arrId*incrementSize;

        arr[arrId][inArrId].r= cellId;
    };

       
    __int64  getCellId( __int64 i ) {

        if(i>=length){
            printf("Error in treeArr getNode, id out of range: %lld, excepting(0, %lld).", 
                    i, length);
            throw StoppingException("\n");
        
        }


        int curId, arrId, inArrId;
        
        curId = i;
        arrId = curId/incrementSize;
        inArrId = curId - arrId*incrementSize;

        return arr[arrId][inArrId].r;
    };

    






private:

		
	typedef struct treeNode{
	    __int64 r;      //cell id of the root of the tree
	    __int64 cld;    //its child
    }treeNode;
 
    OcTree * ot;
	treeNode **arr;
	int numArr;

	__int64 size;
	static const __int64 CAP =500000000;

	void tryResize() {

		while(length >= size){
			size = size + incrementSize;
			numArr++;
			arr = (treeNode**)realloc(arr, sizeof(treeNode*)*numArr);
			arr[numArr-1] = (treeNode*)malloc(sizeof(treeNode)*incrementSize);
		
            if(!arr[numArr-1])
				cout<<"Memory allocation failed in treeArr."<<endl;

            memset( arr[numArr-1], -1, sizeof(treeNode)*incrementSize );
			
		}
	};


};

#endif /* TREEARR_H_ */
