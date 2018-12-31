/*
 * readVRMLFile.cpp
 *
 *  Created on: Jul 8, 2010
 *      Author: zrui
 */

#include "stdafx.h"


#include "geoModel.h"

//#include "stdafx.h"

/**
 * This function has limited support for the VRML file. It will only read the basic geometry types
 * IndexedFaceSet and Point
 *
 */


void geoModel::readVRMLFile(char* fName){


	VRMLFile* vf = new VRMLFile(fName, featureNodes, faces, bndStr);

	vf->loadFile();

}

/**
 * The faces in the VRML file are treated as partition faces as default
 * Thus no zone define faces in VRML file.
 * There is assumed only one zone for VRML file model.
 * This function will add the zone defining face
 * The size of the zone is derived by the size of the VRML file faces defined domain
 * The zone dimension will be 5 times of that of the VRML file face domain
 *
 * The global variable will be updated accordingly
 *
 *
 * Currently the requirements for the VRML model is that a bounding box on the ground
 * should be added. Because the "ground" boundary of the domain will be added in this
 * function, and the program does not deal with complex geometry at the ground level yet,
 * will assume it is a rectangular shaped surface, and will add four more faces to form
 * the "ground" boundary of the domain.
 */
void geoModel::addDomainDefFaceForVRMLFile(){



	if(domMode==2) { // ext by extMultiplier's times

		domainSize[0] *= extMultiplier;
		domainSize[1] *= extMultiplier;
		domainSize[2] *= extMultiplier;
/*		int mm = 0;
		mm = domainSize[mm]<domainSize[1] ? 1 : mm;
		mm = domainSize[mm]<domainSize[2] ? 2 : mm;
		double ratio[2] = {domainSize[(mm+1)%3]/domainSize[mm], domainSize[(mm+2)%3]/domainSize[1]};


		if(ratio[0]<0.3)
			domainSize[(mm+1)%3] = domainSize[mm]*0.3;
		if(ratio[1]<0.3)
			domainSize[(mm+2)%3] = domainSize[mm]*0.3;

*/

		//   double gFrmNd[2][4]={{0, domainFrame[0], domainFrame[3], 0},
		//                        {0, domainFrame[1], domainFrame[4], 0}};

		double oldFrame[6];
		for(int i=0; i<6; i++)
			oldFrame[i] = domainFrame[i];


		//the z direction lower bound should not be changes
		//because in most of the buildng simulation
		//the lower bound usually is the groud level, thus should not be extended
		//in this function, the center is shifted upwards to keep the lower bound the same
		domainCenter[2] = domainFrame[2] + domainSize[2]/2;




		domainFrame[0] = domainCenter[0] - domainSize[0]/2;
		domainFrame[1] = domainCenter[1] - domainSize[1]/2;
		//	domainFrame[2] = domainCenter[2] - domainSize[2]/2;  //z lower bound remains the same
		domainFrame[3] = domainCenter[0] + domainSize[0]/2;
		domainFrame[4] = domainCenter[1] + domainSize[1]/2;
		domainFrame[5] = domainCenter[2] + domainSize[2]/2;

		//add new node to the featureNodes array
		double newNode[8][3];
		for(int i=0; i<8; i++){
			newNode[i][0] = domainFrame[0] + i%2*domainSize[0];
			newNode[i][1] = domainFrame[1] + (i%4)/2*domainSize[1];
			newNode[i][2] = domainFrame[2] + i/4*domainSize[2];
			featureNodes->add(newNode[i], false);
		}


		//add zone defining face to faces array
		int face[6][4]={ {0,1,3,2},
		{0,4,5,1},
		{0,2,6,4},
		{7,5,4,6},
		{7,6,2,3},
		{7,3,1,5}};
		vector<double> info;
		for(int i=1; i<6; i++){ //add all domain face except the "ground"
			for(int j=0;j<4;j++){ // ith face, jth node of the ith face
				info.push_back( newNode[ face[i][j] ][0]);
				info.push_back( newNode[ face[i][j] ][1]);
				info.push_back( newNode[ face[i][j] ][2]);
			}

			info.push_back(0);
			info.push_back(-1);
			info.push_back(-1*(i+10));
			info.push_back(1);
			Face *newF = new Face(4, info);
			faces->add(newF);
			info.clear();
		}

		//add the "ground", will be a PolygonPatch


		info.clear();

		info.push_back(1);		//operation type 1:=addition -1:=subtraction
		info.push_back(4);		//number of polygon
		info.push_back(0);		//neighZone+
		info.push_back(0);		//neighZone+
		info.push_back(0);		//neighZone+
		info.push_back(0);      //neighZone+
		info.push_back(-1);		//neighZone-
		info.push_back(-1);		//neighZone-
		info.push_back(-1);		//neighZone-
		info.push_back(-1);		//neighZone-
		info.push_back(-10); 	//bnd 
		info.push_back(-10); 	//bnd 
		info.push_back(-10); 	//bnd 
		info.push_back(-10); 	//bnd 
		info.push_back(1); 		//zDefFace 
		info.push_back(1); 		//zDefFace 
		info.push_back(1); 		//zDefFace 
		info.push_back(1); 		//zDefFace


		//put in the first polygon

		info.push_back(4); 		// number of vertices

		info.push_back(domainFrame[0]);
		info.push_back(domainFrame[1]);
		info.push_back(domainFrame[2]);

		info.push_back(domainFrame[3]);
		info.push_back(domainFrame[1]);
		info.push_back(domainFrame[2]);

		info.push_back(oldFrame[3]);
		info.push_back(oldFrame[1]);
		info.push_back(oldFrame[2]);

		info.push_back(oldFrame[0]);
		info.push_back(oldFrame[1]);
		info.push_back(oldFrame[2]);


		//second polygon

		info.push_back(4); 		// number of vertices

		info.push_back(domainFrame[3]);
		info.push_back(domainFrame[1]);
		info.push_back(domainFrame[2]);

		info.push_back(domainFrame[3]);
		info.push_back(domainFrame[4]);
		info.push_back(domainFrame[2]);

		info.push_back(oldFrame[3]);
		info.push_back(oldFrame[4]);
		info.push_back(oldFrame[2]);

		info.push_back(oldFrame[3]);
		info.push_back(oldFrame[1]);
		info.push_back(oldFrame[2]);


		//third polygon

		info.push_back(4); 		// number of vertices

		info.push_back(domainFrame[3]);
		info.push_back(domainFrame[4]);
		info.push_back(domainFrame[2]);

		info.push_back(domainFrame[0]);
		info.push_back(domainFrame[4]);
		info.push_back(domainFrame[2]);

		info.push_back(oldFrame[0]);
		info.push_back(oldFrame[4]);
		info.push_back(oldFrame[2]);

		info.push_back(oldFrame[3]);
		info.push_back(oldFrame[4]);
		info.push_back(oldFrame[2]);


		//forth polygon

		info.push_back(4); 		// number of vertices

		info.push_back(domainFrame[0]);
		info.push_back(domainFrame[4]);
		info.push_back(domainFrame[2]);

		info.push_back(domainFrame[0]);
		info.push_back(domainFrame[1]);
		info.push_back(domainFrame[2]);

		info.push_back(oldFrame[0]);
		info.push_back(oldFrame[1]);
		info.push_back(oldFrame[2]);

		info.push_back(oldFrame[0]);
		info.push_back(oldFrame[4]);
		info.push_back(oldFrame[2]);


		Face *newF = new Face(0xa, info);
		faces->add(newF);
		info.clear();

		for(int i=0; i<3; i++)
			(domainSize[i]>length)? length = domainSize[i]: length;

	}

	else if(domMode==1) { // add domain but do not extend by five times

				
//		domainSize[0] *= 5;
//		domainSize[1] *= 5;
		domainSize[2] *= 1.5;

		//find the longest direction
		int mm = 0;
		mm = domainSize[mm]<domainSize[1] ? 1 : mm;
		mm = domainSize[mm]<domainSize[2] ? 2 : mm;
		double ratio[2] = {domainSize[(mm+1)%3]/domainSize[mm], domainSize[(mm+2)%3]/domainSize[1]};

		//adjust the domain, expend the domain size if it is too slim (<0.3)
		if(ratio[0]<0.3)
			domainSize[(mm+1)%3] = domainSize[mm]*0.3;
		if(ratio[1]<0.3)
			domainSize[(mm+2)%3] = domainSize[mm]*0.3;



		//   double gFrmNd[2][4]={{0, domainFrame[0], domainFrame[3], 0},
		//                        {0, domainFrame[1], domainFrame[4], 0}};

//		double oldFrame[6];
//		for(int i=0; i<6; i++)
//			oldFrame[i] = domainFrame[i];


		//the z direction lower bound should not be changes
		//because in most of the buildng simulation
		//the lower bound usually is the groud level, thus should not be extended
		//in this function, the center is shifted upwards to keep the lower bound the same
//		domainCenter[2] = domainFrame[2] + domainSize[2]/2;




		domainFrame[0] = domainCenter[0] - domainSize[0]/2;
		domainFrame[1] = domainCenter[1] - domainSize[1]/2;
		//	domainFrame[2] = domainCenter[2] - domainSize[2]/2;  //z lower bound remains the same
		domainFrame[3] = domainCenter[0] + domainSize[0]/2;
		domainFrame[4] = domainCenter[1] + domainSize[1]/2;
		domainFrame[5] = domainCenter[2] + domainSize[2]/2;

		//add new node ( the four nodes at the top of the domain) to the featureNodes array
		double newNode[8][3];
		for(int i=0; i<8; i++){
			newNode[i][0] = domainFrame[0] + i%2*domainSize[0];
			newNode[i][1] = domainFrame[1] + (i%4)/2*domainSize[1];
			newNode[i][2] = domainFrame[2] + i/4*domainSize[2];
			if( i>3 )
				featureNodes->add(newNode[i], false);
		}


		//add zone defining face to faces array
		int face[6][4]={ {0,1,3,2},
		{0,4,5,1},
		{0,2,6,4},
		{7,5,4,6},
		{7,6,2,3},
		{7,3,1,5}};
		vector<double> info;
		for(int i=1; i<6; i++){ //add all domain face except the "ground"
			for(int j=0;j<4;j++){ // ith face, jth node of the ith face
				info.push_back( newNode[ face[i][j] ][0]);
				info.push_back( newNode[ face[i][j] ][1]);
				info.push_back( newNode[ face[i][j] ][2]);
			}

			info.push_back(0);
			info.push_back(-1);
			info.push_back(-1*(i+10));
			info.push_back(1);
			Face *newF = new Face(4, info);
			faces->add(newF);
			info.clear();
		}

		info.clear();


		for(int i=0; i<3; i++)
			(domainSize[i]>length)? length = domainSize[i]: length;



	}

	else{ // no extension from the vrml file

		return;


	}

	return;




	/*
	info.clear();

	info.push_back(-1);		//operation type 1:=addition -1:=subtraction
	info.push_back(2);		//number of polygon
	info.push_back(0);		//neighZone+
	info.push_back(0);      //neighZone+
	info.push_back(-1);		//neighZone-
	info.push_back(0);    	//neighZone-
	info.push_back(-10); 	//bnd 
	info.push_back(-10); 	//bnd 
	info.push_back(1); 		//zDefFace 
	info.push_back(1); 		//zDefFace



	//put in the first polygon

	info.push_back(4); 		//number of vertices
	info.push_back(domainFrame[0]);
	info.push_back(domainFrame[1]);
	info.push_back(domainFrame[2]);
	info.push_back(domainFrame[3]);
	info.push_back(domainFrame[1]);
	info.push_back(domainFrame[2]);
	info.push_back(domainFrame[3]);
	info.push_back(domainFrame[4]);
	info.push_back(domainFrame[2]);
	info.push_back(domainFrame[0]);
	info.push_back(domainFrame[4]);
	info.push_back(domainFrame[2]);



	//second polyong	
	info.push_back(4);
	info.push_back(oldFrame[0]);
	info.push_back(oldFrame[1]);
	info.push_back(oldFrame[2]);
	info.push_back(oldFrame[3]);
	info.push_back(oldFrame[1]);
	info.push_back(oldFrame[2]);
	info.push_back(oldFrame[3]);
	info.push_back(oldFrame[4]);
	info.push_back(oldFrame[2]);
	info.push_back(oldFrame[0]);
	info.push_back(oldFrame[4]);
	info.push_back(oldFrame[2]);

	Face *newF = new Face(0xa, info);
	faces->add(newF);
	info.clear();

*/







	/*    gFrmNd[0][0]= domainFrame[0];  gFrmNd[0][3] = domainFrame[3];
    gFrmNd[1][0]= domainFrame[1];  gFrmNd[1][3] = domainFrame[4];


    int ff[4][4][2] = { {{1,1}, {1,0}, {3,0}, {3,1}},
                        {{2,1}, {3,1}, {3,3}, {2,3}},
                        {{2,2}, {2,3}, {0,3}, {0,2}},
                        {{1,2}, {0,2}, {0,0}, {1,0}},
                      };
    for(int i=0; i<4; i++){

		for(int j=0;j<4;j++){ // ith face, jth node of the ith face
			info.push_back( gFrmNd[ 0 ][ ff[i][j][0] ] );
			info.push_back( gFrmNd[ 1 ][ ff[i][j][1] ] );
			info.push_back( domainFrame[2] );
		}
		info.push_back(0);
		info.push_back(-1);
		info.push_back(-10);//bnd sign for "ground"
		info.push_back(1);
		Face *newF = new Face(4, info);
		faces->add(newF);
		info.clear();
	}

*/





}





