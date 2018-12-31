/*
 * VRMLFile.h
 *
 *  Created on: Jul 9, 2010
 *      Author: zrui
 */

#ifndef VRMLFILE_H_
#define VRMLFILE_H_


#include <iostream>
#include "myFile.h"
#include "geoNodeArr.h"
#include "geoFaceArr.h"
#include "Patch.h"
#include "pointerArr.h"
#include "stringArrNonDup.h"
#include "Tessellator.h"
#include "geoLib.h"


//default size limit of the bndStr is limited to 1000
//if need to increase then change the array sise of curBndStr in method of readShape()

class VRMLFile {
public:
	VRMLFile(char* fName, geoNodeArr* node, geoFaceArr* face, stringArrNonDup* bndStr);
	virtual ~VRMLFile();
    void loadFile();

private:

	myFile* file;
    geoNodeArr* nodes;
    geoFaceArr* faces;
    stringArrNonDup* bndStr;
    int shapeCnt;



	double offset[3];
	double scale[3];
	double rotation[4]; // the rotation angle for each axis
	char* currentBnd;
	double unitConvt;
	bool   convex;


    //data structure related to triangle patch
    pointerArr poly; // the array to hold the polygons, after all the polygons
                     // have been read in, a function will try to stitch the polygons into patches
                     // testing methods:
                     //                 >> on the same plane;
                     //                 >> at least one shared vertices with other polys
                     //                    already in the patch
                     // then add the patches to the faces array as polyPatch


	pointerArr pat; // list of patches



    vector< vector<int> > fNIds;





//#define TRANSFORM = 1;
//#define SHAPE = 2;
//#define INDEXEDFACESET = 3;



	char geometryLookUpTbl[10][100];
	char nodeLookUpTbl[10][100];


	void initialize();
	void resetNode();

	void readTransformNode();

	void readShapeNode();
    void readIndexedFaceSet(char* curBndStr);
	void readCoord(vector<double> &coord);
	void readCoordIndex(vector<int> &cId);

    void readDataBlockDouble(vector<double> &data, char sta, char end);
    void readDataBlockInt(vector<int> &data, char sta, char end);

    bool checkFaceDup(vector<int> &cdId);

    void addToPolyArr(Face* curF, vector<int>& cdId);

    void addToPatches(int fId, Face* fc, vector<int> nId, int numN);

    void stitchPatch(int ii, int jj);

    void addToFNIds(vector<int> cdId);
    void addPolyPatchToFaceArr();


    void readAppearance(char* curBnd);
    void addToFaceArr(vector<int> id, vector<double> coord, vector<int> & vId, char* curBnd);

	void readSphere(char* curBnd);

};

#endif /* VRMLFILE_H_ */
