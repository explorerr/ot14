/*
 * geoModel.h
 *
 *  Created on: Feb 26, 2009
 *      Author: zrui
 */

#ifndef GEOMODEL_H_
#define GEOMODEL_H_

#include "myFile.h"
//#include "Project.h"
#include "math.h"
#include "intArr.h"
#include "Face.h"
#include "Zone.h"
#include "geoLib.h"
#include "interSectLines.h"
#include "geoNodeArr.h"
#include "geoFaceArr.h"
#include "geoZoneArr.h"
#include "geoBndArr.h"
#include "VRMLFile.h"
#include "stringArrNonDup.h"


#include <vector>
#include <list>
#include <iostream>
#include <string>
#include <sstream>
//class Face;
//class Zone;
using namespace std;

class geoModel {
public:
	geoModel();
	virtual ~geoModel();

    void    buildModel(char* fName, char* ext, int domMode, int extMode, double extMultiplier);
	void    toFile();
	void 	buildModelFromGMZ(char* fName);
	void	buildModelFromVRML(char* fName);


	void    readGMZFile(char* fName);

    void    readVRMLFile(char* fName);
    void    addDomainDefFaceForVRMLFile();

    bool    modelOnGridCheck(double cSize);


	double	shortestDistanceInModel();
	void    getInterFaceId(double x[3], double length, intArr** fId);
	void    getInterFaceId(double x[3], double length, intArr** fId, intArr* idSet);
	void    getInnerNodeId(double x[3], double length, intArr** nId);
	void    getInnerNodeId(double x[3], double length, intArr** nId, intArr* idSet);
	void    getZoneOrBndId(double x[3], __int64& zone, __int64& bnd);
	void    getNode( __int64 id, double cor[3]);
	void    project2Face(double x[3],  __int64 fId, double p[3], __int64& bnd);
	void    getBndNeiZone( __int64 b,  __int64 *z);
     __int64     getFaceBnd( __int64 fId);
    double  minDistanceToFace(double x[3]);
    double  getDistance2Face( __int64 fc, double p[3]);
    bool    getMidPointOfIntersectionLineWithCube( __int64 a,  __int64 b,
                                  double cube[3], double size, geoNodeArr& intPnt);

    void    getPlanarFaceFunction(__int64 fc, double equa[4]);
    __int64 getBndOfFace(__int64 fc);

	bool    isFaceZoneDef(__int64 fc);


    bool    featureNodeCheck(intArr* nd);
    bool    featureFaceCheck(intArr* fc);

	int		getTotolNumberOfFace(){ return totlNumFace;};

	bool    getCubeInterSectMidPoint(double *cb, double cSize, int fc, double mid[3]);



    void printGeoFaces();
    void printGeoFeatureNodes();

	 __int64     ND; //the dimension (2D or 3D) of the input data.
	double  domainFrame[6]; //the physical coordinates of the max and min z, y, z of the domain.
	double  domainSize[3]; //the physical dimension of the domain.
	double  domainCenter[3]; //the physical position of the center point
	double  length; // the longest distance in the model


private:

    void 	getGMZSectionInfo(char* curLine, vector<__int64> &info);
	void 	gotoGMZBeginOfDataBody(myFile &file);
	void 	readGMZNodes(myFile& file, geoNodeArr* nodes,  __int64 numNode);
	void 	readGMZFaces(myFile& file,  __int64 numFace);



    void	calDomainSize();


	void    printNormalInfoToBstrFile(char* fName);




	int extMode;
	int domMode;
	double extMultiplier;

    geoNodeArr  *featureNodes; //feature nodes

	geoNodeArr  *infoNodes; // definition nodes, such as center po __int64 of a sphere

	geoNodeArr  *domainFeatNodes; // feature nodes in the


/***************************************************************/
	geoFaceArr  *faces; //face array will have a tag for each face meaning zonefaces and partfaces

/***************************************************************/

	geoZoneArr  *zones; // store the zoneFace id in zoneFaces array for each zone
	geoBndArr   *bndNeiZone;
	interSectLines *interLines;// store all the intersection lines

	stringArrNonDup *bndStr;

	double  myEPSILON;

     __int64 totlNumNode, totlNumInfoNode, totlNumZone, totlNumFace;
     __int64 totlNumBnd;
};

#endif /* GEOMODEL_H_ */
