/*
 *  Patch.h
 *
 *  Created on: Oct 5 2010
 *      Author: Rui Zhang
 */

#ifndef PATCH_H_
#define PATCH_H_

#include "Project.h"
#include "Exceptions.h"
#include "intArrSorted.h"
#include "Polygon.h"


class Patch{

public:




	Patch() : fc(), nd(){
	
		


	};
	virtual ~Patch(){
		
		
	};

	void addFace (int fId, vector<int> nId, int numNd){
		
		
		fc.insert(fId);
		for(int i=0; i<numNd; i++)
			nd.insert(nId[i]);

		
	};
	


	void setPlaneEqua(double func[4]){
		plane[0] = func[0];
		plane[1] = func[1];
		plane[2] = func[2];
		plane[3] = func[3];
	}


	void addToFC(int f){
		fc.insert(f);

	}

	void addToND(int n){

		nd.insert(n);
	}


	//id is the id of query in array of map not node id or face id
	intArrSorted* getFaceList(){

		return &fc;

	}

	intArrSorted* getNodeList(){

		return &nd;

	}

	void setBnd(int bnd){
		this->bnd = bnd;
	}

	int getBnd(){
		return bnd;
	}

	void setZDefFace(int tag){
		this->zDefFace = tag;
	}

	int getZDefFace(){
		return zDefFace;
	}

	bool attchedToPatchCheck(Face* fc, vector<int> n, int numN){
		
		if(fc->bnd != bnd)
			return false;

		if(fc->zDefFace != zDefFace )
			return false;

		// this checking should make sure

        double myEPSILON = EPSILON;
        Polygon* pp = (Polygon*) fc->f;
        double sum[2] = {0, 0};

        for(int i=0; i<3; i++){
            sum[0] += fabs(pp->func[i]) - fabs(plane[i]);
        }
        // check if the face is facing the opposite direction
 //       for(int i=0; i<3; i++){
 //           sum[1] += fabs(pp->func[i] + plane[i]);
 //       }

 //       if( sum[0] > myEPSILON && sum[1] > myEPSILON)
		if( sum[0] > myEPSILON )
        	return false;
        if( sum[0] < myEPSILON ){
        	sum[0] += fabs(pp->func[3]) - fabs(plane[3]);
        	if( sum[0] > myEPSILON )
        		return false;
        }
        else if( sum [1] < myEPSILON ){
        	sum[1] += fabs( pp->func[3]) - fabs(plane[3] );
        	if( sum[1] > myEPSILON )
        		return false;
        }
        for( int i=0; i<numN; i++ ){
            if( nd.has(n[i]) )
                return true;           
       }
        return false;


	};


private:
	

	
 

	intArrSorted fc;
	intArrSorted nd;
	int bnd;
	int zDefFace;
	double plane[4];






};

#endif /* PATCH_H_ */
