
#ifndef GEOLIB_
#define GEOLIB_

#include "Project.h"
#include "Exceptions.h"
#include "lib.h"
#include <math.h>
#include <vector>

class Face;
class Polygon;
class Sphere;
class Arc;
class PolygonPatch;

//return the shortest distance between two faces, will return -1 if faces intersects
double 	getShortestDistance(Face &f1, Face &f2);

double 	segmentSegmentDistance3D(Polygon *uu, Polygon *vv);

double 	Dot(double x1, double y1, double x2, double y2 );

double 	Dot(double* u, double* v );

double  Dot2D(double *u, double *v);

void 	Cross(double *u, double *v, double *cross);

void 	CrossNormalized(double *u, double *v, double *cross);

void    normalize(double *u);

void    normalize2D(double *u);

double 	quadrilateralToQuadrilateralDistance(double *uu, double *vv);

double 	quadrilateralToQuadrilateralDistance(Polygon *p, Polygon *q);

double  pointToPolygonDistance(double *p, double *vv, int nv, double *func);

double  pointToPolygonDistance(double *p, double *vv, int nv, double *func, double* pPrime);

double 	pointToPolygonDistance(double * p, double *vv, int nv);

double 	pointToPolygonDistance(double *p, Polygon *poly);

double 	pointToPolygonDistance(double *p, Polygon *poly, double* pPrime);

double 	pointToPolygonDistancePtrUsage(double *p, void* q);

double 	pointToPolygonDistancePtrUsage(double *p, Face* q);

double 	pointToPolygonDistance(double *p, Polygon &poly);

double 	pointToPointDistance(double *a, double *b);

double 	pointToPointDistanceSqr(double *a, double *b);

double  pointToPointDistance2D(double *p, double *q);

double  pointToPolygonDistance2D(double *p, double *poly2D, int nv, double *q);

double  pointToLineSegDistance(double *p, double *lSeg, double *q);

double  pointToLineSegDistancePrtUsage(double *p, Face *lSeg);

double  pointToLineSegDistance2D(double *p, double *lSeg, double *q);

double  pointToLineDistance(double *p, double *line, double* dire, double *q, double& t);

double  pointToLineDistance2D(double *p, double *line, double* dire, double *q, double& t);

/*
 * return the distance between the point and the sphere, + outside, - inside.
 */
double pointToSphereDistance(double *p, Sphere &s);

double pointToSphereDistancePtrUsage(double *p, void *s);

double pointToSphereDistancePtrUsage(double *p, Face *s);

double polygonToSphereDistance(Polygon *p, Sphere *s);

double polygonToSphereDistancePtrUsage(void *p, void *q);

double polygonToPolygonDistancePtrUsage(void *p, void *q);

double polygonToPolygonDistance(Polygon *p, Polygon *q);

double sphereToPolygonDistancePtrUsage(void *p, void *q);

double sphereToSphereDistancePtrUsage(void *s, void *c);


/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                            UTILITIES
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*/
void 	polyPlaneEquation(double *vv, int nv, double &a, double &b, double &c, double &d);

void 	polyPlaneEquation(double *vv, int nv, double* a);

void    polyPlaneEquation(vector<double> &vv, int nv, double* a);



/** ###############################################################
 *                          INTERSECTIONS
 *  ###############################################################
 */

/**
 * Find number of intersection points between objects
 */
int getNumIntPntRayLinePtrUsage(double *x, double *dire, void *l);

/**
 * This function will find the number of intersection point between the ray line
 * and a Polygon face object
 *
 * @param x the coordinates of the start point of the ray line
 * @param dire the direction vector of the ray line
 * @param f the Polygon face object, Polygon with nv>3
 */
int getNumIntPntRayPolygonPtrUsage(double *x, double *dire, void *f);


/**
 *  This is a general ray-Polygon intersection checking function
 *  will perform the method ref in GTCG p357
 *
 *  @param x coordinates of the start point of the ray line
 *  @param dire the direction vector of the ray line
 *  @param f pointer to the Polygon object
 *
 * */
int polygonIntersectCheck(double *x, double *dire, Polygon *f);


/**
 *  This function will check if a 2D point is inside the
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
bool pointInPolygonCheck2D(double *p2D, double **vv, int nv);

bool pointInPolygonCheck2D(double *p2D, double  *vv, int nv);


/**
 *  This function will check if a ray intersects the special square,
 *  which is orthogonal to the coordinates axises
 *
 *  @param x the coordinates of the start point of the ray line
 *  @param dire the direction vector the ray line
 *  @param p the pointer of the Polygon object
 *
 * */
int squareIntersectCheck(double *x, double *dire, Polygon *p);

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
                                double &t);

/**
 *  This function will check if a ray intersects a circle,
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
                         double *n);


/**
 *  This function will check if a ray intersects the a sphere,
 *
 *  @param x the coordinates of the start point of the ray line
 *  @param dire the direction vector the ray line
 *  @param s the sphere object
 *
 * */
int getNumIntPntRaySpherePtrUsage(double *x, double *dire, void *s);

/**
 *  This function will check if a ray intersects the a sphere,
 *
 *  @param x the coordinates of the start point of the ray line
 *  @param dire the direction vector the ray line
 *  @param s the sphere object
 *
 * */
int getNumIntPntRaySphere(double *x, double *dire, Sphere *s);


/** ###############################################################
 *                          PROJECTION
 *  ###############################################################
 */

bool project2LineSegPtrUsage(double *x, double *p, void *my);

bool project2PolygonPtrUsage(double *x, double *p, void *my);

bool project2SpherePtrUsage(double *x, double *p, void *my);

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
bool pointInCoplanarPolygonCheck3D(Polygon *poly, double* pnt);



/** ###############################################################
 *                         INTERSECTION LINE
 *  ###############################################################
 */

bool polyPolyInterSect(Face *p, Face *q, vector<Face*>& interLine);

bool polySphereInterSect(Face *p, Face *s, vector<Face*>& interLine);

bool sphereSphereInterSect(Face *s, Face *c, vector<Face*>& interLine);


bool polyPolyInterSect(Polygon *p, Polygon *q, vector<Face*>& interLine);
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

int circleLineSegSamePlaneInterPnt(double *c, double r, double *norm,
        double *p, double *q, double t[2]);


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

bool checkColiner(vector<double>& pnts, int end[2]);

bool checkColiner(double (*pnts)[3], int nv, int end[2]);

/**
 *  This function will check weather the given list of three points are on
 *  the same line or not
 *
 */

bool triangleCheckColiner(vector<double>& pnts);

bool linesegLineSegIntersectCheck(vector<double>& pnts);

bool findIntersectSegCoplanerLinePoly(Polygon* poly,
          double *dire, double *p, vector<double>& interPnt );
void  lineLineSegInterSectCheck(double *o, double *dire, double *P1, double *P2, vector<double> &interPnt, int& code);



bool getLineSegCubeInterSectPoint(double line[6], double cx[3], double size, double pnt[6]);

bool getArcCubeInterSectPoint(Arc* arc, double cx[3], double size, double pnt[6]);

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

double LineToLineDistance(double l[3], double ld[3], double m[3], double md[3], double& s, double & t );








double pointToPolygonPatchDistancePtrUsage(double *p, void* q);

double polygonToPolygonPatchDistancePtrUsage(void* p, void* q);

double sphereToPolygonPatchDistancePtrUsage(void *p, void *q);

int getNumIntPntRayPolygonPatchPtrUsage(double *x, double *dire, void *f);

bool project2PolygonPatchPtrUsage(double *x, double *p, void *poly);

bool polyPolyPatchInterSect(Face* poly, Face* polyPatch, vector<Face*>& lines);

bool polyPatchPolyInterSect(Face* polyPatch, Face* poly, vector<Face*>& lines);

bool polyPatchPolyPatchInterSect(Face* p, Face* q, vector<Face*>& lines);

bool polyPolyPatchInterSect(Polygon* p, PolygonPatch* pp, vector<Face*>& lines);

bool spherePolyPatchInterSect(Face* s, Face* p, vector<Face*>& lines);

bool polyPatchSphereInterSect(Face* s, Face* p, vector<Face*>& lines);

void lineSubtract( vector<Face*>& line, vector<Face*>& sub );

void lineStitch( vector<Face*>& line );

bool pointOnLineSegCheck(double p[3], Polygon *l, double& t);

bool pointOnLineSegCheck(double p[3], double l[6], double& t);

bool pointOnLineSegCheck(double p[3], double l[2][3], double& t);

bool polyCubeInterSectCheck(double cubeBase[3], double size, Face* f);

bool polyCubeInterSectCheck(double cubeBase[3], double size, Polygon* p);

int linePlaneInterSect(double line[2][3], Polygon* p, double& t );

bool polyPatchCubeInterSectCheck(double *cubeBase, double cubeSize, Face *f);

bool pointInCubeCheck(double *p, double *cb, double size);

int lineSegCubeInterSectCheck(double *o, double *d, double *cb, double size, double &tN, double &tF);



bool sphereCubeInterSectCheck(double cubeBase[3], double cubeSize, Face* my);


bool getPolyCubeInterSectMidPoint(double cb[3], double cSize, Face* my, double mid[3]);
bool getPolyPatchCubeInterSectMidPoint(double cb[3], double cSize, Face* my, double mid[3]);
bool getSphereCubeInterSectMidPoint(double cb[3], double cSize, Face* my, double mid[3]);

	

#endif
