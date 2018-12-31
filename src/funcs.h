/*
 * funcs.h
 *
 *  Created on: Oct 27, 2009
 *      Author: zrui
 */

#ifndef FUNCS_H_
#define FUNCS_H_
#include "geoLib.h"
#include "Face.h"


/**
 * 	0: polyLine
 *  1: arc
 *  2: linear
 *  3: triangular
 *  4: quadrilateral
 *  5: polygon
 *  6: circle
 *  7: bspline
 *  8: sphere
 *  9: cylinder
 * 10: polygonpatch
 *
 */


static  bool  (*ptGetInterSectLine[11][11])(Face *face, Face *my, vector<Face*>& interLine) ={
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, &polyPolyInterSect, &polyPolyInterSect, &polyPolyInterSect, &polyPolyInterSect, NULL, NULL, &polySphereInterSect, NULL, &polyPolyPatchInterSect},
        { NULL, NULL, &polyPolyInterSect, &polyPolyInterSect, &polyPolyInterSect, &polyPolyInterSect, NULL, NULL, &polySphereInterSect, NULL, &polyPolyPatchInterSect },
        { NULL, NULL, &polyPolyInterSect, &polyPolyInterSect, &polyPolyInterSect, &polyPolyInterSect, NULL, NULL, &polySphereInterSect, NULL, &polyPolyPatchInterSect },
        { NULL, NULL, &polyPolyInterSect, &polyPolyInterSect, &polyPolyInterSect, &polyPolyInterSect, NULL, NULL, &polySphereInterSect, NULL, &polyPolyPatchInterSect },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, &polySphereInterSect, &polySphereInterSect, &polySphereInterSect, &polySphereInterSect, NULL, NULL, &sphereSphereInterSect, NULL, &spherePolyPatchInterSect },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, &polyPatchPolyInterSect, &polyPatchPolyInterSect, &polyPatchPolyInterSect, &polyPatchPolyInterSect, NULL, NULL, &polyPatchSphereInterSect, NULL, &polyPatchPolyPatchInterSect },
};


static  double  (*ptGetDistanceToPoint[11])(double *p, Face *my) ={
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
        NULL
};

static  double  (*ptGetDistanceToFace[11][11])(void *face, void *my) ={
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, &polygonToPolygonDistancePtrUsage, &polygonToPolygonDistancePtrUsage, &polygonToPolygonDistancePtrUsage, &polygonToPolygonDistancePtrUsage, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, &polygonToPolygonDistancePtrUsage, &polygonToPolygonDistancePtrUsage, &polygonToPolygonDistancePtrUsage, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, &polygonToPolygonDistancePtrUsage, &polygonToPolygonDistancePtrUsage, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, &polygonToPolygonDistancePtrUsage, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
};

static  void  (*ptProjectToFace[11][11])(double *x, double *p, Face *my) ={
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL


};

static int (*getNumIntersectPntWithRay[11][11])(double *x, double *dire)={
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL

};


static  bool  (*ptCubeInterSectCheck[11])(double *cubeBase, double cubeSize, Face *my) ={
		NULL,
		NULL,
		polyCubeInterSectCheck,
		polyCubeInterSectCheck,
		polyCubeInterSectCheck,
		polyCubeInterSectCheck,
		NULL,
		NULL,
		sphereCubeInterSectCheck,
		NULL,
        polyPatchCubeInterSectCheck
};


static  bool  (*ptGetCubeInterSectMidPoint[11])(double *cb, double cSize, Face *my, double mid[3]) ={
		&getPolyCubeInterSectMidPoint,
		NULL,
		&getPolyCubeInterSectMidPoint,
		&getPolyCubeInterSectMidPoint,
		&getPolyCubeInterSectMidPoint,
		&getPolyCubeInterSectMidPoint,
		NULL,
		NULL,
		&getSphereCubeInterSectMidPoint,
		NULL,
        &getPolyPatchCubeInterSectMidPoint
};

/**
 * 	0: polyLine
 *  1: arc
 *  2: linear
 *  3: triangular
 *  4: quadrilateral
 *  5: polygon
 *  6: circle
 *  7: bspline
 *  8: sphere
 *  9: cylinder
 * 10: polygonpatch
 *
 */

#endif /* FUNCS_H_ */
