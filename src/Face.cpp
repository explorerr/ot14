#include "stdafx.h"
#include "Face.h"
#include "Polygon.h"
#include "PolygonPatch.h"
//#include "funcs.h"

Face::Face(int type, vector<double>& info)
{

	this->type = type;
	int len = info.size();
	double temp[100];
	switch (type) {
	case 0x2: 	// linear
				// info contains the coordinates (x, y, z) of the two vertices of the edge
		if (len < 10) throw StoppingException("Error: Not enough information for linear object in Face class.\n");
		for(int i=0; i<6; i++) temp[i] = info.at(i);
		f = (Polygon*) new Polygon(2, temp);
		ptDistToPoint = &pointToPolygonDistancePtrUsage;
		ptDistToPolygon = &polygonToPolygonDistancePtrUsage;
		ptDistToSphere = &sphereToPolygonDistancePtrUsage;
		ptGetNumIntersectPntWithRay = &getNumIntPntRayLinePtrUsage;
		ptProject2Face = &project2LineSegPtrUsage;
		break;

	case 0x3: 	// trianglar
				// three coordinates (x, y, z) of the three vertices of the triangle
		if (len < 13) throw StoppingException("Error: Not enough information for triangle object in Face class.\n");
		for(int i=0; i<9; i++) temp[i] = info.at(i);
		f = (Polygon*) new Polygon(3, temp);
		ptDistToPoint = &pointToPolygonDistancePtrUsage;
		ptDistToPolygon = &polygonToPolygonDistancePtrUsage;
		ptDistToSphere = &sphereToPolygonDistancePtrUsage;
		ptGetNumIntersectPntWithRay = &getNumIntPntRayPolygonPtrUsage;
		ptProject2Face = &project2PolygonPtrUsage;
		break;

	case 0x4: 	//quadrilateral
				// four coordinates (x, y, z) of the four vertices of the quadriateral
		if (len < 16) throw StoppingException("Error: Not enough information for quad object in Face class.\n");
		for(int i=0; i<12; i++) temp[i] = info.at(i);
		f = (Polygon*) new Polygon(4, temp);
		ptDistToPoint = &pointToPolygonDistancePtrUsage;
		ptDistToPolygon = &polygonToPolygonDistancePtrUsage;
		ptDistToSphere = &sphereToPolygonDistancePtrUsage;
		ptGetNumIntersectPntWithRay = &getNumIntPntRayPolygonPtrUsage;
		ptProject2Face = &project2PolygonPtrUsage;
		break;

	case 0x5: 	{// polygonal n
				// first data is the number of vertices of the Polygon NN
				// follows NN coordinates (x, y, z) of the NN vertices of the quadriateral
		        // 
		int nv = (int) info[0];
		if( len < 10 )throw StoppingException("Error: Not enough information for Polygon object in Face class.\n"); //at least to be a linear
		for(int i=0; i<3*nv; i++) temp[i] = info.at(i+1);
		f = (Polygon*) new Polygon(nv, temp);
		ptDistToPoint = &pointToPolygonDistancePtrUsage;
		ptDistToPolygon = &polygonToPolygonDistancePtrUsage;
		ptDistToSphere = &sphereToPolygonDistancePtrUsage;
		ptGetNumIntersectPntWithRay = &getNumIntPntRayPolygonPtrUsage;
		ptProject2Face = &project2PolygonPtrUsage; 
				}
		break;

	case 0x6: 	// circle
    	// first is the diameter of the circle, and followed by the coordinates (x, y, z)
				// of the center point
		if( len < 10 ) throw StoppingException("Error: Not enough information for circle object in Face class.\n");
		for(int i=0; i<6; i++) temp[i] = info.at(i);
		f = (Sphere*) new Sphere(temp);
		break;

	case 0x7: 	// bspline
				// tobe decided
		break;

	case 0x8: 	// sphere
				// first is the diameter of the circle, and followed by the coordinates (x, y, z)
				// of the center point
		if( len < 10 ) throw StoppingException("Error: Not enough information for sphere object in Face class.\n");
		for(int i=0; i<6; i++) temp[i] = info.at(i);
		f = (Sphere*) new Sphere(temp);
		ptDistToPoint = &pointToSphereDistancePtrUsage;
		ptDistToPolygon = &polygonToSphereDistancePtrUsage;
		ptDistToSphere = &sphereToSphereDistancePtrUsage;
		ptGetNumIntersectPntWithRay = &getNumIntPntRaySpherePtrUsage;
		ptProject2Face = &project2SpherePtrUsage;
		break;

	case 0x9:  //cylinder
		       //functions to be added
		break;

	case 0xa: // polygonPatch
			  // info: boolean operation type, pointers to the Face object (polygon type)
		      // then this part of the code will check the "type"
		      // "type" should all be "4"
			  // then check the neighZone, this is a little complicated
		      // then check the bnd sign, also a little complicated

		createPolygonPatch(info);
		return;
		      



	}
	neighZone[0] = (int)info[len-4];
	neighZone[1] = (int)info[len-3];
	bnd = (int) info[len-2];
	zDefFace = (int) info[len-1];
}

Face::~Face(){

	if(f == NULL)return;

	switch (type){
	case 0x2: 	// linear
				// info contains the coordinates (x, y, z) of the two vertices of the edge
		delete ( (Polygon*)f );
		break;

	case 0x3: 	// trianglar
				// three coordinates (x, y, z) of the three vertices of the triangle
		delete ( (Polygon*)f );
		break;
	case 0x4: 	//quadrilateral
				// four coordinates (x, y, z) of the four vertices of the quadriateral
		delete ( (Polygon*)f );
		break;

	case 0x5: 	// polygonal n
				// first data is the number of vertices of the Polygon NN
				// follows NN coordinates (x, y, z) of the NN vertices of the quadriateral
		delete ( (Polygon*)f );
		break;

	case 0x6: 	// circle
				// first is the diameter of the circle, and followed by the coordinates (x, y, z)
				// of the center point
		delete( (Sphere*) f );
		break;

	case 0x7: 	// bspline
				// tobe decided
		break;

	case 0x8: 	// sphere
				// first is the diameter of the circle, and followed by the coordinates (x, y, z)
				// of the center point
		delete ( (Sphere*) f);
		break;

	}
}


double Face::distanceToPoint(double *p){
	return ptDistToPoint(p, this->f);
}

double Face::distanceToPolygon(Polygon *poly){
	return ptDistToPolygon(poly, this->f);
}

double Face::distanceToSphere(Sphere * s){
	return ptDistToSphere(s, this->f);
}

int Face::getNumIntersectPntWithRay(double * x, double *dire){
	return ptGetNumIntersectPntWithRay(x, dire, this->f);
}

void Face::project2Face(double *x, double *p, __int64& bnd){
	ptProject2Face(x, p, this->f);
	bnd = this->bnd;
}

double Face::distanceToFace(Face *f){
	double dist=-1;
	switch (f->type){
	case 0x2:  //linear
	    dist = distanceToPolygon( (Polygon*)f->f);
		break;
	case 0x3:  //trianglar
		dist = distanceToPolygon( (Polygon*)f->f);
		break;
	case 0x4:  //quadrilateral
		dist = distanceToPolygon( (Polygon*)f->f);
		break;
	case 0x5:  //polygonal
		dist = distanceToPolygon( (Polygon*)f->f);
		break;
	case 0x6:  //circle
		dist = 0;
		break;
	case 0x7:  //bspline
		dist = 0;
		break;
	case 0x8:  //sphere
		dist = distanceToSphere((Sphere*) f->f);
	}
	return dist;
}

bool Face::getInterSectLine(Face* face, vector<Face*>& interLine){

	int i,j;
//    i=(this->type > face->type) ? this->type : face->type;
//    j=(this->type < face->type) ? this->type : face->type;

	i = this->type;
	j = face->type;
    if(ptGetInterSectLine[i][j]==NULL){
    	fprintf(stderr,
    			"Error: getInterSectLine method of the faces is not implemented, face type: %d, %d.",
				i, j);
    	throw StoppingException("\n");
    }
    else{
    	return ptGetInterSectLine[i][j](this, face, interLine);
    }
}

bool   Face::getInterSectPointWithCube(double cx[3], double size, double *pnt){
	if(this->type == 2){
		Polygon* p = (Polygon*)this->f;
		double line[6] = {p->v[0][0], p->v[0][1], p->v[0][2], p->v[1][0], p->v[1][1], p->v[1][2]};
		return getLineSegCubeInterSectPoint(line, cx, size, pnt);
	}
	if(this->type == 1){
		return getArcCubeInterSectPoint((Arc*)this->f, cx, size, pnt);
	}
	else{
		throw StoppingException("Error: getInterSectPointWithCube only deals with arc and line segment for now.\n");
	}

}




void Face::createPolygonPatch( vector<double>& info ){


	// assumption is that the polygons are coplanar

	// the info is organized as follows:
	// [0]: operation between all the polygon
	// [1]: number of polygon
	// [+]: neighZone+ for each polygon
	// [+]: neighZone- for each polygon
	// [+]: bnd for each polygon
	// [+]: zDefFace for each polygon
	// [2]: numV of first polygon
	// [+]: coordinates of all the vertices of the first polygon
	// [+]: numV for the next polygon
	// [+]: coordinates for next polygon
	// [+]: and so on for the next polygon

	double myEPSILON = EPSILON;

	if( info.size() < 1 )
		throw StoppingException("Error: Not enough information for polygonPatch object.\n");

	int op = (int) info[0];

	int numP = (int) info[1];
	// Checking the validity of the patch, in terms of neighZone, bnd and zDefFace 

    if(op==1){// boolean union
              // all the polygon should have the same 
              // neighZone, bnd, and zDefFace
        neighZone[0] = (int) info[2];
        neighZone[1] = (int) info[2+numP];
        bnd = (int) info[2+numP*2];
        zDefFace = (int) info[2+numP*3];
        for(int i=1; i<numP; i++){
            if(info[2+i] != neighZone[0] 
                    || info[2+numP+i] != neighZone[1] 
                    || info[2+numP*2+i] != bnd 
                    || info[2+numP*3+i] != zDefFace )

                throw StoppingException("Error: polygon patch union operation, but with different neighZone, bnd and zDefFace.\n");
        }
    }
    
    
    else if(op==-1){ // boolean subtract
                     // assumption is that the first ploy subtract the rest of the poly
                     // the neighzone+-, bnd and zDefFace, should be the same as the first polygon
        neighZone[0] = (int)info[2];
        neighZone[1] = (int)info[2+numP];
        bnd = (int) info[2+numP*2];
        zDefFace = (int) info[2+numP*3];
    }
    
    
    else{
        throw StoppingException("Error: polygon patch, unknown operation.\n");
    }




    // get the first polygon info
	int cur = 2+numP*4;
    Polygon** polys = (Polygon**) malloc( sizeof(Polygon*)*numP );
    int nv;
    double* temp;
	for(int p=0; p<numP; p++){
        nv = (int) info[cur++];
		temp = (double*) malloc(sizeof(double)*nv*3);
		for(int i=0; i<nv*3; i++){
			temp[i] = info[cur++];	
		}
        polys[p] = (Polygon*) new Polygon(nv, temp);

        free(temp);
	}

	//check coplanar
	double cross[3];
	for(int i=0; i<numP; i++) {
		for(int j=i; j<numP; j++){

			Cross(polys[i]->n, polys[j]->n, cross);
			if( fabs (cross[0] + cross[1] + cross[2] ) > myEPSILON ){
				
				for(int k=0; k<numP; k++)
					delete(polys[k]);
				free(polys);

				throw StoppingException("Error: in polygonPatch, polygons not coplanar.\n");

			}
		}
	}

    f = (PolygonPatch*) new PolygonPatch (numP, op, polys);


	ptDistToPoint = &pointToPolygonPatchDistancePtrUsage;
	ptDistToPolygon = &polygonToPolygonPatchDistancePtrUsage;
	ptDistToSphere = &sphereToPolygonPatchDistancePtrUsage;
	ptGetNumIntersectPntWithRay = &getNumIntPntRayPolygonPatchPtrUsage;
	ptProject2Face = &project2PolygonPatchPtrUsage;
	


	


}


bool Face::cubeInterSectCheck(double cb[3], double size){

   if(ptCubeInterSectCheck[this->type]==NULL){
    	fprintf(stderr,
    			"Error: cubeInterSectCheck method of the faces is not implemented, face type: %d.",
				this->type);
    	throw StoppingException("\n");
    }
    else{
    	return ptCubeInterSectCheck[this->type](cb, size, this);
    }


}



Face::Face(int type, vector<int> &info, vector<Polygon*> &f){

	if(type != 0xa){
		throw StoppingException("Error: in Face constructor Face(int type, Face** f, int numP, int op), this "
				" constructor only supports Face construction of PolygonPatch, but the type passed in "
				" is not a PolyPatch.\n");
	}

	this->type = type;
	int op = info[0];
	int numP = info[1];
    neighZone[0] = info[2];
    neighZone[1] = info[3];
    bnd = info[4];
    zDefFace = info[5];

    if( op != -1 && op != 1){
        throw StoppingException("Error: polygon patch, unknown operation.\n");
    }

    Polygon** polys = (Polygon**) malloc( sizeof(Polygon*)*numP );

	

    for(int i=0; i<numP; i++){
    	polys[i] = f[i];
    }

	PolygonPatch* newFace = new PolygonPatch (numP, op, polys);


	this->f = newFace;

	ptDistToPoint = &pointToPolygonPatchDistancePtrUsage;
	ptDistToPolygon = &polygonToPolygonPatchDistancePtrUsage;
	ptDistToSphere = &sphereToPolygonPatchDistancePtrUsage;
	ptGetNumIntersectPntWithRay = &getNumIntPntRayPolygonPatchPtrUsage;
	ptProject2Face = &project2PolygonPatchPtrUsage;

}


double* Face::getPlanarFaceFunction(){

	double* ret = NULL;

	if( (type>5 && type!=10) || type < 2){
		return ret;
	}
	switch (type) {
	case 0x2:  //linear
	    ret = ( (Polygon*)f )->func;
		break;
	case 0x3:  //trianglar
		ret = ( (Polygon*)f )->func;
		break;
	case 0x4:  //quadrilateral
		ret = ( (Polygon*)f )->func;
		break;
	case 0x5:  //polygonal
		ret = ( (Polygon*)f )->func;
		break;
	case 0xa:

		ret = ( (PolygonPatch*)f )->polys[0]->func;

	}
	return ret;

}


bool Face::getCubeInterSectMidPoint(double *cb, double cSize, double mid[3]){

	if(ptGetCubeInterSectMidPoint[this->type]==NULL){
    	fprintf(stderr,
    			"Error: getCubeInterSectMidPoint method of the face is not implemented, face type: %d.",
				this->type);
    	throw StoppingException("\n");
    }
    else{
    	return ptGetCubeInterSectMidPoint[this->type](cb, cSize, this, mid);
    }

}





