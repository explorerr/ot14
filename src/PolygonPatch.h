/**
 * PolygonPatch.h
 *
 *  Created on: Aug 27, 2010
 *      Author: zrui
 *
 */

#ifndef POLYGONPATCH_H_
#define POLYGONPATCH_H_

#include "Project.h"
#include "Face.h"
#include "geolib.h"
#include "Polygon.h"

class PolygonPatch {
public:
	PolygonPatch(int numPoly, int bo, Polygon **polys){
	
//		if(numPoly<2) 
//			throw StoppingException("Error in PolgonPatch constructor: Polygon patch has less than 2 polygons.\n");
		
		np = numPoly;

		if( bo != 1 && bo != -1)
			throw StoppingException("Error: Polygon patch boolean operation not known.\n");

		this->bo = bo;

	
		if(!polys)
			throw StoppingException("Error in PolgonPatch constructor: Polygon pointers is NULL.\n");
		
		this->polys = polys;
    }




	virtual ~PolygonPatch(){
		if( polys == NULL) 
			return;
		for(int i=0; i<np; i++)
			delete(polys);
	
		free(polys);
	};

	int np; //number of polygons
	int bo; //boolean operation on the polygons
	Polygon** polys;
	double radius;
    double center[3];
	

};


#endif /* POLYGONPATCH_H_ */
