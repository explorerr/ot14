/*
 * geoModel.cpp
 *
 *  Created on: Feb 26, 2009
 *      Author: zrui
 */
#include "stdafx.h"
#include "geoModel.h"



geoModel::geoModel() {
	ND = 0;
	myEPSILON = EPSILON;

	bndNeiZone = NULL;
    interLines = NULL;

    featureNodes = new geoNodeArr();
	infoNodes = new geoNodeArr();
	faces = new geoFaceArr();
	zones = new geoZoneArr();
	bndStr = new stringArrNonDup();

    totlNumNode = 0; totlNumInfoNode = 0; totlNumZone = 0; totlNumFace = 0;
    totlNumBnd = 0;

}

geoModel::~geoModel() {
	if (featureNodes != NULL)
        delete(featureNodes);
	if (infoNodes != NULL)
		delete(infoNodes);
	if (faces != NULL)
		delete(faces);
	if (zones != NULL)
		delete(zones);
	if (interLines != NULL)
        delete(interLines);
    if(bndNeiZone != NULL)
        delete(bndNeiZone);
    if(bndStr != NULL)
    	delete(bndStr);
}

/**
 * This function will read in the geometry model file
 * and then initialize all the property variables
 *
 */
void geoModel::buildModel(char* fName, char* ext, int domMode, int extMode, double extMultiplier){

	this->extMode = extMode;
	this->domMode = domMode;
	this->extMultiplier = extMultiplier;

	if(strcmp(ext,"gmz")==0)
		buildModelFromGMZ(fName);
	else if( strcmp(ext, "wrl")==0)
		buildModelFromVRML(fName);
	else
		throw StoppingException("Error in buildModel: fName extention is not recognized.\n");
	
	printNormalInfoToBstrFile(fName);
}

void geoModel::buildModelFromGMZ(char* fName){

	readGMZFile(fName);
	calDomainSize();

}





void geoModel::buildModelFromVRML(char* fName){


	double myEPSILON = EPSILON;

	readVRMLFile(fName);
	bndStr->toFile(fName, ".bstr");
	calDomainSize();

	if(domainSize[0] < myEPSILON || domainSize[1] < myEPSILON || domainSize[2] < myEPSILON)
		throw StoppingException("Error in geoModel construction buildModelFromVRML: domain has zero thickness in one of the axis direction.\n ");

	addDomainDefFaceForVRMLFile();


	totlNumNode = featureNodes->length;
	totlNumInfoNode = infoNodes->length;
	totlNumFace = faces->length;
	totlNumBnd = totlNumFace;
	totlNumZone = 1;//the assumption for VRML model is that, there is one zone

	bndNeiZone = new geoBndArr(totlNumBnd);
	 __int64 z1, z2;
	for( __int64 i=0; i<faces->length; i++){
		Face* curF = faces->getFace(i);
		z1 = curF->neighZone[0];
		z2 = curF->neighZone[1];
		if(z1==z2){
			curF->zDefFace = 0;
			zones->add(z1, i);
		}
		else{
			if(z1 != -1)
				zones->add(z1, i);
			if(z2 != -1)
				zones->add(z2, i);
		}
		 __int64 curBnd = curF->bnd;
		for( __int64 j=0; j<totlNumBnd; j++){
            if( (*bndNeiZone)(j,0) == -1)
            	(*bndNeiZone)(j,0) = curBnd;
			if( (*bndNeiZone)(j,0) != curBnd )
                continue;
			else {
				(*bndNeiZone)(j,1) = curF->neighZone[0];
				(*bndNeiZone)(j,2) = curF->neighZone[1];
				break;
			}
		}
	}

//	zones->print();

}



void geoModel::readGMZFile(char* fName){


	featureNodes = new geoNodeArr();
	infoNodes = new geoNodeArr();
	faces = new geoFaceArr();
	zones = new geoZoneArr();


	myFile gmFile(fName, ".gmz");
	char* curLine;
	while( curLine = gmFile.getNxtLine() ){
		 __int64 i=0;
		while(curLine[i] == ' ' || curLine[i] == '\t' )i++;
			if(curLine[i] == '%' || curLine[i] == '\0' || !curLine[i] ) continue;
			else if (curLine[i]=='('){//start with '('

				vector<__int64> info;
				try{getGMZSectionInfo(curLine, info);}
				catch(...){
					printf("%s information extraction failed\n", curLine);
					free(curLine);
					return;
				}

				switch ( info[0] ){
				case 0x0:  //comments
					info.clear();	free(curLine);	continue;

				case 0x1:  //header
					info.clear();	free(curLine);	continue;

				case 0x2:  //dimension
					if(ND!= 0 && info[1]!= ND){
						info.clear();
						throw StoppingException("Error: Input data dimension not match exception.\n");
					}
					else if( ND == 0 ) ND = info[1];
					info.clear();	free(curLine); continue;

				case 0x10:  // nodes
					if( ND!= 0 && info[5]!= ND ){
						info.clear();
						throw StoppingException("Error: Input data dimension not match exception.\n");
					}
					else if( ND == 0 )
						ND = info[5];
					if(info[1] != 0){

						if(featureNodes->length == 0){
							totlNumNode = info[3] - info[2] + 1;
							readGMZNodes(gmFile, featureNodes, totlNumNode);
						}
						else if(infoNodes->length == 0){
							totlNumInfoNode = info[3] - info[2] + 1;
							readGMZNodes(gmFile, infoNodes, totlNumInfoNode);
						}
					}
					info.clear();	free(curLine); continue;

				case 0x12:  //cells
					if( info[1]==0 )
						totlNumZone = info[3] - info[2] + 1;
					info.clear();	free(curLine); continue;

				case 0x13:  //faces
					if( info[1]==0 )
						totlNumFace = info[3] - info[2] + 1;
					else {
						 __int64 numFace = info[3] - info[2] + 1;
						readGMZFaces(gmFile, numFace);
					}
					info.clear();	free(curLine); continue;
				}
			}
		}
	//set zone and bnd
	totlNumBnd = totlNumFace;
	bndNeiZone = new geoBndArr(totlNumBnd);

	 __int64 z1, z2;
	for( __int64 i=0; i<faces->length; i++){
		Face* curF = faces->getFace(i);
		z1 = curF->neighZone[0];
		z2 = curF->neighZone[1];
		if(z1==z2)
			curF->zDefFace = 0;
		else{
			if(z1 != -1)
				zones->add(z1, i);
			if(z2 != -1)
				zones->add(z2, i);
		}


		 __int64 curBnd = curF->bnd;
		for( __int64 j=0; j<totlNumBnd; j++){
            if( (*bndNeiZone)(j,0) == -1)
            	(*bndNeiZone)(j,0) = curBnd;
			if( (*bndNeiZone)(j,0) != curBnd )
                continue;
			else {
				(*bndNeiZone)(j,1) = curF->neighZone[0];
				(*bndNeiZone)(j,2) = curF->neighZone[1];
				break;
			}
		}
	}
//	calDomainSize();
	printf("finish reading in geometry model file...\n");
}


//read the data body section following the declearing line.

void geoModel::readGMZNodes(myFile& file, geoNodeArr* nodes,  __int64 numNode){
	 __int64 curCnt=0;
	char** curRecord;
	char* curLine;

	gotoGMZBeginOfDataBody(file);

	curLine = file.curLine;
	curRecord = split(curLine, &curCnt);

	while (!curLine || curCnt == 0 || curCnt == 6){ //if line empty, read the next and so on
		curLine = file.getNxtLine();
		curRecord = split(curLine, &curCnt);
	}
	if(!curLine)
		throw StoppingException("Error: No data detected for the section.\n");

	if( curCnt != ND ) {
		freeTokenArray(curRecord, curCnt);
		throw StoppingException("Error: Input data dimension not match exception.\n");
	}
	 __int64 j=0;
	while(j<numNode){
		curRecord = split(curLine, &curCnt);
		if(!curRecord || curCnt != ND)break;
        double nodeCords[3];
        for( __int64 i=0; i<3; i++)
            from_string<double>(nodeCords[i], curRecord[i], std::dec);
        nodes->add(nodeCords);
		freeTokenArray(curRecord, curCnt);
		curCnt=0;
		curLine = file.getNxtLine();
		j++;
	}


//	for(  __int64 i=0; i<numNode; i++)
	//	printf("(%f, %f, %f)\n", nodes[i*ND + 0], nodes[i*ND + 1], nodes[i*ND + 2]);

}

//split the line with ' ' and '(', return the integer information of the given line
void geoModel::getGMZSectionInfo(char* curLine, vector<__int64> &info){
	char **tokensArray;
	 __int64 tokenCnt;
	tokensArray = split( curLine, &tokenCnt );
	for( __int64 i=0; i<tokenCnt; i++){
		 __int64 temp;
		try{
			 from_string<__int64>(temp, tokensArray[i], std::hex);
		}
		catch(MyApplicationException&){
			break;
		}
		info.push_back(temp);
	}

}

//position the filestream to the begining of data body section
void geoModel::gotoGMZBeginOfDataBody(myFile& file){
	 __int64 i=0;
	 __int64 lft=0;
	 __int64 right=0;
	char* curLine = file.curLine; //returns the pointer, memory is managed within myFile class;
	while(curLine && lft<3){ //find the third occurrence of '('
		while (curLine[i]){
			if (curLine[i] == '(') lft++;
			else if(curLine[i] == ')') right++;
			i++;}
		curLine = file.getNxtLine();
	}
	if( !curLine || lft<3 ){ throw StoppingException("Error: Input data body not found exception.\n");	}
	if( lft != 3 || right != 1 ) { throw StoppingException("Error: Input data body not found exception.\n");	}

}


void geoModel::readGMZFaces(myFile& file,  __int64 numFace){
	 __int64 curCnt=0;
	char** curRecord;
	char* curLine;
	geoNodeArr *nodes;
	gotoGMZBeginOfDataBody(file);

	curLine = file.curLine;
	curRecord = split(curLine, &curCnt);

	while (!curLine || curCnt == 0 || curCnt == 6){ //if line empty, read the next and so on
		curLine = file.getNxtLine();
		curRecord = split(curLine, &curCnt);
	}
	if(!curLine)
		throw  StoppingException("Error: No data detected for the section.\n");

	 __int64 j=0;
	 __int64 type;  // temp variable to store the type of the face
	 __int64 iter; //iter for the tokens of the current line

	vector<double> info; // vector to store the info extracted from the line
	while(j<numFace){
		curRecord = split(curLine, &curCnt); //split the current line
		iter=0; //iter reset

		from_string<__int64>(iter, curRecord[0], std::hex); // convert from string to int
		type = iter; //the first  __int64 is the type of the face

		if(type==8){ //sphere, should get the nodes from the infoNodes array
			nodes = this->infoNodes;
		}
		else nodes = this->featureNodes;
		for( __int64 i=1; i<curCnt-3; i++){ // iterate through rest of the info
			iter = 0; //reset
			from_string<__int64>(iter, curRecord[i], std::hex); // string to int
			double* temp = nodes->getNode(iter-1);
			if(ND==3){// get nodes coordinates
				info.push_back(temp[0]);
				info.push_back(temp[1]);
				info.push_back(temp[2]);
			}
			else if(ND==2){
				info.push_back(temp[0]);
				info.push_back(temp[1]);
				info.push_back(0.);
			}
		}
		//get zone and bnd information
		 __int64 zzb[3];
		for( __int64 i=3; i>0; i--){
			from_string<__int64>(iter, curRecord[curCnt-i], std::hex);
			zzb[i] = iter;
			info.push_back(iter);
		}

		//if a face is zonedefining face or a partition face is decided by the two neighbor zone sign
		//if the two neighbor zone sign are the same then it is a partition face

		if( zzb[0] == zzb[1] )
			info.push_back(0);
		else
			info.push_back(1);

		freeTokenArray(curRecord, curCnt);
		Face *curF = new Face(type, info);
		faces->add(curF);
		info.clear();

		//reset temp variable and goto next line
		curCnt=0;
		curLine = file.getNxtLine();
		char** temp = split(curLine, &curCnt);
		if(!temp) break;
		curRecord = temp;
		j++;
	}

}



double	geoModel::shortestDistanceInModel(){
	double srtDist;
	double* tmpN1 = featureNodes->getNode(0);
	double* tmpN2 = featureNodes->getNode(1);
	srtDist = pointToPointDistance(tmpN1, tmpN2); //initialize
	double myEPSILON = EPSILON;
	//po __int64 to po __int64 in featuerNodes
	double *ii, *jj, xx, yy, zz, ll;
	for( __int64 i=0; i<featureNodes->length; i++){
		ii = featureNodes->getNode(i);
		for( __int64 j=i+1; j<featureNodes->length; j++){
			jj = featureNodes->getNode(j);
			xx = ii[0] - jj[0];
			if(fabs(xx) >= srtDist)continue;
			yy = ii[1] - jj[1];
			if(fabs(yy) >= srtDist)continue;
			zz = ii[2] - jj[2];
			if(fabs(zz) >= srtDist)continue;
			ll = sqrt(xx*xx + yy*yy + zz*zz);
			if(ll<myEPSILON)
				continue;
			if(ll<srtDist){
				srtDist = ll;
//				cout<<"shorter distance found between the two points: ("<<ii[0]
	//			         <<", "<<ii[1]<<", "<<ii[2]<<") ("<<jj[0]<<", "<<jj[1]
	//			         <<", "<<jj[2]<<")"<<endl;
			}
		}
	}
	//po __int64 to po __int64 in infoNodes
	for( __int64 i=0; i<infoNodes->length; i++){
		ii = infoNodes->getNode(i);
		for( __int64 j=i+1; j<infoNodes->length; j++){
			jj = infoNodes->getNode(j);
			xx = ii[0] - jj[0];
			if(xx >= srtDist)break;
			yy = ii[1] - jj[1];
			if(yy >= srtDist)break;
			zz = ii[2] - jj[2];
			if(zz >= srtDist)break;
			ll = sqrt(xx*xx + yy*yy + zz*zz);
			if(ll<srtDist)srtDist = ll;
		}
	}

	//po __int64 to surface
//	double temp;
//	for( __int64 i=0; i<faces->length; i++){
//		Face* curFace = faces->getFace(i);
//		for( __int64 j=i+1; j<faces->length; j++){
//			temp = curFace->distanceToFace(faces->getFace(j));
//			if (temp<srtDist && temp > 0)srtDist = temp;
//		}
//	}
	return srtDist;
}

void geoModel::calDomainSize(){

	//For now, the domain Frame is a hex box surrounding the model
	domainFrame[0] = domainFrame[3] = (*featureNodes)(0,0);
	domainFrame[1] = domainFrame[4] = (*featureNodes)(0,1);
	domainFrame[2] = domainFrame[5] = (*featureNodes)(0,2);

	//get domainFrame: left bottom point
	for( __int64 i=1; i<featureNodes->length; i++){
		if((*featureNodes)(i,0) < domainFrame[0])
			domainFrame[0] = (*featureNodes)(i,0);
		if((*featureNodes)(i,1) < domainFrame[1])
			domainFrame[1] = (*featureNodes)(i,1);
		if((*featureNodes)(i,2) < domainFrame[2])
			domainFrame[2] = (*featureNodes)(i,2);



	}

	//get domainFrame: right upper point
	for( __int64 i=1; i<featureNodes->length; i++){
		if((*featureNodes)(i,0) > domainFrame[3])
			domainFrame[3] = (*featureNodes)(i,0);
		if((*featureNodes)(i,1) > domainFrame[4])
			domainFrame[4] = (*featureNodes)(i,1);
		if((*featureNodes)(i,2) > domainFrame[5])
			domainFrame[5] = (*featureNodes)(i,2);
	}
	//Handle case if a circle or sphere in the model
	for( __int64 i=0; i<faces->length; i++){
		Face* curF = faces->getFace(i);
		if(curF->zDefFace == 0)
			continue;
		if(curF->type == 0x8){
			double temp;
			Sphere *s = (Sphere*) curF->f;
			for( __int64 j=0; j<3; j++){
				temp = s->center[j] - s->radius;
				if( temp < domainFrame[j] ) domainFrame[j] = temp;
				temp = s->center[j] + s->radius;
				if( temp > domainFrame[j+3] ) domainFrame[j+3] = temp;
			}
		}
	}

	//get domainSize
	domainSize[0] 	=  domainFrame[3] - domainFrame[0];
	domainSize[1] 	=  domainFrame[4] - domainFrame[1];
	domainSize[2] 	=  domainFrame[5] - domainFrame[2];
	domainCenter[0] = (domainFrame[0] + domainFrame[3])/2;
	domainCenter[1] = (domainFrame[1] + domainFrame[4])/2;
	domainCenter[2] = (domainFrame[2] + domainFrame[5])/2;

	//get the length of the model, which is the longest span in the model
	length = 0;
	for( __int64 i=0; i<3; i++)
		(domainSize[i]>length)? length = domainSize[i]: length;
}


/**
 * This function will find the face that intersect the cube
 *
 * @param x coordinates of the base po __int64 of the cube
 * @param length of the edge of the cube
 * @param id the array pointer to store the face id found
 * @param numF number of face found
 */

void geoModel::getInterFaceId(double x[3], double length, intArr** fId){

	if(!x){
		printf("Error: in getInterFaceId, pointer is NULL: x:%p \n", x);
		throw StoppingException("\n");
	}
	*fId = new intArr();

//	if(x[0]== -5 && x[1] == -5 &&x[2]==-5)
	intArr temp;
	Face * curF;
	for(  __int64 i=0; i<faces->length; i++){
		curF = faces->getFace(i);
		if( ! curF->cubeInterSectCheck(x, length) ) // no intersection, go to the next face
			continue;
		//intersects, add to the fId
		(*fId)->add(i);
	}

	if((*fId)->length==0){
		free(*fId);
		*fId = NULL;
	}

	return;




	/*

	The old method, uses dist function check if there are both positive and negative distance
	if there are, then intersect. The method will work for plane but may not work with polygon,
	furthermore, the calculation of dist is expensive.

	//
	 // First iterate through all the faces in the model
	 // calculate the distance between the center po __int64 of the cube
	 // and each of the face, if the distance is bigger than the biggest
	 // possible distance distMax (half of the diagonal distance of the cube),
	 // then ignore the face, because if the face intersect the cube
	 // the distance between the center po __int64 and the face must be
	 // smaller than this distMax
	 //
	double center[3] = { x[0]+length/2,  x[1]+length/2,  x[2]+length/2};
	double distMax = length * sqrt(3)/2;
	double distMin =length/2;
	//
	 / This flg is for the purpose of checking if all the distance has the same sign
	 // including the center po __int64 of the cube
	 // will first take the center po __int64 and then multiply the distance of
	 // each edge points, if flg less than zero, then intersects
	 //
	double dist;

	Face * curF;
	for(  __int64 i=0; i<faces->length; i++){
		curF = faces->getFace(i);
		dist = curF->distanceToPoint(center);
		if(fabs(dist)< distMin){
			(*fId)->add(i);
			continue;
		}
		if(fabs(dist) < distMax) {
//			double v[8][3];
//			v[0][0]= x[0];         	v[0][1]= x[1];			v[0][2]= x[2];
//			v[1][0]= x[0]+length; 	v[1][1]= x[1];			v[1][2]= x[2];
//			v[2][0]= x[0];		 	v[2][1]= x[1]+length;	v[2][2]= x[2];
//			v[3][0]= x[0]+length; 	v[3][1]= x[1]+length;	v[3][2]= x[2];
//			v[4][0]= x[0];         	v[4][1]= x[1];			v[4][2]= x[2]+length;
//			v[5][0]= x[0]+length; 	v[5][1]= x[1];			v[5][2]= x[2]+length;
//			v[6][0]= x[0];		 	v[6][1]= x[1]+length;	v[6][2]= x[2]+length;
//			v[7][0]= x[0]+length; 	v[7][1]= x[1]+length;	v[7][2]= x[2]+length;
//
			double v[3];
			 __int64 p=0, n=0;
			for( __int64 j=0; j<8; j++){
				v[0] = x[0] + length * fmod(j,2);
				v[1] = x[1] + length * fmod(j/2,2);
				v[2] = x[2] + length * fmod(j/4,2);
				dist = curF->distanceToPoint(v);
				if(dist>0)p=1;
				else if(dist<0)n=1;
//				else n=1;
				if(n*p != 0){
					(*fId)->add(i);
					break;
				}
			}
		}
	}
	if((*fId)->length==0){
		free(*fId);
		*fId = NULL;
	}
*/
}
/**
 * This function will find the face that intersect the cube
 * This function will look at only the known face id, check if any intersection exist
 *
 * The face id in the id set is the face in the two geofaceArr
 * zoneFaces comes first, any id larger than the length of the zoneFaces array length
 * is in the partFaces array, its id= id-zoneFaces->length
 *
 * @param x coordinates of the base po __int64 of the cube
 * @param length of the edge of the cube
 * @param id the array pointer to store the face id found
 * @param numF number of face found
 * @param idSet the list of face ids that is to be checked
 * @param numIdInSet the number of ids in the list of ids
 */

void geoModel::getInterFaceId(double x[3], double length, intArr** fId, intArr* idSet){

	if( !x ){
		printf("Error: in getInterFaceId, pointer is NULL: x:%p \n", x);
		throw StoppingException("\n");
	}

	if( !idSet )
		return getInterFaceId(x, length, fId);

	*fId = new intArr();

	Face * curF;
	intArr temp;
	for(  __int64 i=0; i<idSet->length; i++){
		curF = faces->getFace((*idSet)[i]);
		if( ! curF->cubeInterSectCheck(x, length) ) // no intersection, go to the next face
			continue;
		//intersects, add to the fId
		(*fId)->add((*idSet)[i]);
	}

	if((*fId)->length==0){
		free(*fId);
		*fId = NULL;
	}

	return;





	/**
	The old method, uses dist function check if there are both positive and negative distance
	if there are, then intersect. The method will work for plane but may not work with polygon,
	furthermore, the calculation of dist is expensive.
	*/


	/**
	 * First iterate through all the faces in the model
	 * calculate the distance between the center po __int64 of the cube
	 * and each of the face, if the distance is bigger than the biggest
	 * possible distance distMax (half of the diagonal distance of the cube),
	 * then ignore the face, because if the face intersect the cube
	 * the distance between the center po __int64 and the face must be
	 * smaller than this distMax
	 */

/*	double center[3] = { x[0]+length/2,  x[1]+length/2,  x[2]+length/2};
	double distMax = length * sqrt(3)/2;
	double distMin =length/2;
	double myEPSILON = EPSILON; */
	/**
	 * This flg is for the purpose of checking if all the distance has the same sign
	 * including the center po __int64 of the cube
	 * will first take the center po __int64 and then multiply the distance of
	 * each edge points, if flg less than zero, then intersects
	 */
/*	double dist;
	 __int64 i;
	Face* curF;
	for(  __int64 ii=0; ii<idSet->length; ii++){
		i=(*idSet)[ii];
		if(i<faces->length){
			curF = faces->getFace(i);
		}
		else {
			printf("Error: in getInterFaceId, idset contained id outof range : %d > %d \n", i, faces->length);
			throw StoppingException("\n");
		}


		dist = curF->distanceToPoint(center);
		if(fabs(dist)< distMin){
			(*fId)->add(i);
			continue;
		}
		if(fabs(dist) < distMax) {
			double v[3];
			 __int64 p=0, n=0;
			for( __int64 j=0; j<8; j++){
				v[0] = x[0] + length * fmod(j,2);
				v[1] = x[1] + length * fmod(j/2,2);
				v[2] = x[2] + length * fmod(j/4,2);
				dist = curF->distanceToPoint(v);
				if(dist>0)
					p=1;

//				else  n=1;
				else if(dist<0)
					n=1;
				if(n*p != 0){
					(*fId)->add(i);
					break;
				}
			}
		}
	}
	if((*fId)->length==0){
		free(*fId);
		*fId = NULL;
	}
*/
}

/**
 * This function will find the node that is inside the cube
 *
 * @param x coordinates of the base po __int64 of the cube
 * @param length of the edge of the cube
 * @param id the array pointer to store the face id found
 * @param numF number of face found
 */
void geoModel::getInnerNodeId(double x[3], double length, intArr** nId){

	if(!x){
		printf("Error: in getInnerNodeId, pointer is NULL: x:%p \n", x);
		throw StoppingException("\n");
	}
	*nId = new intArr();
	__int64 outflg = 0;
	double* curNode;
	for( __int64 i=0; i< featureNodes->length; i++){
		outflg = 0;
		curNode = featureNodes->getNode(i);

		for( __int64 j=0; j<3; j++){
			if( curNode[j]<x[j] || curNode[j]>(x[j]+length) ){
				outflg = 1; break;
			}
		}
		if(!outflg)
			(*nId)->add(i);
	}
	if((*nId)->length==0){
		free(*nId);
		*nId = NULL;
	}

}

/**
 * This function will find the node that is inside the cube
 *
 * @param x coordinates of the base po __int64 of the cube
 * @param length of the edge of the cube
 * @param id the array pointer to store the face id found
 * @param numF number of face found
 */
void geoModel::getInnerNodeId(double x[3], double length, intArr** nId, intArr* idSet){

	if(!x){
		printf("Error: in getInnerNodeId, pointer is NULL: x:%p \n", x);
		throw StoppingException("\n");
	}
	if( ! idSet )//no idset provided
		return getInnerNodeId(x, length, nId);
	*nId = new intArr();
	 __int64 outflg = 0, i;
	double *curNode;
	for( __int64 ii=0; ii< idSet->length; ii++){
		outflg = 0;
		i = (*idSet)[ii];
		curNode = featureNodes->getNode(i);
		for( __int64 j=0; j<3; j++){
			if( curNode[j]<x[j] || curNode[j]>(x[j]+length) ){
				outflg = 1; break;
			}
		}
		if(!outflg)
			(*nId)->add(i);
	}
	if((*nId)->length==0){
		free(*nId);
		*nId = NULL;
	}

}



/**
 * This function projects the po __int64 to the face given
 * @param x the coordinates of the points
 * @param fId the id of the face
 * @param p the coordinates of the projected point
 */

void geoModel::project2Face(double x[3],  __int64 fId, double p[3], __int64& bnd){
	if(fId<0 || fId >faces->length){
		printf("Error: in project2Face, face id out of range: id=%lld \n", fId);
		throw StoppingException("\n");
	}
	Face *curF = faces->getFace(fId);

	curF->project2Face(x, p, bnd);
}

/**
 * This function will return the zone or bnd id that the vertex belongs to
 *
 *
 *  then function will set the zone or bnd accordingly
 *
 *  for now: (07/06/2010) assume that the partfaces do not need a zone number
 *
 *
 *  @param x the coordinates of the vertex
 *  @param zone the returned zone id
 *  @param bnd the returned bnd id
 *
 *
 */
void geoModel::getZoneOrBndId( double x[3], __int64& zone, __int64& bnd ){

	double dire[3];
	__int64 nCrossPnt=0, tmp;
	dire[0]=1; dire[1] = 0; dire[2]=0;
	Face* curF;
	bnd=-1, zone=-1; // set default value for bnd and zone
	                 // -1 is the default value of not on the boundary for bnd
	                 // -1 is the default value of not inside the zone for zone


    for( __int64 i=0; i< zones->length; i++){
    	
		intArr* fc = zones->getZoneFaceArr(i);
    	
		for( __int64 j=0; j<fc->length; j++){
    		
			__int64 fId = (*fc)[j];
    		curF = faces->getFace(fId);
   			if( curF->zDefFace == 0)
   				continue;
			double *norm = curF->getPlanarFaceFunction();
			if( curF->type == 10 || curF->type ==3 || curF->type ==4 || curF->type ==5 ){
				
				if( fabs( fabs(norm[2]) - 1 ) < 0.01){ //horizontal plane
					continue;
				}

			}


    		tmp = curF->getNumIntersectPntWithRay(x, dire);
			
        	if(tmp==-1) {// on the boundary
        		bnd = curF->bnd;
				
        		return; // either bnd or zone need tobe set
        		        //once one is set, other does not need tobe updated
        	}
        	else
        		nCrossPnt += tmp;
    	}
        if(nCrossPnt%2 ==1) {// inside the zone
        	zone = i;
        	return;
        }
        else //outside this zone, will continue;
        	continue;
    }

    if(bnd==-1){
    	for( __int64 i=0; i<faces->length; i++){
    		curF = faces->getFace(i);
    		if(curF->zDefFace == 1)
    			continue;
 //   		if(i==161)
  //  			cout<<"JJJ"<<endl;
    		double temp = curF->distanceToPoint(x);
    		if(fabs(temp) < myEPSILON){
    			bnd = curF->bnd;
    			return;
    		}
    	}

    }




}




bool geoModel::isFaceZoneDef(__int64 fc){

    	Face* curF = faces->getFace(fc);
   			
        if( curF->zDefFace == 0)
   			 return false;

        else 
            return true;

}


/**
 * This function will return the two neighbouring zone id of the given bnd
 */
void geoModel::getBndNeiZone( __int64 b,  __int64 *z){
	for( __int64 i=0; i<bndNeiZone->length; i++){
		if( (*bndNeiZone)(i,0) != b)
			continue;
		else{
			z[0]= (*bndNeiZone)(i,1);
			z[1]= (*bndNeiZone)(i,2);
			return;
		}
	}
}


 __int64 geoModel::getFaceBnd( __int64 fId){
	if(fId<0 || fId >totlNumFace){
		printf("Error: in getFaceBnd, face id out of range: id=%lld \n", fId);
		throw StoppingException("\n");
	}
	Face *curF = faces->getFace(fId);
	return curF->bnd;
}

/**
 * This function will return the shortest distance of the po __int64 x[3] to all the faces
 * in the model
 */
double  geoModel::minDistanceToFace(double x[3]){
    if(totlNumFace<1){
    	fprintf(stderr, "Error: in minDistanceToFace, no face in the model.");
    	throw StoppingException("\n");
    }
	Face *curF;
	curF = faces->getFace(0);
	double min =curF->distanceToPoint(x);
    for( __int64 i=1; i<faces->length; i++){
    	curF = faces->getFace(i);
        double dist = curF->distanceToPoint(x);
        if(fabs(dist)<min)
        	min = fabs(dist);
    }
    return min;
}



/**
 *  This function will calculate the mid po __int64 of the intersection points
 *  of the intersection line of face a and b with the cube.
 *  This method will first search the existing array of intersection lines
 *  if found return the pointer
 *  if not found, then will calculate the intersection line
 *                  and add the line the intersection line array
 *
 *  @param a idx of face 1
 *  @param b idx of face 2
 *  @param cube coordinates of the cube
 *  @param intPnt the mid po __int64 of the two intersection points
 *
 */
bool geoModel::getMidPointOfIntersectionLineWithCube( __int64 a,  __int64 b, double cube[3], double size, geoNodeArr& intPnt){

    if(!interLines)
    	interLines = new interSectLines();
	__int64 n = interLines->getNumberOfInterLine(a, b);

    if(n==-1){
        vector<Face*> newLine;
        if( (faces->getFace(a))->getInterSectLine( (faces->getFace(b)), newLine)){
            interLines->addInterLine(a, b, newLine);
            n=newLine.size();
        }
        else{
            interLines->addInterLine(a, b);
            return false;
        }
    }
    if(n==0){
        return false;
    }

    Face *curLine;

    bool found= false;
    for( __int64 i=0; i<n; i++){
        curLine = interLines->getInterLine(a, b, i);
        double pnt[6];
        if(curLine->getInterSectPointWithCube(cube, size, pnt)){
        	found = true;
        	pnt[0] = (pnt[0]+pnt[3])/2;
        	pnt[1] = (pnt[1]+pnt[4])/2;
        	pnt[2] = (pnt[2]+pnt[5])/2;
        	intPnt.addNonDup(pnt);
        }

    }

    if(!found){ // not necessarily have intersection po __int64 with the cube
    	return false;
    }
	else if(intPnt.length!=1)
		return false;

    return true;

}

double geoModel::getDistance2Face( __int64 fc, double p[3]){
	return faces->getFace(fc)->distanceToPoint(p);
}

/**
 * This function will return the coordinates of the nodes, given a node id
 * The returned coordinates is a deep copy
 * @param id the id of the node
 * @param cor the array to hold the returned coordinates
 */
void geoModel::getNode( __int64 id, double cor[3]){
	cor[0] = (*featureNodes)(id, 0);
	cor[1] = (*featureNodes)(id, 1);
	cor[2] = (*featureNodes)(id, 2);
}



bool geoModel::modelOnGridCheck(double cSize){

	double myEPSILON = EPSILON;
	bool flg= false;
	double rCoord[3];

	for( __int64 i=0; i<featureNodes->length; i++){
		for( __int64 j=0; j<3; j++){
			rCoord[j] = (*featureNodes)(i,j) - domainCenter[j];
			double mod = fabs(fmod(rCoord[j], cSize)/cSize);
			//mod may be 0 or 1
			if( mod < myEPSILON || fabs(mod-1) < myEPSILON ){
				flg = true;
				return flg;
			}
		}
	}
	for( __int64 i=0; i<faces->length; i++){
		if(faces->getFace(i)->type == 8){//sphere
			Sphere* s = (Sphere*) faces->getFace(i)->f;
			double node[3] = {s->center[0] - s->radius,
					s->center[1] - s->radius,
					s->center[2] - s->radius};

			for( __int64 j=0; j<3; j++){
				rCoord[j] = node[j] - domainCenter[j];
				double mod = fabs(fmod(rCoord[j], cSize)/cSize);
				//mod may be 0 or 1
				if( mod < myEPSILON || fabs(mod-1) < myEPSILON ){
					flg = true;
					return flg;
				}
			}
		};

	}
	return flg;
}


void geoModel::printGeoFaces(){

	cout<<"total number of faces: "<<faces->length<<endl;;


	faces->print();

}

void geoModel::printGeoFeatureNodes(){

	cout<<"total number of nodes: "<<featureNodes->length<<endl;;

	featureNodes->print();



}


bool geoModel::featureNodeCheck(intArr* nd){

	for( __int64 i=0; i<nd->length; i++){
		if(featureNodes->featureNodeCheck( (*nd)[i] ) )
			return true;
	}
	return false;

}

bool geoModel::featureFaceCheck(intArr* fc){

     __int64 cur, bnd;
    for( __int64 i=0; i<fc->length; i++){
    	cur = (*fc)[i];
    	bnd = getFaceBnd(cur);
    	if(bnd>=-2)
    		return true;
    }
    return false;

}


/**
 * will return the plane function for planar face objects
 * 
 * This function will make a deep copy
 *
 */

void geoModel::getPlanarFaceFunction(__int64 fc, double equa[4]){

	double *ret=NULL;
	Face* curf = faces->getFace(fc);

	ret = curf->getPlanarFaceFunction();

	if(ret){
		equa[0] = ret[0];
		equa[1] = ret[1];
		equa[2] = ret[2];
		equa[3] = ret[3];
	}



}


/**
 * return the bnd sign of the face
 *
 * @param fc face id
 */
__int64 geoModel::getBndOfFace(__int64 fc){


	return faces->getFace(fc)->bnd;



}


bool geoModel::getCubeInterSectMidPoint(double *cb, double cSize, int fc, double mid[3]){


	return faces->getFace(fc)->getCubeInterSectMidPoint( cb, cSize, mid);

}

/**
 * Will also append the normals of the faces so that when translating to msh file, 
 * the program can determine the direction of the element face, then decide whether the face
 * belongs to the geoface or geoface's shaddow
 *
 */
void geoModel::printNormalInfoToBstrFile(char* fName){

    	char* fullFilePath = NULL;
        getFullFilePath(&fullFilePath, fName, ".bstr");
        FILE * file = NULL;

    	file = fopen(fullFilePath, "ab");

        if( !file ){
            fprintf(stderr, "error opening bstr file for appending.\n");
            throw StoppingException("\n");
        }
 		//for now will assume all patches are co-planar
		int len = 0;
		fwrite(&len, sizeof(int), 1, file);

		double norm[3];
    	for(int i=0; i<faces->length; i++){

			Face* curF = (*faces)[i];
        
    		if( curF->type > 2 && curF->type <6 ){

				Polygon* pp = (Polygon*) curF->f;

				norm[0] = pp->n[0];
				norm[1] = pp->n[1];
				norm[2] = pp->n[2];

				fwrite (norm , sizeof(double) , 3 , file );
					
			}
    		
    		else if (curF->type == 10){
    			PolygonPatch * pp = (PolygonPatch*) curF->f;
    			
				norm[0] = pp->polys[0]->n[0];
				norm[1] = pp->polys[0]->n[1];
				norm[2] = pp->polys[0]->n[2];
				fwrite (norm , sizeof(double) , 3 , file );
   			}
    	}

    	fclose(file);



}