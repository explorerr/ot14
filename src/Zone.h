/* *
 * Zone.h
 *
 *  Created on: Mar 30, 2009
 *      Author: zrui
 */

#ifndef ZONE_H_
#define ZONE_H_

#include "Face.h"


class Zone {
public:
	Face** face;
	 __int64 faceCnt;

	Zone(){
		faceCnt = 0;
		face = NULL;
	}
	virtual ~Zone(){
		if( face!= NULL) free(face);
	}
	void addFace(Face* f){//shallow copy
		faceCnt++;
		face = (Face**)realloc(face, faceCnt*sizeof(Face*));
		face[faceCnt-1] = f;
	}

    /**
     *  This function will return the zone id of the given point
     *  return value:
     *  1         in this zone, caller function will konw the zone id;
     *  0         outside the zone, caller function will know the zone id;
     *  (  , -10) on the zone boudary, and "bndId" of the oundary will be returned;
     *
     */ 

	 __int64 getZoneOrBndId(double x[3]){
		double dire[3];
		 __int64 nCrossPnt=0, tmp;
		dire[0]=1; dire[1] = 0; dire[2]=0;
        for( __int64 i=0; i< faceCnt; i++){
        	tmp = (face[i])->getNumIntersectPntWithRay(x, dire);
        	if(tmp==-1)
        		return -(face[i]->bnd)-10;
        	else nCrossPnt += tmp;
        }
        if(nCrossPnt%2 ==1) return 1;
        else return 0;
	}




};

#endif /* ZONE_H_ */
