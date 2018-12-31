/**
 * Polygon.h
 *
 *  Created on: Mar 24, 2009
 *      Author: zrui
 */

#ifndef POLYGON_H_
#define POLYGON_H_

#include "Project.h"
#include "Face.h"
#include "geolib.h"

class Polygon {
public:
	Polygon(int numV, vector<double> &vv){
 		nv = numV;
		if(nv<2)
			throw StoppingException("Error: Polygon vertices less than two in Polygon constructor.\n");

		v 		= (double(*)[3])malloc(sizeof(double)*nv*3);

		for(int i=0; i<nv; i++)
			for(int j=0; j<3; j++)
				 v[i][j] = vv[i*3+j];

		if (nv>2){
			polyPlaneEquation(vv, nv, func);
			for(int i=0; i<3; i++) n[i] = func[i];
		}
		else {
			for(int i=0; i<3; i++)
				n[i] = v[1][i] - v[0][i];
		}
        normalize(n);

        //get the center of the Polygon
        for(int i=0; i<3; i++){
            center[i]=0;
            for(int j=0; j<nv; j++)
				center[i] += v[j][i];
            center[i] /= nv;
        }

        squareFlg = checkSquFlg();
        //get the radius of the quad, which is the longest distance between center point and all the vertices
        radius = 0;
        double dist;
        for (int i=0; i<nv; i++) {
        	dist = pointToPointDistance(v[i], center);
        	radius = (dist>radius) ? dist : radius;
        }

	}
	Polygon(int numV, double *vv){
 		nv = numV;
		if(nv<2)
			throw StoppingException("Error: Polygon vertices less than two in Polygon constructor.\n");

		v 		= (double(*)[3])malloc(sizeof(double)*nv*3);

		for(int i=0; i<nv; i++)
			for(int j=0; j<3; j++)
				 v[i][j] = vv[i*3+j];

		if (nv>2){
			polyPlaneEquation(vv, nv, func);
			for(int i=0; i<3; i++) n[i] = func[i];
		}
		else {
			for(int i=0; i<3; i++)
				n[i] = v[1][i] - v[0][i];
		}
        normalize(n);

        //get the center of the Polygon
        for(int i=0; i<3; i++){
            center[i]=0;
            for(int j=0; j<nv; j++) 
				center[i] += v[j][i];
            center[i] /= nv;
        }

        squareFlg = checkSquFlg();
        //get the radius of the quad, which is the longest distance between center point and all the vertices
        radius = 0;
        double dist;
        for (int i=0; i<nv; i++){
        	dist = pointToPointDistance(v[i], center);
        	radius = (dist>radius)? dist : radius;
        }

    }


	bool checkSquFlg(){
        //check if this Polygon is a square?
		double myEPSILON = EPSILON;
		if(nv!= 4)
            return false;
		int tmp=0, idx;
		for(int i=0; i<3; i++){
			if(fabs(n[i])<myEPSILON) tmp++;
			else idx = i;
		}
		if(tmp!=2)
			return false;

		for(int i=0; i<4; i++){
			tmp=0;
			for(int j=0; j<3; j++){
				if( fabs(v[(i+1)%4][j] - v[i][j]) < myEPSILON)tmp++;
			}
			if(tmp != 2)
				return false;
		}
		return true;

	}



	virtual ~Polygon(){
		if( v != NULL) free(v);
	};

	int nv; //number of vertices
	double (*v)[3]; // the coordinates of the vertices
	double n[3]; //normal vector of the plane, normalized
	double func[4]; //plane equation
    bool squareFlg;
    double radius;
    double center[3];
};

#endif /* POLYGON_H_ */
