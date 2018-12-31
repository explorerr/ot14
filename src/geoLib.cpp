/*
 * geoLib.cpp
 *
 *  Created on: Apr 4, 2009
 *      Author: zrui
 */

#include "stdafx.h"
#include "geoLib.h"
#include "Polygon.h"
#include "PolygonPatch.h"
#include "Arc.h"


//return the shortest distance between two faces, will return 0 if faces intersects
double getShortestDistance(Face &f1, Face &f2){

	double dist = 0;
	int type = f1.type*0x10 + f2.type;
	switch (type){
	case 0x22: 	// linear to linear
				// info contains the coordinates (x, y, z) of the two vertices of the edge
		dist = segmentSegmentDistance3D( (Polygon*) f1.f, (Polygon*) f2.f );
		break;

	case 0x23: 	// trianglar
				// three coordinates (x, y, z) of the three vertices of the triangle
		//TODO
		break;
	case 0x24: 	//quadrilateral coplanar
				// four coordinates (x, y, z) of the four vertices of the quadriateral
		//TODO
		dist = segmentSegmentDistance3D( (Polygon*) f1.f, (Polygon*) f2.f );
		break;

	case 0x25: 	// Polygonal n
				// first data is the number of vertices of the Polygon NN
				// follows NN coordinates (x, y, z) of the NN vertices of the quadriateral
		//TODO
		dist = segmentSegmentDistance3D( (Polygon*) f1.f, (Polygon*) f2.f );
		break;

	case 0x26: 	// circle
				// first is the diameter of the circle, and followed by the coordinates (x, y, z)
				// of the center point
		//TODO
		break;

	case 0x27: 	// bspline
				// tobe decided
		//TODO
		break;

	case 0x28: 	// sphere
				// first is the diameter of the circle, and followed by the coordinates (x, y, z)
				// of the center point
		//TODO
		break;
	case 0x32: 	// triangle to linear
				// info contains the coordinates (x, y, z) of the two vertices of the edge
		dist = segmentSegmentDistance3D( (Polygon*) f1.f, (Polygon*) f2.f );
		break;
	}
	return dist;
}



double segmentSegmentDistance3D(Polygon *uu, Polygon *vv){
	double base0[3], base1[3], dirt0[3], dirt1[3], u[3], v[3];
	double a, b, c, d, e, det, sNum, tNum, tDenom, sDenom, s, t;
	for(int i=0; i<3; i++) {
		dirt0[i] = uu->v[1][i] - uu->v[0][i];
		dirt1[i] = vv->v[1][i] - vv->v[0][i];
		base0[i] = uu->v[0][i];
		base1[i] = vv->v[0][i];
	}

	for( int i=0; i<3; i++) u[i] = base0[i] - base1[i];
	a = Dot(	dirt0, dirt0	);
	b = Dot(	dirt0, dirt1	);
	c = Dot(	dirt1, dirt1	);
	d = Dot(	dirt0, u	);
	e = Dot(	dirt1, u	);
	det = a * c - b * b;
	//check for (near) parallelism
	double myEPSILON;
	myEPSILON = EPSILON;
	if( det < myEPSILON ) {
		sNum = 0;	tNum = e;	tDenom = c;		sDenom = det;
	}
	else{
		//find parameter values of closest points
		//on each segment's infinite line Denominator
		//assumed at this point to e "det"
		//which is always positive, we can check
		//value of numerators to see if we're outside
		sNum = b * e - c * d; 	tNum = a * e - b * d;
	}

	//check s
	sDenom = det;
	if(sNum <0){ sNum = 0;	tNum = e; 	tDenom = c; }
	else if( sNum > det) { 	sNum = det; 	tNum = e + b;	tDenom = c; }
	else tDenom = det;

	//check t
	if(tNum < 0) {
		tNum = 0;
		if(-d < 0) sNum = 0;
		else if(-d > a) sNum = sDenom;
		else { sNum = -d; sDenom = a; }
	}
	else if(tNum > tDenom ){
		tNum = tDenom;
		if( (-d + b) < 0 ) sNum = 0;
		else if ( (-d + b) > a ) sNum = sDenom;
		else { sNum = -d + b; 	sDenom = a; }
	}

	//parameters of nearest points on restricted domain
	s = sNum / sDenom;
	t = tNum / tDenom;

	//Dot product of vector between points is squared distance
	//between segments
	for (int i=0; i<3; i++)
		v[i] = base0[i] + ( s * dirt0[i]) - base1[i] + ( t * dirt1[i]);

	return Dot(v, v);

}
/**
 * Return the dot product of the two vectors
 *
 * @param x1 x coordinates of p1
 * @param y1 y coordinates of p1
 * @param x2 x coordinates of p2
 * @param y2 y coordinates of p2
 */
double Dot(double x1, double y1, double z1, double x2, double y2, double z2 ){
	return x1*x2 + y1*y2 + z1*z2;
}

/**
 * Return the dot product of the two vectors
 *
 * @param u coordinates of p1
 * @param v coordinates of p2
 */
double Dot(double* u, double* v ){
	return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

double Dot2D(double *u, double *v){
	return u[0] *v[0] + u[1]*v[1];
}

/**
 * Return the cross product of the two vectors
 * @param u coordinates of p1
 * @param v coordinates of p2
 * @param cross the resulting cross product vector of the two vectors
 */
void Cross(double *u, double *v, double *cross){
	cross[0] = u[1]*v[2] - u[2]*v[1];
	cross[1] = u[2]*v[0] - u[0]*v[2];
	cross[2] = u[0]*v[1] - u[1]*v[0];
}

/**
 * Return the cross product of the two vectors
 *
 * @param u coordinates of p1
 * @param v coordinates of p2
 * @param cross the resulting cross product vector of the two vectors, normalized
 */
void CrossNormalized(double *u, double *v, double *cross){
	double myEPSILON = EPSILON;
	cross[0] = u[1]*v[2] - u[2]*v[1];
	cross[1] = u[2]*v[0] - u[0]*v[2];
	cross[2] = u[0]*v[1] - u[1]*v[0];
	double l=0;
	for(int i=0; i<3; i++)l += cross[i] * cross[i];
	l = sqrt(l);
	if(l < myEPSILON)
		return;
	for(int i=0; i<3; i++) cross[i] = cross[i] / l;
}


void normalize(double *u){

    double len = u[0]*u[0] + u[1]*u[1] + u[2]*u[2];
    len = sqrt(len);
    u[0] /= len;
    u[1] /= len;
    u[2] /= len;

}

void normalize2D(double *u){
    double len = u[0]*u[0] + u[1]*u[1];
    u[0] /= len;
    u[1] /= len;

}

/**
 * The distance between quads
 *
 * @param uu the coordinates of the quad 1, in right-hand sequence
 * @param vv the coordinates of the quad 2, in right-hand sequence
 */
double quadrilateralQuadrilateralDistance(double *uu, double *vv){
	for(int i=0; i<4; i++){
//		pointToPolygonDistance(uu+i*3*sizeof(double), )
	}

	return 0.;
}

/**
 * The distance between ploygons
 *
 * @param uu the coordinates of the Polygon 1, in right-hand sequence
 * @param vv the coordinates of the Polygon 2, in right-hand sequence
 */
double polygonToPolygonDistance(Polygon *p, Polygon *q){
	int plus=0, neg=0, nv;
	double myEPSILON = EPSILON;
	double  dist=0, temp;
	nv = p->nv;
	for(int i=0; i<nv; i++){
		temp = pointToPolygonDistance(p->v[1], q);
		if( fabs(temp) < myEPSILON)return 0;
		( temp<0 ) ? neg = 1 : plus = 1;
		if(fabs(dist)< fabs(temp))dist = temp;
	}
	if( (neg*plus) != 0) return 0;
	neg = plus = 0;
	nv = q->nv;
	for(int i=0; i<nv; i++){
		temp = pointToPolygonDistance(q->v[1], p);
		if( fabs(temp) < myEPSILON )return 0;
		( temp<0 ) ? neg = 1 : plus = 1;
		if(fabs(dist)< fabs(temp))dist = temp;
	}
	if( (neg*plus) != 0) return 0;

	return dist;
}

/**
 * This is for the function assignment in Face class
 * The distance between ploygons
 *
 * @param uu pointer to object which will be casted to Polygon
 * @param vv pointer to object which will be casted to Polygon
 */
double polygonToPolygonDistancePtrUsage(void *p, void *q){
	return 	polygonToPolygonDistance((Polygon*) p, (Polygon*) q);
}

/**
 * Returns distance between quad to quad
 *
 * @param p pointer to Polygon object
 * @param q pointer to Polygon object
 */
double quadrilateralToQuadrilateralDistance(Polygon *p, Polygon *q){
	int plus=0, neg=0;
	double  dist=0, temp;
	for(int i=0; i<4; i++){
		temp = pointToPolygonDistance(p->v[1], q);
		if( ISZERO(temp) )return 0;
		( temp<0 ) ? neg = 1 : plus = 1;
		if(fabs(dist)< fabs(temp))dist = temp;
	}
	if( (neg*plus) != 0) return 0;
	neg = plus = 0;
	for(int i=0; i<4; i++){
		temp = pointToPolygonDistance(q->v[1], p);
		if( ISZERO(temp) )return 0;
		( temp<0 ) ? neg = 1 : plus = 1;
		if(fabs(dist)< fabs(temp))dist = temp;
	}
	if( (neg*plus) != 0) return 0;

	return dist;
}


/**
 * Returns distance between point and Polygon
 * ref: P300
 * The sequence of the Polygon points follows right-hand rule
 * and the distance is positive if at the positive side of the face
 * negative at the negative side of the face according to right-hand rule
 *
 * @param p coordinates of the points
 * @param vv coordinates of the points of the Polygon, in right-hand rule sequence
 * @param nv number of the points of the Polygon
 * @param func the function parameters of the plane of the polygon 
 */

double pointToPolygonDistance(double *p, double *vv, int nv, double *func, double *pPrime){

//	double pPrime[3];
	pPrime[0] = p[0] - ((Dot(p, func)+func[3])/Dot(func, func))*func[0];
	pPrime[1] = p[1] - ((Dot(p, func)+func[3])/Dot(func, func))*func[1];
	pPrime[2] = p[2] - ((Dot(p, func)+func[3])/Dot(func, func))*func[2];

	//calculate the sign of the distance
	double signDouble = (func[0]*p[0] + func[1]*p[1] + func[2]*p[2] + func[3] );
	int sign = signDouble > 0 ? 1 : -1;

	//determine plane to project Polygon to
    int pIdx=0;
    for(int i=1; i<3; i++){
        if( fabs(func[i]) > fabs(func[pIdx]) )
            pIdx = i;
    }

    //project the polygon and pPrime onto the plane;
    double pPrimePrime[3];
    for(int i=0; i<2; i++){
        pPrimePrime[i] = pPrime[(pIdx+1+i)%3];
    }
    pPrimePrime[2] = 0;

    double *poly2D;
    poly2D = (double*) malloc(sizeof(double)*nv*2);
    for(int i=0; i<nv; i++){
        for(int j=0; j<2; j++){
            int id = i*3+((pIdx+1+j)%3);
            poly2D[i*2+j] = vv[id];
        }
    }

    //find  closest point in 2D
    double qPrime[3];
    double dist2D;
    dist2D = pointToPolygonDistance2D(pPrimePrime, poly2D, nv, qPrime);
    if(dist2D==-1)
        return func[0]*p[0] + func[1]*p[1] + func[2]*p[2] + func[3];

    //compute q, the closest point on the 3D polygon's plane
    double q[3];
    q[pIdx] = 0;
    for(int i=0; i<2; i++)
        q[(pIdx+1+i)%3] = qPrime[i];
    double third = 0;
    qPrime[2]=0;
    third = func[0]*q[0] + func[1]*q[1] + func[2]*q[2] +func[3];
    third = third/func[pIdx];

    double d[3];
    d[0] = p[0] - q[0];
    d[1] = p[1] - q[1];
    d[2] = p[2] - q[2];

    free(poly2D);

    return sign*sqrt(Dot(d, d));
}

double pointToPolygonDistance(double *p, double *vv, int nv, double *func){
	
	double pPrime[3];


	return pointToPolygonDistance(p, vv, nv, func, pPrime);


}

/**
 *  This function will return the distance from a point to a polygon
 *  and will return the closest point in the polygon to the point
 *
 *  ref p194
 *
 *  @param p the point coordinates in 2D
 *  @param poly2D the vertices of polygon in 2 D
 *  @param nv number of vertices of the polygon
 *  @param q the closest point on the polygon to p
 *
 *  return:
 *  (0,   ) distance from p to poly2D
 *   -1     p inside the poly2D
 *
 */
double pointToPolygonDistance2D(double *p, double *poly2D, int nv, double *q){


    if (pointInPolygonCheck2D(p, poly2D, nv)) {
        q[0] = p[0]; q[1] = p[1];
        return -1;
    }

    double miu;
    miu = pointToLineSegDistance2D(p, poly2D, q);
    double xPre, yPre;
    double x, y;
    xPre = poly2D[2] - p[0];
    yPre = poly2D[3] - p[1];
    for(int i=1; i<nv; i++){
        x = poly2D[(i+1)%nv*2] - p[0];
        y = poly2D[(i+1)%nv*2+1] - p[1];
        if( fabs(xPre) >= miu &&
                fabs(x) >= miu &&
                    x*xPre > 0){

            xPre = x; yPre = y;
            continue;
        }
        if ( fabs(yPre) >= miu &&
                fabs(y) >= miu &&
                    yPre * y >0 ){
            xPre = x; yPre = y;
            continue;
        }
        else {
        	double curS[4];
           	curS[0] = poly2D[i*2]; curS[1] = poly2D[i*2+1];
           	curS[2] = poly2D[((i+1)%nv)*2]; curS[3] = poly2D[((i+1)%nv)*2+1];
            double qPrime[2];
            double tmpDist = pointToLineSegDistance2D(p, curS, qPrime);
            if(tmpDist<miu){
                miu = tmpDist;
                q[0] = qPrime[0]; q[1] = qPrime[1];
            }
        }
    }
    return miu;
}


/**
 *  This function will return the distance from a point to a line segment
 *  and will return the closest point on the line to the point
 *
 *  @param p the coordinate of the point
 *  @param lSeg the coordinates of the two ending point of the line segment
 *  @param q the closest point on the line to p
 */

double pointToLineSegDistance(double *p, double *lSeg, double *q){
    double dire[3], t;
    dire[0] = lSeg[3] - lSeg[0];
    dire[1] = lSeg[4] - lSeg[1];
    dire[2] = lSeg[5] - lSeg[2];

    double dist = pointToLineDistance2D(p, lSeg, dire, q, t);

    if(t<0){
        q[0] = lSeg[0]; q[1] = lSeg[1]; q[2] = lSeg[2];
        return pointToPointDistance2D(p , lSeg);
    }
    else if(t>1){
        q[0] = lSeg[3]; q[1] = lSeg[4]; q[2] = lSeg[5];
        return pointToPointDistance2D(p , lSeg);
    }
    else return dist;
}
double pointToLineSegDistance2D(double *p, double *lSeg, double *q){
    double dire[2];
    dire[0] = lSeg[2] - lSeg[0];
    dire[1] = lSeg[3] - lSeg[1];

    double pl[2];
    pl[0] = p[0] - lSeg[0];
    pl[1] = p[1] - lSeg[1];
    double t = Dot2D(dire, pl);
    if(t<=0){
    	q[0] = lSeg[0]; q[1] = lSeg[1];
        return sqrt(Dot2D(pl, pl));
    }
    double ll = Dot2D(dire, dire);
    if(t>=ll){
    	q[0] = lSeg[2]; q[1] = lSeg[3];
        double pl2[2] = { p[0] - lSeg[2], p[1] - lSeg[3]};
        return sqrt(Dot2D(pl2, pl2));
    }
    q[0] = lSeg[0] + t*dire[0]/ll;
    q[1] = lSeg[1] + t*dire[1]/ll;
    return sqrt(Dot2D(pl, pl) - t*t/ll);
}

double pointToLineSegDistancePtrUsage(double *p, Face *lSeg){

    double q[3];
    Polygon *poly = (Polygon*)lSeg->f;
    double *ll;
    ll = (double*)malloc(sizeof(double)*poly->nv*3);
    for(int i=0; i<poly->nv; i++){
    	ll[i*3+0]= poly->v[i][0];
    	ll[i*3+1]= poly->v[i][1];
    	ll[i*3+2]= poly->v[i][2];

    }
    return  pointToLineSegDistance(p, ll, q);
}

/**
 *  This function will return the distance from a point to a line
 *  and will return the closest point on the line to the point
 *
 *  @param p the coordinate of the point
 *  @param line the coordinates of one point on the line
 *  @param dire the direction vector of the line
 *  @param q the closest point on the line to p
 */
double pointToLineDistance(double *p, double *o, double *dire, double *q, double& t){

    double vec[3] ={ p[0]-o[0], p[1]-o[1], p[2]-o[2]};
    t  = Dot(dire, vec);
    q[0] = o[0] + t*dire[0];
    q[1] = o[1] + t*dire[1];
    q[2] = o[2] + t*dire[2];

    double v[3];
    v[0] = p[0] - q[0];
    v[1] = p[1] - q[1];
    v[2] = p[2] - q[2];

    return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}



/**
 *  This function will return the distance from a point to a line
 *  and will return the closest point on the line to the point
 *
 *  @param p the coordinate of the point
 *  @param line the coordinates of one point on the line
 *  @param dire the direction vector of the line
 *  @param q the closest point on the line to p
 */
double pointToLineDistance2D(double *p, double *line, double *dire, double *q, double& t){

    double vec[3] ={p[0]-line[0], p[1]-line[1]};
    t  = Dot(dire, vec);
    q[0] = p[0] + t*dire[0];
    q[1] = p[1] + t*dire[1];

    double v[2];
    v[0] = p[0] - q[0];
    v[1] = p[1] - q[1];

    return sqrt(v[0]*v[0] + v[1]*v[1]);
}






double pointToPointDistance2D(double *p, double *q){

    return sqrt( (p[0]-q[0])*(p[0]-q[0]) + (p[1]-q[1])*(p[1]-q[1]) );
}



/**t
 * Returns distance between point and Polygon
 * ref: P300
 * The sequence of the Polygon points follows right-hand rule
 * and the distance is positive if at the positive side of the face
 * negative at the negative side of the face according to right-hand rule
 *
 * @param p coordinates of the points
 * @param vv coordinates of the points of the Polygon, in right-hand rule sequence
 * @nv number of the points of the Polygon
 */
double pointToPolygonDistance(double * p, double *vv, int nv){

	double func[4];
	polyPlaneEquation(vv, nv, func[0], func[1], func[2], func[3]);
	return pointToPolygonDistance(p, vv, nv, func);
}

/**
 * Return point to Polygon distance
 * ref: P300
 * The sequence of the Polygon points follows right-hand rule
 * and the distance is positive if at the positive side of the face
 * negative at the negative side of the face according to right-hand rule
 *
 * @param p coordinates of the points
 * @param poly pointer to a Polygon object
 */
double pointToPolygonDistance(double *p, Polygon &poly){

    double *vv;
    vv = (double*) malloc(sizeof(double)*(poly.nv)*3);
    for(int i=0; i<poly.nv; i++){
        vv[i*3] = poly.v[i][0];
        vv[i*3+1] = poly.v[i][1];
        vv[i*3+2] = poly.v[i][2];
    }
	return pointToPolygonDistance(p, vv, poly.nv, poly.func);
    free(vv);
}

/**
 * Return point to Polygon distance
 * ref: P300
 * The sequence of the Polygon points follows right-hand rule
 * and the distance is positive if at the positive side of the face
 * negative at the negative side of the face according to right-hand rule
 *
 * @param p coordinates of the points
 * @param poly pointer to a Polygon object
 */
double pointToPolygonDistance(double *p, Polygon *poly){
    double *vv;
    vv = (double*) malloc(sizeof(double)*(poly->nv)*3);
    for(int i=0; i<poly->nv; i++){
        vv[i*3] = poly->v[i][0];
        vv[i*3+1] = poly->v[i][1];
        vv[i*3+2] = poly->v[i][2];
    }
	double dist = pointToPolygonDistance(p, vv, poly->nv, poly->func);
    free(vv);
    return dist;
}

double 	pointToPolygonDistance(double *p, Polygon *poly, double* pPrime){

	double *vv;
    vv = (double*) malloc(sizeof(double)*(poly->nv)*3);
    for(int i=0; i<poly->nv; i++){
        vv[i*3] = poly->v[i][0];
        vv[i*3+1] = poly->v[i][1];
        vv[i*3+2] = poly->v[i][2];
    }
	double dist = pointToPolygonDistance(p, vv, poly->nv, poly->func, pPrime);
    free(vv);
    return dist;
}



/**
 * This function is for the function assignment in Face class
 * Return point to Polygon distance
 * ref: P300
 * The sequence of the Polygon points follows right-hand rule
 * and the distance is positive if at the positive side of the face
 * negative at the negative side of the face according to right-hand rule
 *
 * @param p coordinates of the points
 * @param poly pointer to a Polygon object
 */
double pointToPolygonDistancePtrUsage(double *p, void* q){
	Polygon* poly = (Polygon*)q;
    return pointToPolygonDistance(p, poly);
}

double pointToPolygonDistancePtrUsage(double *p, Face* q){
	Polygon* poly = (Polygon*) q->f;
    return pointToPolygonDistance(p, poly);
}
/**
 * This function returns the Equation of the infinite surface that the Polygon is on
 * Normal vector and function parameter are normalized
 *
 * @param vv coordinates of the points of the Polygon, in right-hand rule sequence
 * @param nv number of vertices of the Polygon
 * @param a coefficient of the equation
 * @param b coefficient of the equation
 * @param c coefficient of the equation
 * @param d coefficient of the equation
 */
void polyPlaneEquation(double *vv, int nv, double &a, double &b, double &c, double &d){
	if (nv<3)throw StoppingException("Error: Polygon vertices less than three in PolyEquation.\n");

	double n[3], v[3], u[3];
	n[0]=n[1]=n[2]=0;
	double myEPSILON = EPSILON;
	int st=0;

	while(  (fabs(n[0]) + fabs(n[1]) + fabs(n[2]) ) < myEPSILON && st<nv){
		for(int i=0; i<3; i++){
	        v[i] = vv[((st+1)%nv)*3+i] - vv[st*3+i];
	        u[i] = vv[((st+2)%nv)*3+i] - vv[((st+1)%nv)*3+i];
	    }
		CrossNormalized(v, u, n);
		st++;

	}
	a = n[0];	b = n[1];	c = n[2];
	d = - a*vv[0] - b*vv[1] - c*vv[2];


}

/**
 * This function returns the Equation of the infinite surface that the Polygon is on
 *
 * @param vv coordinates of the points of the Polygon, in right-hand rule sequence
 * @param nv number of vertices of the Polygon
 * @param a pointer to the coefficients of the equation, array of double[4]
 */
void polyPlaneEquation(double *vv, int nv, double* a){

	if (nv<3)throw StoppingException("Error: Polygon vertices less than three in PolyEquation.\n");
	double n[3], v[3], u[3];
	n[0]=n[1]=n[2]=0;
	double myEPSILON = EPSILON;
	int st=0;

	while(  (fabs(n[0]) + fabs(n[1]) + fabs(n[2]) ) < myEPSILON && st<nv){
		for(int i=0; i<3; i++){
	        v[i] = vv[((st+1)%nv)*3+i] - vv[st*3+i];
	        u[i] = vv[((st+2)%nv)*3+i] - vv[((st+1)%nv)*3+i];
	    }
		CrossNormalized(v, u, n);
		st++;

	}
	a[0] = n[0];	a[1] = n[1];	a[2] = n[2];
	a[3] = - a[0]*vv[0] - a[1]*vv[1] - a[2]*vv[2];
}

void polyPlaneEquation(vector<double> &vv, int nv, double* a){

	if (nv<3)throw StoppingException("Error: Polygon vertices less than three in PolyEquation.\n");
	double n[3], v[3], u[3];
	n[0]=n[1]=n[2]=0;
	double myEPSILON = EPSILON;
	int st=0;

	while(  (fabs(n[0]) + fabs(n[1]) + fabs(n[2]) ) < myEPSILON && st<nv){
		for(int i=0; i<3; i++){
	        v[i] = vv[((st+1)%nv)*3+i] - vv[st*3+i];
	        u[i] = vv[((st+2)%nv)*3+i] - vv[((st+1)%nv)*3+i];
	    }
		CrossNormalized(v, u, n);
		st++;

	}
	a[0] = n[0];	a[1] = n[1];	a[2] = n[2];
	a[3] = - a[0]*vv[0] - a[1]*vv[1] - a[2]*vv[2];
}

/*
 * Return the distance from the point to the sphere,
 * + outside
 * - inside
 *
 * @param p coordinates of the point
 * @param s the sphere objects
 */
double pointToSphereDistance(double *p, Sphere &s){
	double temp=0;
	for(int i=0; i<3; i++)	temp += pow( p[i] - s.center[i],  2 );
	return sqrt(temp) - s.radius;
}

/*
 * This is for the function assignment in the Face class
 * Return the distance from the point to the sphere
 * + outside
 * - inside
 *
 * @param p coordinates of the point
 * @param s the sphere objects
 */
double pointToSphereDistancePtrUsage(double *p, void *s){
	double temp=0;
	Sphere* ss = (Sphere*) s;
	for(int i=0; i<3; i++)	temp += pow( p[i] - ss->center[i],  2 );
	return sqrt(temp) - ss->radius;
}
double pointToSphereDistancePtrUsage(double *p, Face *s){
	double temp=0;
	Sphere* ss = (Sphere*) s->f;
	for(int i=0; i<3; i++)	temp += pow( p[i] - ss->center[i],  2 );
	return sqrt(temp) - ss->radius;
}

/**
 * Returns the distance between two points
 *
 * @param a coordinates of point 1
 * @param b coordinates of point 2
 */
double pointToPointDistance(double *a, double *b){
	double temp = 0;
	for(int i=0; i<3; i++) temp += (a[i]-b[i])*(a[i]-b[i]);
	return sqrt(temp);
}
double pointToPointDistanceSqr(double *a, double *b){
	double temp = 0;
	for(int i=0; i<3; i++) temp += (a[i]-b[i])*(a[i]-b[i]);
	return temp;
}
/**
 * Return the distance between a Polygon and a sphere
 *
 * @param p pointer to the Polygon object
 * @param s pointer to the sphere object
 */
double polygonToSphereDistance(Polygon *p, Sphere *s){
	return  fabs( pointToPolygonDistance(s->center, p) ) - s->radius;
}

/**
 * This is for the function assignment in Face class
 * Return the distance between a Polygon and a sphere
 *
 * @param p pointer to the Polygon object (need to be casted)
 * @param s pointer to the sphere object (need to be casted)
 */
double polygonToSphereDistancePtrUsage(void *p, void *q){
	return  polygonToSphereDistance((Polygon*) p, (Sphere*) q);
}

/**
 * This is for the function assignment in Face class
 * Return the distance between a Polygon and a sphere
 *
 * @param p pointer to the sphere object (need to be casted)
 * @param s pointer to the Polygon object (need to be casted)
 */
double sphereToPolygonDistancePtrUsage(void *p, void *q){
	return  polygonToSphereDistance( (Polygon*) q, (Sphere*) p);
}

/**
 * This is for the function assignment in Face class
 * Return the distance between two spheres
 * @param p pointer to the sphere object (need to be casted)
 * @param s pointer to the sphere object (need to be casted)
 */
double sphereToSphereDistancePtrUsage(void *s, void *c){
	Sphere *ss = (Sphere*)s;
	Sphere *cc = (Sphere*)c;
	return pointToPointDistance(ss->center, cc->center) - ss->radius - cc->radius;
}




/**
 * This function will find the number of intersection point between the ray line
 * and line segment object
 *
 * @param x the coordinates of the start point of the ray line
 * @param dire the direction vector of the ray line
 * @param p the line segment object, Polygon with nv=2
 */
int getNumIntPntRayLinePtrUsage(double *x, double* dire, void *f){
	Polygon* l = (Polygon*) f;

	if(l->nv!= 2)
		throw StoppingException("Error: in getNumIntPntRayLinePtrUsage, object is not a line.\n");


	/**
	 * this function will first check if starting point of the ray is on the
	 * line segment, if yes, then return 1
	 * if not construct a plane with starting point and the line segment
	 * check if the ray is on the plane
	 * if no then return 0
	 * if yes, check if the ray is parallel with the segment
	 * if yea, return 0
	 * else return 1
	 *
	 */
    double myEPSILON;
	myEPSILON = EPSILON;
	double t[3];
	double *p, *q;
	p= l->v[0];
	q= l->v[1];
	for(int i=0; i<3; i++){
		t[i] =  (x[i] - p[i])/(p[i] - q[i]);
	}
	if(fabs(t[0] - t[1]) > myEPSILON
			&& fabs(t[1] - t[2]) < myEPSILON
			&& fabs(t[2] - t[0]) < myEPSILON){ // starting point is on the line segment,
		                                       //then the starting point is the intersection
		                                       // point, will return 1
		return 1;
	}

    double coef[4];
    double vv[9] = {x[0], x[1], x[3], p[0], p[1], p[2], q[0], q[1], q[2]};
    polyPlaneEquation(vv, 3, coef);
    if( Dot((double*)coef, dire) > myEPSILON ){// the ray is not on the plane,
										//there is no intersection point, will return 0
        return 0;
    }
    double segdir[3] = {p[0]-q[0], p[1]-q[1], p[2]-p[2]};
    double crossPro[3];
    Cross(segdir, dire, crossPro);
    if(fabs(crossPro[0])>myEPSILON
    		|| fabs(crossPro[1])>myEPSILON
    		|| fabs(crossPro[2])>myEPSILON){ // not paralelle, there will be one intersection point.
        return 1;
    }
    else return 0;
}

/**
 * This function will find the number of intersection point between the ray line
 * and and a triangle face objet
 *
 * @param x the coordinates of the start point of the ray line
 * @param dire the direction vector of the ray line
 * @param f the triangle face object, Polygon with nv=3
 */
int getNumIntPntRayTrianglePtrUsage(double *x, double* dire, void *f){

	Polygon* l = (Polygon*) f;

	if(l->nv!= 3)
		throw StoppingException("Error: in getNumIntPntTrianglePtrUsage, object is not a triangle face.\n");

	/**
     * This function ref the Geometric Tools for Computer Graphics p355
     *
     */
   double myEPSILON;
	myEPSILON = EPSILON;
    double e1[3], e2[3], p[3], s[3], q[3];
    double t, u, tmp;

    e1[0] = l->v[1][0] - l->v[0][0];
    e1[1] = l->v[1][1] - l->v[0][1];
    e1[2] = l->v[1][2] - l->v[0][2];
    e2[0] = l->v[2][0] - l->v[0][0];
    e2[1] = l->v[2][1] - l->v[0][1];
    e2[2] = l->v[2][2] - l->v[0][2];

    Cross(dire, e2, p);
    tmp = Dot(p, e1);
    if(fabs(tmp) < myEPSILON){
        return 0;
    }
    tmp = 1.0/tmp;
    s[0] = x[0] - l->v[0][0];
    s[1] = x[1] - l->v[0][1];
    s[2] = x[2] - l->v[0][2];

    u = tmp * Dot(s, p);

    if(u < 0.0 || u > 1.0)
     return 0;

    t = tmp * Dot(e2, q);

    return 1;
}



/**
 * This function will find the number of intersection point between
 * the ray line and a Polygon face objet
 * If there are intersection point, there will be only one
 * but for consistancy with other surfaces, that could have multiple
 * intersection points, like a sphere
 * this function returns the number of intersection points
 * thus the calling function can be consistant
 *
 * @param x the coordinates of the start point of the ray line
 * @param dire the direction vector of the ray line
 * @param f the Polygon face object, Polygon with nv>3
 */
int getNumIntPntRayPolygonPtrUsage(double *x, double* dire, void *f){

	Polygon* l = (Polygon*) f;

	if(l->nv < 3)
		throw StoppingException("Error: in getNumIntPntPolygonPtrUsage, object is not a Polygon face.\n");

    if(l->squareFlg)
    	return squareIntersectCheck(x, dire, l);

    if( ! circleIntersectCheck(x, dire, l->center, l->radius, l->n))
    	return 0;

    return polygonIntersectCheck(x, dire, l);
}

/**
 *  This is a general ray-Polygon intersection checking function
 *  will perform the method ref in GTCG p489
 *
 *  @param x coordinates of the start point of the ray line
 *  @param dire the direction vector of the ray line
 *  @param f pointer to the Polygon object
 *
 * */
int polygonIntersectCheck(double *x, double *dire, Polygon *f){

    double myEPSILON;
	myEPSILON = EPSILON;

    double p[3], e1[3], e2[3];
    double numer, denom, t;
    e1[0] = f->v[1][0] - f->v[0][0];
    e1[1] = f->v[1][1] - f->v[0][1];
    e1[2] = f->v[1][2] - f->v[0][2];
    e2[0] = f->v[2][0] - f->v[1][0];
    e2[1] = f->v[2][1] - f->v[1][1];
    e2[2] = f->v[2][2] - f->v[1][2];

    p[0] = f->v[0][0];
    p[1] = f->v[0][1];
    p[2] = f->v[0][2];
    denom = Dot(dire, f->n);
    if(fabs( denom) >myEPSILON ){ // not perpendicular to the norm
        double tmp[3];
        tmp[0] = p[0] - x[0]; tmp[1] = p[1] - x[1]; tmp[2] = p[2] - x[2];
        numer = Dot(f->n, tmp);
        t = numer/ denom;
        if(t<0) // on the opposite direction of the ray
            return 0;
        p[0] = x[0] + t*dire[0];
        p[1] = x[1] + t*dire[1];
        p[2] = x[2] + t*dire[2];
        int projIdx = 0;
        for(int i=1; i<2; i++){
            if( fabs(f->n[i]) > fabs(f->n[projIdx]) ){
                projIdx = i;
                break;
            }
        }
        if(projIdx == -1)
        	throw StoppingException("Error: in polygonIntersectCheck, Polygon normal is zero.\n");

        double * twoDPoints;
        double p2D[2];
        twoDPoints = (double*) malloc(sizeof(double)*2*(f->nv));
        for(int i=0; i<f->nv; i++){
            for(int j=0; j<2; j++){
                twoDPoints[i*2+j] = f->v[i][(j+projIdx+1)%3];
            }
        }
        p2D[0] = p[(projIdx+1)%3];
        p2D[1] = p[(projIdx+2)%3];
        
		if(pointInPolygonCheck2D(p2D, twoDPoints, f->nv)){
            
			free(twoDPoints);
			return 1;
		}
        else{
			
			free(twoDPoints);
            return 0;
		}
    }
    else {
		return 0;
	}
}


/**
 *  This function will check if a 2D point is inside the polygon
 *  This method will again project a ray to check number of intersection
 *  points, if odd then inside, else then outside
 *
 *  This method will require that the Polygon is manifold
 *
 *  @param p2D the coordinates of the 2D points
 *  @param vv the coordinates of the vertices of the Polygon
 *  @param nv number of vertices of the Polygon
 *
 * */
bool pointInPolygonCheck2D(double *p2D, double **vv, int nv){

    double t;
    int sum=0;

    for(int i=0; i<nv; i++){
        t = (p2D[1] -vv[i][1]) /( vv[(i+1)%nv][1] - vv[i][1] );
        if(t<0 || t>1)
            continue;
        t = vv[i][0] + t*(vv[(i+1)%nv][0] - vv[i][0]);
        if( (t - p2D[0]) > 0)//intersect point at positive side of the ray
            sum++;
    }
   if( sum%2 ==0)
       return true;
   else
       return false;
}

/**
 *	The method will pose a x-direction ray
 *  then lookat the y coordinates of the line segment
 *  if both at the same side of the ray then no intersection possible
 *  else find the point on the line segment, whose y coordinates is the same as the ray
 *  if the point is at the positive side of the ray (x coord > o[0])
 *  then there is intersection 
 *  else there is no intersection
 *
 *
 */

bool pointInPolygonCheck2D(double *p2D, double *vv, int nv){

    double px;
    int sum=0;
    double myEPSILON = 0.0001;
	
	double p1[2], p2[2];

	for(int i=0; i<nv; i++){

		p1[0] = vv[i*2];  p1[1] = vv[i*2+1];

		p2[0] = vv[((i+1)%nv)*2]; p2[1] = vv[((i+1)%nv)*2+1];

		if(  (p1[1] - p2D[1]) * (p2[1] - p2D[1]) > 0 ) //at the same side

			continue;

		else {

			px = p2[1]-p1[1];
			if(px==0)
				continue;
			px = (p2D[1]-p1[1])/px;

			px = p1[0] + px * (p2[0]-p1[0]);

			if(px<p2D[0])//negative side
				continue;
			else
				sum++;

		}

	}

	if( sum%2 ==0)
		return false;
	else
		return true;


/* 

    for(int i=0; i<nv; i++){

    	double deNorm = ( vv[((i+1)%nv)*2+1] - vv[i*2+1] );
    	if( fabs(deNorm) < myEPSILON )// This line segment is parallel with the ray, will not count
    		continue;
        
		t = (p2D[1] -vv[i*2+1]) /deNorm;

        if( fabs(t) < myEPSILON ){ // intersects on start point of the line segment in the polygon
        	                       // check the end point
        	                       // if the end point is below the ray then count
        	if( vv[((i+1)%nv)*2+1] < p2D[1] ) // end point below the ray
        		sum++;
        }
        else if (fabs(t-1) < myEPSILON){ // intersects on the end point of the line segment in the polygon
										// check the start point
										// if the start point is below the ray then count
			if( vv[i*2+1] < p2D[1] ) // start point below the ray
				sum++;
        }
        else if(t<0 || t>1)
            continue;
        doubel x = vv[i*2] + t*(vv[((i+1)%nv)*2] - vv[i*2]);
        if( (x - p2D[0]) > 0)//intersect point at positive side of the ray
            sum++;
    }
   if( sum%2 ==0)
       return false;
   else
       return true;

	   */
}


/**
 *  This funtion will check if the points is in the polygon
 *  Assumption is that the point is on the same plane with the polygon
 *  
 *  Will projec the polygon and point to a axis plane
 *  Check using the function of pointInPolygonCheck2D 
 *  
 *  @param poly the pointer to a polygon
 *  @param pnt  the point to be checked
 *  @return ture the point is in the polygon
 *  @return false the point is outside the polygon
 *
 */
bool pointInCoplanarPolygonCheck3D(Polygon *poly, double* pnt){


	int pIdx=0;
   	for(int i=1; i<3; i++){
       	if( fabs(poly->func[i]) > fabs(poly->func[pIdx]) )
           	pIdx = i;
    }

	//project the polygon and pnt onto the plane;
    double pPrime[2];
   	for(int i=0; i<2; i++){
    	pPrime[i] = pnt[(pIdx+1+i)%3];
    }
	
	double *poly2D;
    poly2D = (double*) malloc(sizeof(double)*(poly->nv)*2);
    for(int i=0; i<poly->nv; i++){
    	for(int j=0; j<2; j++){
      	    int id = ((pIdx+1+j)%3);
       	    poly2D[i*2+j] = poly->v[i][id];
        }
    }


	

    
	bool ret = pointInPolygonCheck2D(pPrime, poly2D, poly->nv);

	free(poly2D);

	return ret;

}


/**
 *  This function will check if a ray intersects the special square,
 *  which is orthogonal to the coordinates axises
 *
 *  return code:
 *   1: inside
 *   0: outside
 *   -1: on the boundary
 *
 *  @param x the coordinates of the start point of the ray line
 *  @param dire the direction vector the ray line
 *  @param p the pointer of the Polygon object
 *
 * */
int squareIntersectCheck(double *x, double *dire, Polygon *p){

    double *n = p->n;
    double myEPSILON = EPSILON;
    int rIdx, cnt=0, pIdx, cnt2=0;
    for(int i=0; i<3; i++){
        if(fabs(dire[i]) < myEPSILON )cnt++;
        else rIdx = i;
       if(fabs(n[i]) >myEPSILON) pIdx = i;
        else cnt2++;
    }
    if(cnt2 != 2)
      throw StoppingException("Error: in squareIntersectCheck, Polygon passed is not a coordinate-orthogonal square.\n");

    if( cnt==2 ){ // ray orthogonal to the coordinates axies
        if( rIdx != pIdx ){ //parallel
            if( fabs(p->v[0][pIdx] - x[pIdx]) < myEPSILON ){ //starting point on the Polygon plane
            	double t[2];
            	for(int i=0; i<2; i++){
					int tmp = (pIdx+1+i)%3;
					t[i] =(x[tmp]-p->v[0][tmp])/(p->v[2][tmp]-p->v[0][tmp]);
				}
				if( (t[0] < 0 || t[0] > 1)||
					(t[1] < 0 || t[1] > 1) ) //starting point outside the Polygon
					return 0;
				else
					return -1;

            }
            else
            	return 0;
        }
        else{// not parallel, there may be intersection points
            //find the points on the Polygon plane
        	double tt = (p->v[0][rIdx]-x[rIdx])/dire[rIdx];
        	if(tt<0)//negative side of the ray
        		return 0;
            double t[2];
            for(int i=0; i<2; i++){
                int tmp = (rIdx+1+i)%3;
                t[i] =(x[tmp]-p->v[0][tmp])/(p->v[2][tmp]-p->v[0][tmp]);
            }
            if( (t[0] < 0 || t[0] > 1)||
                (t[1] < 0 || t[1] > 1) ) // not
                return 0;
            else{
            	if(tt==0)//on the plane and in the squere
            		return -1; // on the boundary
            	else return 1; // in the cube
            }
        }
    }
    else { // ray is not orthogonal to the coordinates axies
           // get the intersection point on the surface:
        double tt = ( p->v[0][rIdx] - x[rIdx] )/ dire[rIdx];
        double pp[3];
        for(int i=0; i<3; i++) pp[i] = x[i] + dire[i] * tt;
        double t[2];
        for(int i=0; i<2; i++){
            int tmp = (rIdx+1+i)%3;
            t[i] =(pp[tmp]-p->v[0][tmp])/(p->v[2][tmp]-p->v[0][tmp]);
        }
        if( fabs(t[0] - 1) > myEPSILON || fabs(t[1] -1) > myEPSILON)
            return 0;
        else return 1;
    }

}


/**
 *  This function will return the intersection of point
 *  between a line, and a plane
 *  Will throw ContinuingException if there is no intersection point
 *  ref: GTCG p352
 *
 *  @param x a point on the line
 *  @param dire direction vector of the line
 *  @param coeff coefficients of the plane equation
 *  @param pnt the returned position vector of the intersection point
 *  @param t the coefficient in the line function for the intersection point
 *
 * */
bool getLinePlaneIntersectPoint(double *x,
                                double *dire,
                                double *coeff,
                                double *pnt,
                                double &t){


    double denom;
    denom = coeff[0] *dire[0] + coeff[1] * dire[1] + coeff[2] * dire[2];
    if(denom == 0)
        return false;

    t =  coeff[0] * x[0] + coeff[1] * x[1] + coeff[2] * x[2] + coeff[3];
    t /= -(denom);
    pnt[0] = x[0] + t* dire[0];
    pnt[1] = x[1] + t* dire[1];
    pnt[2] = x[2] + t* dire[2];
    return true;

}

/**
 *  This function will check if a ray intersects the a circle,
 *  which is the circumcircle of the Polygon object
 *
 *  @param x the coordinates of the start point of the ray line
 *  @param dire the direction vector the ray line
 *  @param c the center of the circle
 *  @param r the radius of the circle
 *  @param n the direction vector of the circle
 *
 * */
int circleIntersectCheck(double *x,
                         double *dire,
                         double *c,
                         double r,
                         double *n){

    double coef[4], t, pnt[3];
    coef[0] = n[0]; coef[1] = n[1]; coef[2] = n[2];
    coef[3] = - n[0]*c[0] - n[1]*c[1] - n[2]*c[2];

    getLinePlaneIntersectPoint(x, dire, coef, pnt, t);

    if( t<0 ) // on the opposit direction of the ray
        return false;
    double sqrDist = ( pow(c[0]-pnt[0],2) + pow(c[1]-pnt[1],2) + pow(c[2]-pnt[2],2) );
    if( sqrDist > r*r)
       return false;
    else
       return true;

}

/**
 *  This function will check if a ray intersects the a sphere,
 *
 *  @param x the coordinates of the start point of the ray line
 *  @param dire the direction vector the ray line
 *  @param s the sphere object
 *
 * */
int getNumIntPntRaySpherePtrUsage(double *x,
                         double *dire,
                         void *s){

	return getNumIntPntRaySphere(x, dire, (Sphere*) s);
}
/**
 *  This function will check if a ray intersects the a sphere,
 *
 *  @param x the coordinates of the start point of the ray line
 *  @param dire the direction vector the ray line
 *  @param s the sphere object
 *
 * */
int getNumIntPntRaySphere(double *x,
                          double *dire,
                          Sphere *s){

	double b ,c, discrm;
	double pc[3];
	pc[0] = x[0] - s->center[0];
	pc[1] = x[1] - s->center[1];
	pc[2] = x[2] - s->center[2];

	b=Dot(dire, pc);
	c = Dot(pc, pc) - s->radius*s->radius;
	discrm = b*b -c;
	if(discrm > 0){
		double tmp = sqrt(discrm);
		double x1=-b-tmp, x2=-b+tmp;
		if( x1<0 && x2>0)
			return 1;
		else if(x2<0)
			return 0;
		else
			return 2;
	}
	else if (discrm == 0)
		return 1;
	else
		return 0;
}

bool project2LineSegPtrUsage(double *x, double *p, void *my){


	Polygon* line = (Polygon*) my;
	
	p[0] = x[0] - line->v[0][0];
	p[1] = x[1] - line->v[0][1];
	p[2] = x[2] - line->v[0][2];

	double len = 0;
	len += (line->v[1][0]-line->v[0][0])*(line->v[1][0]-line->v[0][0]);
	len += (line->v[1][1]-line->v[0][1])*(line->v[1][1]-line->v[0][1]);
	len += (line->v[1][2]-line->v[0][2])*(line->v[1][2]-line->v[0][2]);
	len = sqrt(len);

	double t = Dot(p, line->n);

	p[0] = line->v[0][0] + t*line->n[0];
	p[1] = line->v[0][1] + t*line->n[1];
	p[2] = line->v[0][2] + t*line->n[2];


	if( t > len || t < 0 )
		return false;
	else 
		return true;

	
	
	
	
	
	/*    Polygon * poly = (Polygon *) my;
    double dire[3] = {poly->v[1][0] - poly->v[0][0],
                      poly->v[1][1] - poly->v[0][1],
                      poly->v[1][2] - poly->v[0][2]};
    double pl[3] = {x[0] - poly->v[0][0],
                    x[1] - poly->v[0][1],
                    x[1] - poly->v[0][2]};

    double t = Dot(pl, dire);
    if(t<=0){
        p[0] = poly->v[0][0];
        p[1] = poly->v[0][1];
        p[2] = poly->v[0][2];
        return;
    }
    double ll = Dot(dire, dire);
    if(t>= ll){
        p[0] = poly->v[1][0];
        p[1] = poly->v[1][1];
        p[2] = poly->v[1][2];
        return;
    }

    p[0] = poly->v[0][0] + t*dire[0]/ll;
    p[1] = poly->v[0][1] + t*dire[1]/ll;
    p[2] = poly->v[0][2] + t*dire[2]/ll;

    return; */
}



/**
 * This function will project the given point to the polygon
 * 
 * @return if 
 *
 */


bool project2PolygonPtrUsage(double *x, double *p, void *my){

    Polygon * poly = (Polygon *) my;

	double D = Dot(x, poly->n) + poly->func[3];
	p[0] = x[0] - D*(poly->n)[0];
	p[1] = x[1] - D*(poly->n)[1];
	p[2] = x[2] - D*(poly->n)[2];
	
	return pointInCoplanarPolygonCheck3D(poly, p);
	
	
	/*    double tmpa, tmpb;
    double *func = poly->func;


    tmpa = Dot(x, func);
    tmpb = Dot(func, func);
	pPrime[0] = x[0] - ((tmpa+func[3])/tmpb)*func[0];
	pPrime[1] = x[1] - ((tmpa+func[3])/tmpb)*func[1];
	pPrime[2] = x[2] - ((tmpa+func[3])/tmpb)*func[2];

    	//determine plane to project Polygon to
    int pIdx=0;
    for(int i=1; i<3; i++){
        if( fabs(func[i]) > fabs(func[pIdx]) )
            pIdx = i;
    }

    //project the polygon and pPrime onto the plane;
    double pPrimePrime[3];
    for(int i=0; i<2; i++){
        pPrimePrime[i] = pPrime[(pIdx+1+i)%3];
    }
    pPrimePrime[2] = 0;

    double *poly2D;
    poly2D = (double*) malloc(sizeof(double)*nv*2);
    for(int i=0; i<nv; i++){
        for(int j=0; j<2; j++){
            int id =((pIdx+1+j)%3);
            poly2D[i*2+j] = poly->v[i][id];
        }
    }
    if (pointInPolygonCheck2D(pPrimePrime, poly2D, nv)){
        p[0] = pPrime[0];
        p[1] = pPrime[1];
        p[2] = pPrime[2];
    }
    else{
        double qPrime[3];
        pointToPolygonDistance2D(pPrimePrime, poly2D, nv, qPrime);
            //compute q, the closest point on the 3D polygon's plane
        double sum = 0;
        for(int i=0; i<2; i++)
            sum += func[(pIdx+1+i)%3] * qPrime[(pIdx+1+i)%3];
        sum = (-sum-func[3])/func[pIdx];

        p[pIdx] = sum;
        for(int i=0; i<2; i++)
            p[(pIdx+1+i)%3] = qPrime[i];
    }
    free(poly2D);
    return; */
}

bool project2SpherePtrUsage(double *x, double *p, void *my){

    Sphere * s = (Sphere *) my;
    double *c = s->center;
    double r  = s->radius;
    double dire[3] = { x[0] - c[0], x[1] - c[1], x[2] - c[2] };
    double l =sqrt( Dot(dire, dire) );
    p[0] = (dire[0]==0) ? 0 : c[0] + r/l*dire[0];
    p[1] = (dire[1]==0) ? 0 : c[1] + r/l*dire[1];
    p[2] = (dire[2]==0) ? 0 : c[2] + r/l*dire[2];
    return true;
}


/**
 *  The characteristic of architectural drawings is that usually face
 *  are connected with another face at the end edges, thus function will
 *  first check
 *
 *  if there are shared points between the two face, if there
 *  are two shared points then the edge constructed by the two points
 *  must be the interSectLine
 *
 *  else, the function will perform the generic interSectLine calculation
 *  that is referenced in the book GTCG
 *
 *
 *
 */
bool polyPolyInterSect(Face *p, Face *q, vector<Face*>& interLine){

    return polyPolyInterSect((Polygon*)p->f, (Polygon*)q->f, interLine);

}

bool polyPolyInterSect(Polygon *p, Polygon *q, vector<Face*>& interLine){

    vector<int> dup;
    vector<double> info;

    double sum;
    double myEPSILON = VRML_EPSILON;
    for(int i=0; i<p->nv; i++){
        for(int j=0; j<q->nv; j++){
        	sum  = fabs(p->v[i][0]-q->v[j][0]);
        	sum += fabs(p->v[i][1]-q->v[j][1]);
        	sum += fabs(p->v[i][2]-q->v[j][2]);
            if( fabs(sum) < myEPSILON){
                dup.push_back(i);
                dup.push_back(j);
                break;
            }
        }
    }


    if(dup.size()==2){ // has one common vertex
    	//check the edges that the vertex belongs to
    	//if one of the edge is subset of another one
    	//then return the subset
    	double l[2][2][3], ppp[2][3];
    	for(int i=0; i<3; i++){
    		l[0][0][i] = p->v[ dup[0] ][i]; // line 0, p0
    		l[0][1][i] = p->v[(dup[0]-1+p->nv)%(p->nv) ][i]; // line 0, p1
    		l[1][0][i] = p->v[ dup[0] ][i]; //line 1, p0
    		l[1][1][i] = p->v[(dup[0]+1)%(p->nv) ][i]; //line 1, p1
            ppp[0][i]  = q->v[(dup[1]-1+q->nv)%(q->nv) ][i];
            ppp[1][i]  = q->v[(dup[1]+1)%(q->nv) ][i];

    	}


        bool flg = false;
    	for(int i=0; i<2; i++){
    		for(int j=0; j<2; j++){
    			double r;

    			if( ! pointOnLineSegCheck(ppp[j], l[i], r)){ // not on the line
    				continue;
    			}
                else if (r<0) // on the opposite side of the line segments
                    continue;
                else if (r<1){ // inside the line, should return the starting point to the point
                    l[0][1][0] = ppp[j][0];
                    l[0][1][1] = ppp[j][1];
                    l[0][1][2] = ppp[j][2];
                    flg = true;
                    break;
                }

                else {// out side the line, thus this line is a subset, return this line
                    l[0][1][0] = l[i][1][0];
                    l[0][1][1] = l[i][1][1];
                    l[0][1][2] = l[i][1][2];
                    flg = true;
                    break;
                }
    		}
            if(flg)
                break;
    	}
        if(flg){

            info.push_back(l[0][0][0]);
            info.push_back(l[0][0][1]);
            info.push_back(l[0][0][2]);
            info.push_back(l[0][1][0]);
            info.push_back(l[0][1][1]);
            info.push_back(l[0][1][2]);
            info.push_back(0);
            info.push_back(0);
            info.push_back(0);
            info.push_back(0);
            Face* inl = new Face(2, info);
            interLine.push_back(inl);
            return true;
        }
        else
        	return false;
    }
    if(dup.size()==4){
        info.push_back(p->v[dup[0]][0]);
        info.push_back(p->v[dup[0]][1]);
        info.push_back(p->v[dup[0]][2]);
        info.push_back(p->v[dup[2]][0]);
        info.push_back(p->v[dup[2]][1]);
        info.push_back(p->v[dup[2]][2]);
        info.push_back(0);
        info.push_back(0);
        info.push_back(0);
        info.push_back(0);
        Face* inl = new Face(2, info);
        interLine.push_back(inl);
        return true;
    }

    if(dup.size()>4){
        vector<double> pnts;
        for(unsigned int i=0; i<dup.size(); i+=2){
            pnts.push_back( p->v[dup[i]][0]);
            pnts.push_back( p->v[dup[i]][1]);
            pnts.push_back( p->v[dup[i]][2]);
        }
        int end[2];
        if(checkColiner(pnts, end)){
            vector<double> info;
            info.push_back(p->v[end[0]][0]);
            info.push_back(p->v[end[0]][1]);
            info.push_back(p->v[end[0]][2]);
            info.push_back(p->v[end[1]][0]);
            info.push_back(p->v[end[1]][1]);
            info.push_back(p->v[end[1]][2]);
            info.push_back(0);
            info.push_back(0);
            info.push_back(0);
            info.push_back(0);
            Face* inl = new Face(2, info);
            interLine.push_back(inl);
            return true;
        }
    }

 /*   if(dup.size()== 2){
    	double *np = p->v[ (dup[0]+1)%(p->nv) ];
    	if()
    }
*/

    //perform the generic interSectLine calculation, from here:

    //find the direction of the interSectLine between the two plane
    //that the two polygon sit on
    double dire[3];
    CrossNormalized(p->n, q->n, dire);

    if(Dot(dire, dire) <myEPSILON){
		
		//the two plane is parallel to each other
    	//check if there are common "edges" of the two polygon
    	//the assumption is that polygons only intersect on edges

    	//checking logic
    	//both of the end point should be on the line
    	//determine the intersect points
    	//if not in [0,1], then replace with 0 (case<0) or 1 (case >1)
    	//then return the intersect points
        bool flg = false;
        double pl[2][3], qp[3], rr, inter[2][3];

    	for(int i=0; i<p->nv; i++){
    		pl[0][0] = p->v[i][0]; pl[0][1] = p->v[i][1]; pl[0][2] = p->v[i][2];
    		pl[1][0] = p->v[(i+1)%p->nv][0]; pl[1][1] = p->v[(i+1)%p->nv][1]; pl[1][2] = p->v[(i+1)%p->nv][2];
 //   		qp[0] = q->v[0][0]; qp[1] = q->v[0][1]; qp[2] = q->v[0][2];
	//		ret.push_back (pointOnLineSegCheck(qp, pl, rr));
		//	r.push_back(rr);
			vector<bool> ret;
			vector<double> r;

    		for(int j=0; j<q->nv; j++){
    			
        		qp[0] = q->v[j][0]; qp[1] = q->v[j][1]; qp[2] = q->v[j][2];
				ret.push_back(pointOnLineSegCheck(qp, pl, rr));
				r.push_back(rr);
    			if(ret.size()>1 && ret[j] && ret[(j-1)]){ 
					// both of the points are on the line
    				// then when both points are outside, then the two can not be on the same side
    				// that is r[0] and r[1] can not be >1 at the same time, or <0 at the same time

					int p1=j, p2=j-1;
    				if( ( r[p1]<0 && r[p2]<0 ) || ( r[p1]>1 && r[p2]>1 ) )
    					continue;
					if(r[p1]<0) r[p1] =0;
					if(r[p1]>1) r[p1] =1;
					if(r[p2]<0) r[p2] =0;
					if(r[p2]>1) r[p2] =1;
					
					for(int n=0; n<3; n++){
						inter[0][n] = (1-r[p1])*pl[0][n]+r[p1]*pl[1][n];
						inter[1][n] = (1-r[p2])*pl[0][n]+r[p2]*pl[1][n];
					}

	/*				for(int m=0; m<2; m++){
						if(r[m]<0)
							r[m] = 0;
						else if(r[m]>1)
							r[m] = 1;
						for(int n=0; n<3; n++){
							inter[m][n] = (1-r[m])*pl[0][n]+r[m]*pl[1][n];
						}
					}*/
					flg = true;
					break;
    			}
    		}
			if(!flg && ret[0] && ret[q->nv-1] ){
			
				int p1=0, p2=q->nv-1;
    			if( ( r[p1]<0 && r[p2]<0 ) || ( r[p1]>1 && r[p2]>1 ) )
    				continue;
				if(r[p1]<0) r[p1] =0;
				if(r[p1]>1) r[p1] =1;
				if(r[p2]<0) r[p2] =0;
				if(r[p2]>1) r[p2] =1;
					
				for(int n=0; n<3; n++){
					inter[0][n] = (1-r[p1])*pl[0][n]+r[p1]*pl[1][n];
					inter[1][n] = (1-r[p2])*pl[0][n]+r[p2]*pl[1][n];
				}
				flg = true;
				break;
				
			}
			if(flg)
               break;
    	}
        if(flg){

            info.push_back(pl[0][0]);
            info.push_back(pl[0][1]);
            info.push_back(pl[0][2]);
            info.push_back(pl[1][0]);
            info.push_back(pl[1][1]);
            info.push_back(pl[1][2]);
            info.push_back(0);
            info.push_back(0);
            info.push_back(0);
            info.push_back(0);
            Face* inl = new Face(2, info);
            interLine.push_back(inl);
            return true;
        }
        else
        	return false;
    }



    double s1=- p->func[3], s2= - q->func[3];
    double n1n2dot = Dot(p->n, q->n);
    double n1normsqr = Dot(p->n, p->n);
    double n2normsqr = Dot(q->n, q->n);
    double aDenorm = (n1n2dot*n1n2dot - n1normsqr*n2normsqr);
    double bDenorm = (n1n2dot*n1n2dot - n1normsqr*n2normsqr);
    if(fabs(aDenorm)<myEPSILON || fabs(bDenorm)<myEPSILON )
    	return false;
    double a = (s2*n1n2dot - s1 *n2normsqr)/aDenorm;
    double b = (s1*n1n2dot - s2 *n1normsqr)/bDenorm;
    double linePnt[3];
    linePnt[0] = a*p->n[0] + b*q->n[0];
    linePnt[1] = a*p->n[1] + b*q->n[1];
    linePnt[2] = a*p->n[2] + b*q->n[2];


    //then this function will iterate through all the edges,
    //if there are intersect line between these two polygon,
    //then, this line must pass through both the polygon
    //else there is no intersect line between the two polygon
    //The above functionality is performed by the following function
    vector<double> inPnts, inPnt1, inPnt2;
    if(!findIntersectSegCoplanerLinePoly(p, dire, linePnt, inPnt1))
        return false;
	if(!findIntersectSegCoplanerLinePoly(q, dire, linePnt, inPnt2))
		return false;

	if(inPnt1.size()==3)
		return false;
	else if(inPnt1.size()==3)
		return false;
	else{
		inPnts = inPnt1;
		for(int jj=0; jj<inPnt2.size(); jj++)
			inPnts.push_back(inPnt2[jj]);
	}
    int end[2];
    if(!checkColiner(inPnts, end)){
    	return false;
//            fprintf(stderr, "Error in polyPolyInterSect: points return from findIntersectSegCoplanerLinePoly for the two polygon is not on the same line.");
 //           throw StoppingException("\n");
    }
    if(! linesegLineSegIntersectCheck( inPnts ) )
    	return false;


    info.push_back(inPnts[end[0]*3+0]);
    info.push_back(inPnts[end[0]*3+1]);
    info.push_back(inPnts[end[0]*3+2]);
    info.push_back(inPnts[end[1]*3+0]);
    info.push_back(inPnts[end[1]*3+1]);
    info.push_back(inPnts[end[1]*3+2]);
    info.push_back(0);
    info.push_back(0);
    info.push_back(0);
    info.push_back(0);
    Face* inl = new Face(2, info);
    interLine.push_back(inl);

    return true;

}



/**
 *  This function will check the given list of points is on
 *  the same line or not
 *  if on the same line will also return the shortest line segment
 *  of the line, that contain all the points
 *  (which are the ending points of the list of points)
 *
 *
 *
 */
bool checkColiner(vector<double>& pnts, int end[2]){

	int size = pnts.size()/3;
	double (*pp)[3] = new double[size][3];

	for(int i=0; i<size; i++){
		pp[i][0] = pnts[i*3];
		pp[i][1] = pnts[i*3+1];
		pp[i][2] = pnts[i*3+2];
	}

	bool ret = checkColiner(pp, size, end);

	delete(pp);

	return ret;
}

bool checkColiner(double (*pnts)[3], int nv, int end[2]){
    double dire[3];
    double myEPSILON = EPSILON;
	if(nv==2)
		return true;
    if(nv<2){
		printf("Error: in checkColiner,number of points less than 2, nv=%d.", nv);
        throw StoppingException("\n");
    }
    dire[0] = pnts[1][0] - pnts[0][0];
    dire[1] = pnts[1][1] - pnts[0][1];
    dire[2] = pnts[1][2] - pnts[0][2];
    double t[3];
    end[0] = 0; end[1] = 1;
    for(int i=2; i<nv; i++){
        t[0] = (dire[0]==0) ? 0 : (pnts[i][0] - pnts[end[0]][0])/dire[0];
        t[1] = (dire[1]==0) ? 0 : (pnts[i][1] - pnts[end[0]][1])/dire[1];
        t[2] = (dire[2]==0) ? 0 : (pnts[i][2] - pnts[end[0]][2])/dire[2];

        double diff[3];
        diff[0] = ( fabs(t[0])<myEPSILON || fabs(t[1])<myEPSILON)? 0 : fabs((t[1]-t[0]));
        diff[1] = ( fabs(t[1])<myEPSILON || fabs(t[2])<myEPSILON)? 0 : fabs((t[2]-t[1]));
        diff[2] = ( fabs(t[2])<myEPSILON || fabs(t[0])<myEPSILON)? 0 : fabs((t[0]-t[2]));
        if( diff[0] > myEPSILON
                || diff[1] > myEPSILON
                || diff[2] > myEPSILON )// not on the line
            return false;

        if(t[0]<0) // to the " lower left" of the current line segment
            end[0] = i;
        else if(t[0]>1) // to the "upper right" of the current line segment
            end[1] = i;
        dire[0] = pnts[end[0]][0] - pnts[end[1]][0];
        dire[1] = pnts[end[0]][1] - pnts[end[1]][1];
        dire[2] = pnts[end[0]][2] - pnts[end[1]][2];
        
    }
    return true;
}


/**
 *  This function will check weather the given list of three points are on
 *  the same line or not
 *
 */

bool triangleCheckColiner(vector<double>& pnts){
	    
	double dire1[3], dire2[3], cross[3];
    double myEPSILON = EPSILON;
    int nv = pnts.size()/3;
    if(nv!=3){
		printf("Error: in triangleCheckColiner,number of points not equal to 3, nv=%d.", nv);
        throw StoppingException("\n");
    }

    dire1[0] = pnts[3] - pnts[0];
    dire1[1] = pnts[4] - pnts[1];
    dire1[2] = pnts[5] - pnts[2];

    dire2[0] = pnts[6] - pnts[0];
    dire2[1] = pnts[7] - pnts[1];
    dire2[2] = pnts[8] - pnts[2];


	Cross(dire1, dire2, cross);

	double sum = fabs(cross[0]) + fabs(cross[1]) + fabs(cross[2]);

	if(sum<myEPSILON)
		return true;
	else 
		return false;

}

bool linesegLineSegIntersectCheck(vector<double>& pnts){


    double dire[3];
    double myEPSILON = EPSILON;
    int nv = pnts.size()/3;
    if(nv<2){
		printf("Error: in linesegLineSegIntersectCheck,number of points less than 2, nv=%d.", nv);
        throw StoppingException("\n");
    }

    dire[0] = pnts[3] - pnts[0];
    dire[1] = pnts[4] - pnts[1];
    dire[2] = pnts[5] - pnts[2];
    double t[3];
    int end[2];
    end[0] = 0; end[1] = 1;
	if(nv==2){
		return true;
	}

    for(int i=2; i<nv; i++){
        t[0] = (dire[0]==0) ? 0 : (pnts[i*3+0] - pnts[end[0]*3+0])/dire[0];
        t[1] = (dire[1]==0) ? 0 : (pnts[i*3+1] - pnts[end[0]*3+1])/dire[1];
        t[2] = (dire[2]==0) ? 0 : (pnts[i*3+2] - pnts[end[0]*3+2])/dire[2];

        double diff[3];
        diff[0] = ( fabs(t[0])<myEPSILON || fabs(t[1])<myEPSILON)? 0 : fabs((t[1]-t[0]));
        diff[1] = ( fabs(t[1])<myEPSILON || fabs(t[2])<myEPSILON)? 0 : fabs((t[2]-t[1]));
        diff[2] = ( fabs(t[2])<myEPSILON || fabs(t[0])<myEPSILON)? 0 : fabs((t[0]-t[2]));
        if( diff[0] > myEPSILON
                || diff[1] > myEPSILON
                || diff[2] > myEPSILON )// not on the line
            return false;
        if(t[0]<1&&t[0]>0)
        	return true;
    }

    return false;


}



bool findIntersectSegCoplanerLinePoly(Polygon* poly,
          double *dire, double *p, vector<double>& interPnt ){

    int nv = poly->nv;

    double myEPSILON = EPSILON;

    int numP;

    int code;

    int cnt = 0;

	for(int i=0; i<nv; i++){

        lineLineSegInterSectCheck(p, dire, poly->v[i], poly->v[(i+1)%nv], interPnt, code);
        if(code==0)//there is no intersect line between the line and the line segment
            continue;
        else if(code==1){//there is one intersect line between the line and the line segment, and the interPnt is updated with the coordinates of the intersection point
        	numP = interPnt.size()/3;
        	for(int j=0; j<numP-1; j++){
        		double tmp = interPnt[j*3] - interPnt[(numP-1)*3];
        		tmp += interPnt[j*3+1] - interPnt[(numP-1)*3+1];
        		tmp += interPnt[j*3+2] - interPnt[(numP-1)*3+2];
        		if(fabs(tmp) < myEPSILON ){
        			interPnt.erase(interPnt.end()-1);
        			interPnt.erase(interPnt.end()-1);
        			interPnt.erase(interPnt.end()-1);
        			cnt--;
        			numP = interPnt.size()/3;
        		}

            }


        }
        else if(code==2){// the line segment is on the line, then the segment is the intersect line segment, should add two points

        	for(int k=1; k>=0; k--){//check these points are not added before
            	numP = interPnt.size()/3;
 				for(int j=0; j<numP-1-k; j++){
					double tmp = interPnt[j*3] - interPnt[(numP-1-k)*3];
					tmp += interPnt[j*3+1] - interPnt[(numP-1-k)*3+1];
					tmp += interPnt[j*3+2] - interPnt[(numP-1-k)*3+2];
					if( fabs(tmp) < myEPSILON ){
						int srt = (numP-1-k)*3;
						int end = srt+3;
//						cout<<"points before erase: "<<endl;
//						for(int ll=0; ll<interPnt.size(); ll++){
//							cout<<interPnt[ll]<<"\t";
//						}
//						cout<<endl;
		    			interPnt.erase(interPnt.begin()+ srt, interPnt.begin()+ end);
//						cout<<"points after erase: "<<endl;
//						for(int ll=0; ll<interPnt.size(); ll++){
//							cout<<interPnt[ll]<<"\t";
//						}
//						cout<<endl;
		    			cnt--;
		    			numP = interPnt.size()/3;
		    			break;
					}
				}
        	}
		}
        else{
            throw StoppingException("Error: in findIntersectSegCoplanerLinePoly, unknown code return from lineLineSegInterSectCheck.\n");
        }

    	cnt += code;

    }
    if( ( interPnt.size()%3 ) != 0){
    	throw StoppingException("Error in findIntersectSegCoplanerLinePoly.\n");
    }
    if(cnt==0) {
        return false;
    }
    else
    	return true;
}


/**
 *  This function will calculate the intersection point between a line
 *  and a line segment.
 *
 *  ref GTCG p241
 *
 *  There three conditions, which are represented by the return code:
 *  code=0: no intersection point
 *  code=1: one intersection point
 *  code=2: line segment is on the line
 */


void  lineLineSegInterSectCheck(double *o, double *dire, double *p1, double *p2, vector<double> &interPnt, int& code){



	normalize(dire);
    double segDire[3];
    segDire[0] = p2[0] - p1[0];
    segDire[1] = p2[1] - p1[1];
    segDire[2] = p2[2] - p1[2];

    normalize(segDire);
    double myEPSILON = 0.01;
    
	double kross = dire[0]*segDire[1] - dire[1]*segDire[0];
	double sqrKross = kross*kross;
	double sqrLen0 = Dot(dire, dire);
	double sqrLen1 = Dot(segDire, segDire);
	double s;
	

    if( sqrKross < myEPSILON*sqrLen0*sqrLen1 ){// the two line is parallel
		code = 0;
		return;
    }
    // not parallel, should check if there are intersection point

	code = 1;
	double E[3] = {p1[0]-o[0], p1[1]-o[1], p1[2]-o[2]};
	s = ( E[0]*segDire[1] - E[1]*segDire[0] )/kross;

    interPnt.push_back( o[0] + s*dire[0] );
    interPnt.push_back( o[1] + s*dire[1] );
    interPnt.push_back( o[2] + s*dire[2] );

	return;

		/*
    double myEPSILON = EPSILON;
    code = 0;

    //get the point to line distance between p1 p2 and the line

	double dist[2], q[3], tTemp;
	dist[0] = pointToLineDistance(p1, o, dire, q, tTemp);
	dist[1] = pointToLineDistance(p2, o, dire, q, tTemp);

	for(int i=0; i<2; i++){
		if( fabs(dist[i]) < myEPSILON ){
			code ++;
			int a = (i+1)%2;
			interPnt.push_back(p1[0]*a + p2[0]*i);
			interPnt.push_back(p1[1]*a + p2[1]*i);
			interPnt.push_back(p1[2]*a + p2[2]*i);
		}
	}
	if( code > 0 )//has intersection pnts, have been found
		return;

	//get the min distance between the two line first
	double s,t;
	double lsDire[3] = {p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]};
	double minDist = LineToLineDistance(o, dire, p1, lsDire, s, t );
	if(fabs(minDist) > myEPSILON){ // no intersection between the two line
		return;
	}
	else if(t>1 || t<0 ){ // there is intersection between the two line, but not on the line segment
		return;
	}
	else {
		interPnt.push_back( (1-t)*p1[0] + t*p2[0] );
		interPnt.push_back( (1-t)*p1[1] + t*p2[1] );
		interPnt.push_back( (1-t)*p1[1] + t*p2[2] );
		code = 1;
		return;
	}

	*/
}


/**
 * This function calculate the distance between two lines
 * Ref: p411
 * @param l the coordinates of pnt on line 1
 * @param ld direction of line 1
 * @param m point on line 2
 * @param md direction of line 2
 * @param s parameter for min distant point on line 1 q1=l+s*ld
 * @param t parameter for min distant point on line 2 q2=m+t*md
 *
 */

double LineToLineDistance(double l[3], double ld[3], double m[3], double md[3], double& s, double & t ){
	if(!l || !ld || !m || !md){
		throw StoppingException("Error in LineToLineDistant, non pointer\n");
	}
	double myEPSILON = EPSILON;

	double u[3] = {l[0]-m[0], l[1]-m[1], l[2]-m[2]};
	double a = Dot(ld, ld);
	double b = Dot(ld, md);
	double c = Dot(md, md);
	double d = Dot(ld, u);
	double e = Dot(md, u);
	double f = Dot(u, u);
	double det = a*c - b*b;

	if( fabs(det) < myEPSILON ){//near parallel
		if(b>c)
			t = d/b;
		else
			t = e/c;
		return -e*t + f;
	}
	else{
		s = ( b*e - c*d )/det;
		t = ( a*e - b*d )/det;
		return s*( a*s - b*t + 2*d ) + t*(-b*s + c*t - 2*e) + f;
	}
}


/**
 *  This function will calculate the intersect arc between the sphere
 *  and the polygon
 *
 *  intersect conditions includes:
 *      >no intersect: will return false;
 *      >intersects: will return true
 *          intersection conditions are:
 *              ** a circle inside the polygon
 *              ** the polygon is inside the circle, will return false
 *                 since, there is no intersect between the polygon
 *                 and the sphere, the polygon is inside the sphere
 *                 how to quickly check this?
 *                 calculate the dist between all the vertices and the
 *                 center of the circle
 *              ** there are intersection arcs, not necessarily to be one
 *                 will return all the arcs
 *
 *
 */

bool polySphereInterSect(Face *poly, Face *sph, vector<Face*>& interLine){

	Polygon *p = (Polygon*) poly;
	Sphere  *s = (Sphere*) sph;


    double centerPlaneDist = s->center[0] * p->func[0] +
                             s->center[1] * p->func[1] +
                             s->center[2] * p->func[2] + p->func[3];
    if(centerPlaneDist > s->radius)// no intersection
        return false;
    //calculate the intersection circle between the plane and the sphere
    double rrSqr = s->radius*s->radius + centerPlaneDist*centerPlaneDist;
    double rr = sqrt(rrSqr);
    double myEPSILON = EPSILON;
    double cc[3];
    cc[0] = s->center[0] - centerPlaneDist* p->n[0];
    cc[1] = s->center[1] - centerPlaneDist* p->n[1];
    cc[2] = s->center[2] - centerPlaneDist* p->n[2];

    // will calculate all the dist between vertices and center
    vector<double> dist;
    double *vv;
    int flg=0;
    int nv = p->nv;
    for(int i=0; i<nv; i++){
        vv = p->v[i];
        dist[i] = pointToPointDistanceSqr(cc, vv);
        if(dist[i]>rrSqr)flg=1; //for poly in circle check
       // dist[i] = sqrt(dist[i]);
    }
    if(flg==1)// all vertices is inside the circle
              // there must be no intersection between the polygon and
              // the circle, thus return false;
        return false;

    //try to find all the intersect arc/circles
    //will iterate through all the vertices of the polygon
    //find "sign change edge", which is where the circle intersect
    //with the polygon
    //then will find a point on the circle and between the two intersect
    //points, do a inPolygon check, if in polygon then this part of
    //the arc is the enclosed segment of arc

    double pre, nxt;
    vector<double> interPnt;
    pre = dist[0] - rrSqr;
    if(fabs(pre)<myEPSILON){
    	interPnt.push_back( p->v[0][0]);
    	interPnt.push_back( p->v[0][1]);
    	interPnt.push_back( p->v[0][2]);
    }
    for(int i=1; i<nv; i++){
        pre = nxt;
        nxt = dist[i]- rrSqr;
        if(fabs(nxt)<myEPSILON){ // on the circle
        	interPnt.push_back(p->v[i][0]);
        	interPnt.push_back(p->v[i][1]);
        	interPnt.push_back(p->v[i][2]);
            if(fabs(pre)<myEPSILON)
                continue;
            if(pre>0){ //there may be one intersection on the line seg
                double t[2];
                double *pp, *qq;
                pp=p->v[(i-1+nv)%nv];
                qq=p->v[i];
                int cnt = circleLineSegSamePlaneInterPnt(cc, rr,
                        p->n, pp, qq, t);
                if(cnt<1)
                    continue;
                else{
                    double tt;
                    if((t[0]-1)>=myEPSILON)
                        tt = t[0];
                    else
                        tt= t[1];
                    double inp[3] = {p->v[(i-1+nv)%nv][0]*(1-tt) + tt*p->v[i][0],
                    		p->v[(i-1+nv)%nv][1]*(1-tt) + tt*p->v[i][1],
                    		p->v[(i-1+nv)%nv][2]*(1-tt) + tt*p->v[i][2]
                    };
                    interPnt.push_back(inp[0]);
                    interPnt.push_back(inp[1]);
                    interPnt.push_back(inp[2]);
                }
            }
        }
        if(pre<0 && nxt<0) //no intersections
            continue;
        if(pre>0 && nxt>0){// there may be one intersection on the line seg

			double t[2];
			double *pp, *qq;
            pp=p->v[(i-1+nv)%nv];
            qq=p->v[i];
			int cnt = circleLineSegSamePlaneInterPnt(cc, rr,
					p->n, pp, qq, t);
			if(cnt<0)
				continue;
			else{
				double tt;
				if( t[0]<1 && t[0] >0)
					tt = t[0];
				else
					tt= t[1];
                double inp[3] = {p->v[(i-1+nv)%nv][0]*(1-tt) + tt*p->v[i][0],
                		p->v[(i-1+nv)%nv][1]*(1-tt) + tt*p->v[i][1],
                		p->v[(i-1+nv)%nv][2]*(1-tt) + tt*p->v[i][2]
                };
                interPnt.push_back( inp[0]);
                interPnt.push_back( inp[1]);
                interPnt.push_back( inp[2]);
			}
        }
    }



    // will then find the enclosed arcs
    // will use a third pnt to test


    int nn = interPnt.size();
    if( nn < 3){
        return false;}
    else if( nn == 3 ){
        vector<double> info;
        info.push_back(cc[0]);
        info.push_back(cc[1]);
        info.push_back(cc[2]);
        info.push_back(rr);
        info.push_back(interPnt[0]);
        info.push_back(interPnt[1]);
        info.push_back(interPnt[2]);
        info.push_back(0);
   }
    else{
        double mid[3], amid[3];
        for(int i=0; i<nn-1; i++){
            mid[0] = (interPnt[i*3+0] + interPnt[((i+1)%nn)*3+0]) /2;
            mid[1] = (interPnt[i*3+1] + interPnt[((i+1)%nn)*3+1]) /2;
            mid[2] = (interPnt[i*3+2] + interPnt[((i+1)%nn)*3+2]) /2;
            double dd = pointToPointDistance(mid, cc);
            double ttt= rr/dd;
            amid[0] = cc[0]*(1-ttt) + ttt*mid[0];
            amid[0] = cc[0]*(1-ttt) + ttt*mid[0];
            amid[0] = cc[0]*(1-ttt) + ttt*mid[0];
    //TODO
        }
    }




    throw StoppingException("Error: in polySphereInterSect, not known situation in polySphereInterSect.\n");

}


/**
 *  This function will find the intersection pnt between a colinar circle
 *  and line segment
 *
 *  @param c the coordinates of the center of the circle
 *  @param r the radius of the circle
 *  @param norm the normal director of the plane that the circle and line
 *              segment shares
 *  @param p the first point of the line segment
 *  @param q the second point of the line segment
 *  @param t the parameter of the intersect points
 *  @return number of intersection points
 *
 */

int circleLineSegSamePlaneInterPnt(double *center, double r, double *norm,
        double *p, double *q, double t[2]){

    double dire[3] = {q[0]-p[0], q[1]-p[1], q[2]-p[2]};
    double myEPSILON = EPSILON;
    if(fabs(Dot(norm, dire))>myEPSILON)
        throw StoppingException("Error: in circleLineSegSamePlaneInterPnt, circle line segment not on the same plane.\n");
    double a, b, c, sqr;
    double pc[3] = {p[0]-center[0], p[1]-center[1], p[2]-center[2]};
    b= 2*Dot(dire, pc);
    c= Dot(pc, pc)-r;
    a= Dot(dire, dire);
    sqr = b*b - 4*a*c;
    if(sqr<0) {
        t[0] = t[1] = 0.5;
        return 0;

    }
    else {
        t[0] = (-b - sqrt(sqr))/2/a;
        t[1] = (-b + sqrt(sqr))/2/a;
    }
    int cnt=0;
    if(t[0] >0 && t[0]<1) cnt++;
    if(t[1] >0 && t[1]<1) cnt++;
    return cnt;
}


bool sphereSphereInterSect(Face *p, Face *q, vector<Face*>& interLine){

	Sphere *s1 = (Sphere*) p->f;
	Sphere *s2 = (Sphere*) q->f;

	double cDist = pointToPointDistanceSqr(s1->center, s2->center);
	double con[3] = {
			s2->center[0]-s1->center[0],
			s2->center[1]-s1->center[1],
			s2->center[2]-s1->center[2],
	};
	if(sqrt(cDist)> s1->radius+s2->radius)
		return false;

	double r1 = s1->radius*s1->radius;
	double r2 = s2->radius*s2->radius;
	double t = ( 1 + (r1 - r2)/ cDist)/2;
	double cen[3] = {
			s1->center[0]+ t*con[0],
			s1->center[1]+ t*con[1],
			s1->center[2]+ t*con[2],
	};

	double r = sqrt(r1 - cDist*t*t);
	vector<double> info;
	info.push_back(cen[0]);
	info.push_back(cen[1]);
	info.push_back(cen[2]);
	info.push_back(cen[0]+r);
	info.push_back(cen[1]);
	info.push_back(cen[2]);
	info.push_back(0);
	info.push_back(0);
	info.push_back(0);
    info.push_back(0);

	Face *nf = new Face(6, info);
	interLine.push_back(nf);

	return true;

}


/**
 * This function will try to find the two intersection point of the line segment and the
 * cube
 *
 * @param line coordinates of the line ending points
 * @param cx coordinates of the eight vertices of the cube
 * @param pnt the coordinates of the returned intersection points
 *
 */
bool getLineSegCubeInterSectPoint(double line[6], double cx[3], double size, double pnt[6]){


	double tN, tF;
    double d[3] = {line[3]-line[0], line[4]-line[1], line[5]-line[2]};

    int numInter = lineSegCubeInterSectCheck(line, d, cx, size, tN, tF);
	if (numInter < 2)
        return false;

    pnt[0] = line[0] + tN*d[0];
    pnt[1] = line[1] + tN*d[1];
    pnt[2] = line[2] + tN*d[2];
    pnt[3] = line[0] + tF*d[0];
    pnt[4] = line[1] + tF*d[1];
    pnt[5] = line[2] + tF*d[2];
    return true;




	/*
	 * Old method seems not general enough


	double dire[3]={
			line[3] - line[0],
			line[4] - line[1],
			line[5] - line[2],
	};
	double coeff[6][4]={
			{ 1,  0,  0, -cx[0]},
			{ 0,  1,  0, -cx[1]},
			{ 0,  0,  1, -cx[2]},
			{-1,  0,  0,  cx[0]+size},
			{ 0, -1,  0,  cx[1]+size},
			{ 0,  0, -1,  cx[2]+size},
	};

	double curpnt[3], range[2][3];
	double t;
	int offsetFlg, cnt=0;
	for(int j=0; j<6; j++){
		if(!getLinePlaneIntersectPoint(line, dire, coeff[j], curpnt, t))
			continue;
		if(j>2)
			offsetFlg = -1;
		else
			offsetFlg = 1;
		range[0][0] = (j<3)? cx[0]:cx[0]+size;
		range[0][1] = (j<3)? cx[1]:cx[1]+size;
		range[0][2] = (j<3)? cx[2]:cx[2]+size;
		range[1][0] = (((int)(coeff[j][0]+1))%2)*size*offsetFlg;
		range[1][1] = (((int)(coeff[j][1]+1))%2)*size*offsetFlg;
		range[1][2] = (((int)(coeff[j][2]+1))%2)*size*offsetFlg;
		double tt=0.5;
		if(coeff[j][0] == 0){
			tt = (curpnt[0] - range[0][0]) / range[1][0];
			if( tt > 1 || tt < 0 )
				continue;
		}
		if(coeff[j][1] == 0){
			tt = (curpnt[1] - range[0][1]) / range[1][1];
			if( tt > 1 || tt < 0 )
				continue;
		}
		if(coeff[j][2] == 0){
			tt = (curpnt[2] - range[0][2]) / range[1][2];
			if( tt > 1 || tt < 0 )
				continue;
		}
		if(cnt>1)
			throw StoppingException("Error: in getLineSegCubeInterSectPoint, number of intersection point"
					                " greater than 2.\n");
		pnt[cnt*3+0] = curpnt[0];
		pnt[cnt*3+1] = curpnt[1];
		pnt[cnt*3+2] = curpnt[2];
		cnt++;
	}

	if(cnt==2)
		return true;
	else
		return false;
*/
}



/**
 * This function will try to find the two intersection point of the arc and the cube
 *
 * @param arc pointer to the the Arc object
 * @param cx the coordinates of the eight vertices of the cube
 * @param pnt the coordinates of the returned intersection points
 *
 */
bool getArcCubeInterSectPoint(Arc* arc, double cx[3], double size, double pnt[6]){

	double func[4];
	func[0] = arc->n[0];
	func[1] = arc->n[1];
	func[2] = arc->n[2];
	func[3] = -(arc->n[0]*arc->end[0] + arc->n[1]*arc->end[1] + arc->n[2]*arc->end[2]);

	double corner[6][3];
	int cnt=0;
	double vals[8];
	double xx[8][3]={
			{cx[0], 		cx[1], 			cx[2]},
			{cx[0]+size, 	cx[1], 			cx[2]},
			{cx[0]+size, 	cx[1]+size, 	cx[2]},
			{cx[0], 		cx[1]+size, 	cx[2]},
			{cx[0], 		cx[1], 			cx[2]+size},
			{cx[0]+size, 	cx[1], 			cx[2]+size},
			{cx[0]+size, 	cx[1]+size, 	cx[2]+size},
			{cx[0], 		cx[1]+size, 	cx[2]+size},
	};
	for(int i=0; i<8; i++)
		vals[i] = Dot(xx[i], arc->n) + func[3];

	//edge pairs are: (0,1), (1,2), (2,3), (3,0),   (i=0..3, j=0)
 	//                (0,4), (1,5), (2,6), (3,7),   (i=0..3, j=1)
	//                (4,5), (5,6), (6,7), (7,4),   (i=0..3, j=2)
	int pre, nxt;
	for(int j=0; j<3; j++){
		for(int i=0; i<4; i++){
			pre = i+ (j/2)*4;
			nxt = (i+1-j%2)%4 + 4*((j+1)/2);
			if(vals[pre]*vals[nxt]<0){
				double t = fabs(vals[pre])/fabs(vals[pre]-vals[nxt]);
				corner[cnt][0] = xx[pre][0] + t*(xx[nxt][0] - xx[pre][0]);
				corner[cnt][1] = xx[pre][1] + t*(xx[nxt][1] - xx[pre][1]);
				corner[cnt++][2] = xx[pre][2] + t*(xx[nxt][2] - xx[pre][2]);
			}
		}
	}

	if(cnt<3)
		return false;
	double mid[3]= {0};
	for(int i=0; i<cnt; i++){
		mid[0] += corner[i][0];
		mid[1] += corner[i][1];
		mid[2] += corner[i][2];
	}

	double a = pointToPointDistance(mid, arc->center);
	a = arc->radius/a;
	pnt[0] = pnt[3] = (1-a)*arc->center[0] + a*mid[0];
	pnt[1] = pnt[4] = (1-a)*arc->center[1] + a*mid[1];
	pnt[2] = pnt[5] = (1-a)*arc->center[2] + a*mid[2];

	return true;
}


double pointToPolygonPatchDistancePtrUsage(double *p, void* q){

	PolygonPatch* pp = (PolygonPatch*) q;

	double dist, temp;
	if ( pp->bo == 1 ) { // boolean union
		                 // distance should be the min from all the polys
		dist = pointToPolygonDistance( p, pp->polys[0] );
		for(int i=1; i<pp->np; i++) {
			temp = pointToPolygonDistance(p, pp->polys[i] );
			if( fabs(temp) < fabs(dist) )
				dist = temp;
		}
	}

	else if (pp->bo == -1 ) { // boolean subtraction
		                      // find the min distance from the first poly
		                      // also find the shortest dist point on the first poly
		                      // if it is on the subtract surface then have to 
		                      // recalculate the min distance
		                      // the new shortest dist line, 
		                      // the old shortest dist line,
		                      // and the shortest dist from the point to the poly
		                      // will form a perpendicular triangle

		double pPrime[3];
		dist = pointToPolygonDistance(p, pp->polys[0], pPrime);

        for(int l=1; l<pp->np; l++){


            Polygon* curP = pp->polys[l];
			if(pointInCoplanarPolygonCheck3D(curP, pPrime)) {
				
				double distPrime = pointToPolygonDistance(pPrime, curP);
    	        if(distPrime==-1) // on this polygon
                    continue;
                dist = sqrt( dist*dist + distPrime*distPrime );
                break;                    
			}
        }
    }
    else {

       throw StoppingException("Error in pointToPolygonPatchDistancePtrUsage: Unknown boolean operation for polygonPatch.\n");
    }

    return dist;

}


double polygonToPolygonPatchDistancePtrUsage(void* p, void* q){


	Polygon* pp = (Polygon*) p;
    PolygonPatch* qq = (PolygonPatch*) q;

	double dist, temp;
	if ( qq->bo == 1 ) { // boolean union
		                 // distance should be the min from all the polys
		dist = polygonToPolygonDistance( pp, qq->polys[0] );
		for(int i=1; i<qq->np; i++) {
			temp = polygonToPolygonDistance(pp, qq->polys[i] );
			if( temp < dist )
				dist = temp;
		}
	}

	else if (qq->bo == -1 ) { // boolean subtraction
		                      // find the min distance from the first poly
		                      // also find the shortest dist point on the first poly
		                      // if it is on the subtract surface then have to 
		                      // recalculate the min distance
                              // the new shortest distance point should be on the
                              // the subtraction polygon's edges, 
                              // then have to iterate through all the edges,
                              // find the minimum distance 

        //TODO
    }

    else {

        throw StoppingException("Error in polygonToPolygonPatchDistancePtrUsage: Unknown boolean operation for polygonPatch.\n");
    }

    return dist;

}



double sphereToPolygonPatchDistancePtrUsage(void *p, void *q){

	Sphere* pp = (Sphere*) p;
    PolygonPatch* qq = (PolygonPatch*) q;

    return pointToPolygonPatchDistancePtrUsage(pp->center, qq) - pp->radius;

}


int getNumIntPntRayPolygonPatchPtrUsage(double *x, double *dire, void *f){

    PolygonPatch* p = (PolygonPatch*) f;

    int cnt = 0;

	if ( p->bo == 1 ) { // boolean union
		                 // distance should be the min from all the polys
		for(int i=0; i<p->np; i++) {
            cnt += getNumIntPntRayPolygonPtrUsage(x, dire, p->polys[i]);
		}
	}

	else if (p->bo == -1 ) { // boolean subtraction
                              // find the first and then add to cnt
		                      // if no intersection with the first then
		                      // return 0
		                      // else
		                      // iterate through the rest of the polys
                              // if intersect with one of the subtraction
		                      // polys then, return 0;

		cnt =getNumIntPntRayPolygonPtrUsage(x, dire, p->polys[0]);
 
		if ( cnt == 0 )
			return 0;

        for(int i=1; i<p->np; i++){
            cnt += getNumIntPntRayPolygonPtrUsage(x, dire, p->polys[i]);
            if( cnt == 2 ) // also intersect the subtraction poly
				           // return 0;
                return 0;
        }

    }

    else {

        throw StoppingException("Error in sphereToPolygonPatchDistancePtrUsage: Unknown boolean operation for polygonPatch.\n");
    }

    return cnt;


}


bool project2PolygonPatchPtrUsage(double *x, double *p, void *poly){

    PolygonPatch* pp = (PolygonPatch*) poly;

	bool ret= false;

	if ( pp->bo == 1 ) { // boolean union
		                // iterate through all the polys
		                // return if any of the projection return true
		for(int i=0; i<pp->np; i++) {
            if( project2PolygonPtrUsage(x, p, pp->polys[i])){
				 ret = true;
				 break;
			}
		}
	}

	else if ( pp->bo == -1 ) { // boolean subtraction
							  // project to first poly
		                      // if return false from projection
		                      // then return false
		                      // else check the projection point is on 
		                      // either of the subtraction polygon
		                      // if on either of the subtraction polygon
		                      // then return false
		                      // else return true;

		if(project2PolygonPtrUsage(x, p, pp->polys[0])){
		
			ret = true;
			for(int i=1; i<pp->np; i++) {
				
				if( pointInCoplanarPolygonCheck3D(pp->polys[i], p)) {
					ret = false;
					break;
				}
			}
		}
		else 
			ret = false; 
	}

	else {

        throw StoppingException("Error in project2PolygonPatchPtrUsage: Unknown boolean operation for polygonPatch.\n");
    }

    return ret;

    

}


bool polyPatchPolyInterSect(Face* polyPatch, Face* poly, vector<Face*>& lines){

    return polyPolyPatchInterSect(poly, polyPatch, lines);
}

bool polyPolyPatchInterSect(Face* poly, Face* polyPatch, vector<Face*>& lines){
    Polygon * p = (Polygon*) poly->f;
    PolygonPatch * pp = (PolygonPatch*) polyPatch->f;

    return polyPolyPatchInterSect(p, pp, lines);
}



bool polyPolyPatchInterSect(Polygon* p, PolygonPatch* pp, vector<Face*>& lines){


    bool ret;

    vector<Face*> temp;

    if( pp->bo == 1){ // boolean union
                      // should stitch all the intersection lines together
                      // if stitch failed, then throw exception

    	int num;
    	bool flg = false;
        for(int i=0; i<pp->np; i++)  {
            ret = polyPolyInterSect(p, pp->polys[i], temp);
            if(ret)
            	flg = ret;
            num = temp.size();
        }

        if(flg && temp.size() > 1)
        	lineStitch(temp);


    }

    else if ( pp->bo == -1 ) { // boolean subtraction 
                               // find the first intersect line
                               // and then subtract the intersect lines 
                               // from all the intersection line with the
                               // subtraction polygons


        ret = polyPolyInterSect(p, pp->polys[0], temp);

        if(ret){
			vector<Face*> sub;
			for(int i=1; i<pp->np; i++)
				polyPolyInterSect(p, pp->polys[i], sub);

			if(sub.size()>0)
				lineSubtract(temp, sub);

			for(int i=0; i<(int)sub.size(); i++)
				free(sub[i]);

        }

    }

    else {
        throw StoppingException("Error in polyPolyPatchInterSect: Unknown boolean operation for polygonPatch.\n");

    }


    for(int i=0; i<(int)temp.size(); i++)
        lines.push_back(temp[i]);

    if(temp.size() > 0)
        return true;
    else
        return false;



}


bool polyPatchPolyPatchInterSect(Face* pp1, Face* pp2, vector<Face*>& lines){
    //
    // things are a little complicated for this function

//	cout<<"polyPatchPolyPatchInterSect not implemented yet"<<endl;



	PolygonPatch* p = (PolygonPatch*) pp1->f;
	PolygonPatch* q = (PolygonPatch*) pp2->f;
	if(p->bo != 1 || q->bo != 1){
		throw StoppingException("Error: in polyPatchPolyPatchInterSect, poly is a subsract poly, function "
				"not implemented yet.\n");
	}

	vector<Face*>temp;
	int nl;

	for(int i=0; i< p->np; i++){
		polyPolyPatchInterSect(p->polys[i], q, temp);
		nl = temp.size();

	}

//	lineStitch(temp);

	if(temp.size()>0){
		for(int i=0; i<(int)temp.size(); i++)
			lines.push_back(temp[i]);
		return true;
	}
	else
		return false;

}

bool polyPatchSphereInterSect(Face* p, Face* s, vector<Face*>& lines){

    return spherePolyPatchInterSect(s, p, lines);


}
bool spherePolyPatchInterSect(Face* s, Face* p, vector<Face*>& lines){
    // TODO
    // 
    // if boolean union, stitch all the intersection lines
    //
    // else if boolean subtraction, subtract from the first line 
	cout<<"spherePolyPatchInterSect not implemented yet"<<endl;

	throw StoppingException("\n");

}


void lineStitch( vector<Face*>& line ){

    Polygon *p ,*q;

    double t[2];

    for(int i=0; i< (int)line.size(); i++){

        p = (Polygon*) line[i]->f;

        for(int j=i+1; j< (int)line.size(); j++){
            
            q = (Polygon*) line[j]->f;

            if( pointOnLineSegCheck(q->v[0], p, t[0]) ){ // on the ith line
                
               if( pointOnLineSegCheck(q->v[1], p, t[1]) ) { // both point on the ith line
                                        // replace end point if t or r is less
                                        // than 0 or greater than len
                                        // and have to remove this linesegment
                    for(int k = 0; k<2; k++){
                    
                        if(t[k]<0) {
                            p->v[0][0] = q->v[k][0];
                            p->v[0][1] = q->v[k][1];
                            p->v[0][2] = q->v[k][2];
                        }
                        else if(t[k] > 1){
                            p->v[1][0] = q->v[k][0];
                            p->v[1][1] = q->v[k][1];
                            p->v[1][2] = q->v[k][2];
                        }
                    }

                    line.erase(line.begin() + j);
                }
            }
        }
    }

}


bool pointOnLineSegCheck(double p[3], Polygon *l, double& t){

	if(l->nv != 2){
		throw StoppingException("Error in pointOnLineSegCheck, argument 2 is not line segment.\n");
	}

	return pointOnLineSegCheck (p, l->v, t);



}
bool pointOnLineSegCheck(double p[3], double l[6], double& t){

	double ll[2][3] = { {l[0], l[1], l[2]} , { l[3], l[4], l[5] } };

	return pointOnLineSegCheck(p, ll, t);

}

bool pointOnLineSegCheck(double p[3], double l[2][3], double& t){

	double pvec[3], lvec[3];


	for(int i=0; i<3; i++){
		pvec[i] = p[i] - l[0][i];
		lvec[i] = l[1][i] - l[0][i];
	}

	// if the point is on the line segment
	// the vector vec will be parallel with the direction of the line segment
	// the cross product will be all zero

	double cross[3];
	Cross(pvec, lvec, cross);

	double sum = fabs(cross[0]) + fabs(cross[1]) + fabs(cross[2]);
	double myEPSILON = (double) EPSILON;
	bool ret = false;
	if( sum < myEPSILON ){ // on the line

		for(int i=0; i<3; i++){
			if( lvec[i] != 0 ){
				t = (p[i]-l[0][i])/lvec[i];
				ret = true;
				break;
			}
		}
	}
	return ret;
}


void lineSubtract( vector<Face*>& line, vector<Face*>& sub ){

    Polygon *p ,*q;

    double t[2];


    for(int i=0; i<(int)line.size(); i++){

        p = (Polygon*) line[i]->f;
        if( pointOnLineSegCheck(q->v[0], p, t[0]) ){ // on the ith line
            
        	   if( pointOnLineSegCheck(q->v[1], p, t[1]) ){ // both point on the ith line
                                        // replace end point if t or r is less
                                        // than len or greater than 0

                    for(int k = 0; k<2; k++){

                    
                        if(t[k]>0) {
                            p->v[0][0] = q->v[k][0];
                            p->v[0][1] = q->v[k][1];
                            p->v[0][2] = q->v[k][2];
                        }
                        else if(t[k] < p->radius){
                            p->v[1][0] = q->v[k][0];
                            p->v[1][1] = q->v[k][1];
                            p->v[1][2] = q->v[k][2];
                        }
                    }
                }
            }
        }

}


/**
 * This function is partially based on the method described in GTCG P634
 * For a polygon to intersect with a cube, the most-perpendicular-topolygon
 * diagonal line of the cube will have intersection point inside the polygon
 * 
 *
 *
 */


bool polyCubeInterSectCheck(double cubeBase[3], double size, Face* f){

	if( f->type > 5 || f->type < 2 ){ // polygon
		throw StoppingException("Error in polyCubeInterSectCheck, Face passed in is not a polygon.\n");
	}

	Polygon* p = (Polygon*) f->f;


	return polyCubeInterSectCheck(cubeBase, size, p);



}

bool polyCubeInterSectCheck(double cubeBase[3], double size, Polygon* p){

	//find points for the diagonal line, that is most perpendicular to the polygon
	double diag[2][3];
	for(int i=0; i<3; i++){

		if( p->n[i] > 0 ){
			diag[0][i] = cubeBase[i];
			diag[1][i] = cubeBase[i]+size;
		}
		else{
			diag[0][i] = cubeBase[i]+size;
			diag[1][i] = cubeBase[i];
		}
	}

	if( (Dot(diag[0], p->func) + p->func[3]) > 0)//the diagonal point is on the plane
		return false;
	if( (Dot(diag[1], p->func) + p->func[3]) < 0)//the diagonal point is on the plane
		return false;



	double t;
	linePlaneInterSect(diag, p, t);

	if( t>1 || t<0 ) //the plane does not intersect with diagonal line
		return false;
	//the plane intersect with the cube
	
	double pPrime[3] = { (1-t)*diag[0][0] + t*diag[1][0], 
		                 (1-t)*diag[0][1] + t*diag[1][1], 
		                 (1-t)*diag[0][2] + t*diag[1][2]};

    //check if the diagonal line intersects the polygon
	if ( pointInCoplanarPolygonCheck3D(p, pPrime) )
		return true;

    // the diagonal line does not intersects the polygon
    // now have to check if any of the polygon edges intersects with
    // the cube    
    
    double d[3];

    for(int i=0; i<p->nv; i++) {
        
        d[0] = p->v[(i+1)%(p->nv)][0] - p->v[i][0];
        d[1] = p->v[(i+1)%(p->nv)][1] - p->v[i][1];
        d[2] = p->v[(i+1)%(p->nv)][2] - p->v[i][2];

        double tn, tf;
        int cnt = lineSegCubeInterSectCheck(p->v[i], d, cubeBase, size, tn, tf);
        if ( cnt >0 )
            return true;
        
    }

    return false;

}

/**
  * This function will try to find the intersection point between a line and a plane
  * 
  * Return code:  0:= no intersection
  *               1:= one intersection
  *              -1:= line on the plane
  *
  *
  */
int linePlaneInterSect(double line[2][3], Polygon* p, double& t ){

	if(!line || !p){
		throw StoppingException("Error in linePlaneInterSect, NULL point passed in.\n");
	}
	

	double dire[3] = { line[1][0]-line[0][0], line[1][1]-line[0][1], line[1][2]-line[0][2] };
	double denom = Dot(dire, p->n);
	double myEPSILON = EPSILON;
	if( fabs(denom) <myEPSILON ){ // parallel
		if( fabs(Dot(line[0], p->func) +p->func[3] ) < myEPSILON ) // on the plane
			return -1;
		else
			return 0;
	}
	
	t = -1 * (Dot(line[0], p->func) + p->func[3]) / denom;

	return 1;

}


bool polyPatchCubeInterSectCheck(double *cubeBase, double cubeSize, Face *f){
	
	if( f->type != 0xa ){ // polygon
		throw StoppingException("Error in polyPatchCubeInterSectCheck, Face passed in is not a polygonPatch.\n");
	}

	PolygonPatch* p = (PolygonPatch*) f->f;

	for(int i=0; i<p->np; i++){
		if( polyCubeInterSectCheck(cubeBase, cubeSize, p->polys[i]) )
			return true;
	}

	return false;
	
}


bool pointInCubeCheck(double *p, double *cb, double size){

	for(int i=0; i<3; i++){
		if ( p[i]>cb[i]+size || p[i]<cb[i] )
			return false;
	}

	return true;
}




/**
 * This function calculate the line segment intersection points with a cube
 * 
 * return code: 0:= no intersection
 *              1:= one intersection
 *              2:= two intersection
 *
 *
 */

int lineSegCubeInterSectCheck(double *o, double *d, double *cb, 
        double size, double &tN, double &tF) {

    double myEPSILON = EPSILON;
    tN = -1/EPSILON;  //DBL_MAX
    tF = 1/EPSILON;
    double t0, t1;

    for( int i=0; i<3; i++ ){

        if( fabs(d[i]) < myEPSILON ) { //parallel to plane
            if( o[i]<cb[i] || o[i]>cb[i]+size )
                return 0;
            continue;
        }
        //not parallel to the plane

        t0 = ( cb[i] - o[i] ) / d[i] ;
        t1 = ( cb[i] + size - o[i] ) / d[i];

        // Check ordering
        //
        
        if( t0 > t1 ) {
            //swap
            double temp = t0;
            t0 = t1;
            t1 = temp;
        }

        //compare with current values

        if( t0 > tN) 
            tN = t0;
        if( t1 < tF )
            tF = t1;

        // check if ray missed entirely

        if( tN > tF )
            return 0;
        if( tF < 0 )
            return 0;

    }

    // Box definitely intersected

    if( tN<1 && tF<1 )
    	return 2;
    else if( tN<1 && tF>1)
    	return 1;
    else
    	return 0;

    

}


/**
  * Function to check if the cube and sphere has intersections
  *
  * Method:referene boob GTCG pp645  // this method seems only the the point inside the sphere, but the cube may be 
  *
  * New method is to first get the distance from the center of cube and center of the sphere
  * If the distaance D+/-Rs must falls in the range of the biggest R (Circumscribed circle) and smallest r (Inscribed circle) of the cube
  * 
  *
  */

bool sphereCubeInterSectCheck(double cubeBase[3], double cubeSize, Face* my){
	
	Sphere* s = (Sphere*) my->f;

	double cR = 1.732*cubeSize/2;
	double cr = cubeSize/2;

	double r = s->radius;

	double D = sqrt( ( s->center[0] - (cubeBase[0]+cubeSize/2) ) * ( s->center[0] - (cubeBase[0]+cubeSize/2) ) 
		            +( s->center[1] - (cubeBase[1]+cubeSize/2) ) * ( s->center[1] - (cubeBase[1]+cubeSize/2) ) 
				    +( s->center[2] - (cubeBase[2]+cubeSize/2) ) * ( s->center[2] - (cubeBase[2]+cubeSize/2) ) ); 
	if( D-r > cR) // too far
		return false;

	else if ( r-D > cR ) // cube inside the sphere
		return false;

	else {

		double distSquared = 0;
		for(int i=0; i<3; i++){
			if( s->center[i] < cubeBase[i] ){

				distSquared += (s->center[i] - cubeBase[i] )*(s->center[i] - cubeBase[i] );

			}
			else if( s->center[i] > cubeBase[i]+cubeSize ){

				distSquared += (s->center[i] - ( cubeBase[i] + cubeSize ) )*(s->center[i] - ( cubeBase[i] + cubeSize ) );
			}

		}

		if(distSquared <= (s->radius)*(s->radius) )
			return true;
		else 
			return false;

	}

	return true;
/*
	Sphere* s = (Sphere*) my->f;
	double distSquared = 0;
	for(int i=0; i<3; i++){
		if( s->center[i] < cubeBase[i] ){

			distSquared += (s->center[i] - cubeBase[i] )*(s->center[i] - cubeBase[i] );

		}
		else if( s->center[i] > cubeBase[i]+cubeSize ){

			distSquared += (s->center[i] - ( cubeBase[i] + cubeSize ) )*(s->center[i] - ( cubeBase[i] + cubeSize ) );
		}

	}

	if(distSquared <= (s->radius)*(s->radius) )
		return true;
	else 
		return false; */
	
}


bool getPolyCubeInterSectMidPoint(double cb[3], double cSize, Face* my, double mid[3]){

	double vals[8];
	

	__int64 edge[12][2]= {{0, 1},
			              {1, 3},
			              {3, 2},
			              {2, 0},
			              {4, 5},
			              {5, 7},
			              {7, 6},
			              {6, 4},
			              {0, 4},
			              {1, 5},
			              {3, 7},
			              {2, 6}};
	Polygon* p = (Polygon*) my->f;

	

	// calculate the intersect point
	// since the face can be any form the number of intersection point is not determined
	// However, for plane, the intersection point can be 3, 4, 5, 6 for regular case,
	// or can be a corner, an edge, or a face of the cube
	// assumption for now is that there will be maximumly 6 intersection points (not quite?)

	double corner[12][3];
	double cx[8][3];
	__int64 cnt=0;
	for(__int64 i=0; i<8; i++){
		
		cx[i][0] = cb[0]+cSize*(i%2);
		cx[i][1] = cb[1]+cSize*((i/2)%2);
		cx[i][2] = cb[2]+cSize*(i/4);
		vals[i] = pointToPolygonDistance(cx[i], p);
	}

	for(__int64 i=0; i<12; i++){

		if(vals[edge[i][0]]*vals[edge[i][1]]<0){
			double t = fabs(vals[edge[i][0]])/fabs(vals[edge[i][0]]-vals[edge[i][1]]);
			corner[cnt][0] = cx[edge[i][0]][0] + t*(cx[edge[i][1]][0] - cx[edge[i][0]][0]);
			corner[cnt][1] = cx[edge[i][0]][1] + t*(cx[edge[i][1]][1] - cx[edge[i][0]][1]);
			corner[cnt][2] = cx[edge[i][0]][2] + t*(cx[edge[i][1]][2] - cx[edge[i][0]][2]);
			cnt++;
		}
	}

	mid[0]=mid[1]=mid[2]=0;
	for(__int64 i=0; i<cnt; i++){
		mid[0] += corner[i][0];
		mid[1] += corner[i][1];
		mid[2] += corner[i][2];
	}

	mid[0] /= cnt;
	mid[1] /= cnt;
	mid[2] /= cnt;

	return true;

}


bool getPolyPatchCubeInterSectMidPoint(double cb[3], double cSize, Face* my, double mid[3]){

	double vals[8];
	

	__int64 edge[12][2]= {{0, 1},
			              {1, 3},
			              {3, 2},
			              {2, 0},
			              {4, 5},
			              {5, 7},
			              {7, 6},
			              {6, 4},
			              {0, 4},
			              {1, 5},
			              {3, 7},
			              {2, 6}};
	PolygonPatch* p = (PolygonPatch*) my->f;

	

	// calculate the intersect point
	// since the face can be any form the number of intersection point is not determined
	// However, for plane, the intersection point can be 3, 4, 5, 6 for regular case,
	// or can be a corner, an edge, or a face of the cube
	// assumption for now is that there will be maximumly 6 intersection points (not quite?)

	double corner[12][3];
	double cx[8][3];
	__int64 cnt=0;
	for(__int64 i=0; i<8; i++){
		
		cx[i][0] = cb[0]+cSize*(i%2);
		cx[i][1] = cb[1]+cSize*((i/2)%2);
		cx[i][2] = cb[2]+cSize*(i/4);
		vals[i] = pointToPolygonPatchDistancePtrUsage(cx[i], p);
	}

	for(__int64 i=0; i<12; i++){

		if(vals[edge[i][0]]*vals[edge[i][1]]<0){
			double t = fabs(vals[edge[i][0]])/fabs(vals[edge[i][0]]-vals[edge[i][1]]);
			corner[cnt][0] = cx[edge[i][0]][0] + t*(cx[edge[i][1]][0] - cx[edge[i][0]][0]);
			corner[cnt][1] = cx[edge[i][0]][1] + t*(cx[edge[i][1]][1] - cx[edge[i][0]][1]);
			corner[cnt][2] = cx[edge[i][0]][2] + t*(cx[edge[i][1]][2] - cx[edge[i][0]][2]);
			cnt++;
		}
	}

	mid[0]=mid[1]=mid[2]=0;
	
	for(__int64 i=0; i<cnt; i++){
		mid[0] += corner[i][0];
		mid[1] += corner[i][1];
		mid[2] += corner[i][2];
	}

	mid[0] /= cnt;
	mid[1] /= cnt;
	mid[2] /= cnt;


	return true;

}


bool getSphereCubeInterSectMidPoint(double cb[3], double cSize, Face* my, double mid[3]){


	//first go through each edge of the cube
	//if there are interection points withe edges then get all the intersection points and return the average of all the points
	//if there is no intersection points on edge, then the intersection must on one of the face

//	mid[0] = cb[0]+cSize/2;
//	mid[1] = cb[1]+cSize/2;
//	mid[2] = cb[2]+cSize/2;
//	return true;

	__int64 edge[12][2]= {{0, 1},
			              {1, 3},
			              {3, 2},
			              {2, 0},
			              {4, 5},
			              {5, 7},
			              {7, 6},
			              {6, 4},
			              {0, 4},
			              {1, 5},
			              {3, 7},
			              {2, 6}};
	Sphere* s = (Sphere*) my->f;

	

	// calculate the intersect point
	// since the face can be any form the number of intersection point is not determined
	// However, for plane, the intersection point can be 3, 4, 5, 6 for regular case,
	// or can be a corner, an edge, or a face of the cube
	// assumption for now is that there will be maximumly 6 intersection points (not quite?)

	double corner[12][3];
	double vals[8];
	double cx[8][3];
	__int64 cnt=0;
	for(__int64 i=0; i<8; i++){
		
		cx[i][0] = cb[0]+cSize*(i%2);
		cx[i][1] = cb[1]+cSize*((i/2)%2);
		cx[i][2] = cb[2]+cSize*(i/4);
		vals[i] = pointToSphereDistance(cx[i], *s);
	}

	for(__int64 i=0; i<12; i++){

		if(vals[edge[i][0]]*vals[edge[i][1]]<0){
			double t = fabs(vals[edge[i][0]])/fabs(vals[edge[i][0]]-vals[edge[i][1]]);
			corner[cnt][0] = cx[edge[i][0]][0] + t*(cx[edge[i][1]][0] - cx[edge[i][0]][0]);
			corner[cnt][1] = cx[edge[i][0]][1] + t*(cx[edge[i][1]][1] - cx[edge[i][0]][1]);
			corner[cnt][2] = cx[edge[i][0]][2] + t*(cx[edge[i][1]][2] - cx[edge[i][0]][2]);
			cnt++;
		}
	}

	if(cnt>0){
		mid[0]=mid[1]=mid[2]=0;
		for(__int64 i=0; i<cnt; i++){
			mid[0] += corner[i][0];
			mid[1] += corner[i][1];
			mid[2] += corner[i][2];
		}

		mid[0] /= cnt;
		mid[1] /= cnt;
		mid[2] /= cnt;

		return true;

	}
	else{

		double dist;

		//x direction, the distance of the two 
		for(int i=0; i<3; i++){

			dist = fabs(cb[i]-s->center[i]);
			if(dist<s->radius){
				mid[i] = cb[i];
				mid[(i+1)%3] = s->center[(i+1)%3];
				mid[(i+2)%3] = s->center[(i+2)%3];
				return true;
			}
			dist = fabs(cb[i]+cSize-s->center[i]);
			if(dist<s->radius){
				mid[i] = cb[i]+cSize;
				mid[(i+1)%3] = s->center[(i+1)%3];
				mid[(i+2)%3] = s->center[(i+2)%3];
				return true;
			}


		}

		return true;
	}

	return false;

}
