/*
 * Face.h
 *
 * This class is the super class for all the different types of face
 * This class will provide the functions that is not face type specific,
 * i.e. distance to between face and face, face and points
 *
 *  Created on: Mar 24, 2009
 *      Author: zrui
 */

#ifndef FACE_H_
#define FACE_H_

using namespace std;
#include <vector>

#include "Exceptions.h"
#include "Sphere.h"
#include "geoLib.h"
#include "funcs.h"

/**
 * 	0: polyLine
 *  1: arc
 *  2: line segment
 *  3: triangular
 *  4: quadrilateral
 *  5: polygon
 *  6: circle
 *  7: bspline
 *  8: sphere
 *  9: cylinder
 *  10: polygonPatch
 *  11: misc line
 *
 */

class Polygon;

class Face{

public:

	Face(int type, vector<double> &info);

	Face(int type, vector<int> &info, vector<Polygon*> &f); //special constructor for PolygonPatch

	virtual ~Face();

	double distanceToPoint(double *p);
	double distanceToFace(Face *f);
	double distanceToPolygon(Polygon *poly);
	double distanceToSphere(Sphere *s);
	int    getNumIntersectPntWithRay(double *x, double *dire);
	void   project2Face(double *x, double *p, __int64& bnd);
	bool   getInterSectLine(Face *face, vector<Face*>& interLine);
	bool   getInterSectPointWithCube(double cx[3], double size, double *pnt);
	bool   cubeInterSectCheck(double cb[3], double size);
	bool   getCubeInterSectMidPoint(double *cb, double cSize, double mid[3]);

	double* getPlanarFaceFunction();


	int type;
	__int64 neighZone[2];
	        // -1  : not in the zone
	        // >=0 : zone id
	        // -2  : on the boundary
	__int64 bnd;
			// -10 -11 -12 -13 -14 -15 is reserved for the six surface for domain
			// -1: vtx not on the boundary
			//>=0: vtx on the boundary, bnd id of the boundary
			//        bnd id should be non-negative
	int zDefFace; 	// 1 if it is a zone defining face;
					// 0 if not a zone defining face;
					// whether a face is zone-defining face or a partition face is decided by the two neighbor zone sign
					//if the two neighbor zone sign are the same then it is a partition face

	void *f;

private:
	void   intilizeFuncs();
	double (*ptDistToPoint)(double *p, void *my);
	double (*ptDistToPolygon)(void *poly, void *my);
	double (*ptDistToSphere)(void *sphr, void *my);
	int    (*ptGetNumIntersectPntWithRay)(double *x, double *dire, void *my);
    bool   (*ptProject2Face)(double *x, double *p, void *my);


    void   createPolygonPatch( vector<double>& info );

};

#endif /* FACE_H_ */
