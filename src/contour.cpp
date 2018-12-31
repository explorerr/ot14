/*
 * contour.cpp
 *
 *  Created on: May 31, 2010
 *      Author: zrui
 */
#include "stdafx.h"
#include "Contour.h"


Contour::Contour( geoModel* GM, OcTree *ot, vtxArr* v, elemArr* e, faceArr* f, char* fName, bool binary ) {

	this->GM    = GM;
	this->ot    = ot;
	this->vtx   = v;
	this->elem  = e;
	this->face  = f;

	this->fName = fName;

	elemFaces[0][0] = 0; elemFaces[0][1] = 2; elemFaces[0][2] = 6; elemFaces[0][3] = 4;
    elemFaces[1][0] = 0; elemFaces[1][1] = 4; elemFaces[1][2] = 5; elemFaces[1][3] = 1;
    elemFaces[2][0] = 0; elemFaces[2][1] = 1; elemFaces[2][2] = 3; elemFaces[2][3] = 2;
    elemFaces[3][0] = 7; elemFaces[3][1] = 3; elemFaces[3][2] = 1; elemFaces[3][3] = 5;
    elemFaces[4][0] = 7; elemFaces[4][1] = 6; elemFaces[4][2] = 2; elemFaces[4][3] = 3;
    elemFaces[5][0] = 7; elemFaces[5][1] = 5; elemFaces[5][2] = 4; elemFaces[5][3] = 6;

    twoDisconFaceCells   = new intArr();
    multiDisconFaceCells = new intArr();
    multiVtxCells        = new intArr(); // data structure to hold all the twoVtxCells

	this->binary = binary;


}


Contour::~Contour() {

	delete(twoDisconFaceCells);
	delete(multiDisconFaceCells);
	delete(multiVtxCells);

}

void Contour::doContour(){

		
	char* fullFilePath = NULL;

	if(!binary){
		getFullFilePath(&fullFilePath, fName, ".mesh");
		meshFile = fopen(fullFilePath, "w");
		if(!meshFile)
			throw StoppingException("Can not open the mesh file for output.\n");
		fclose(meshFile);
		meshFile = fopen(fullFilePath, "a");

		if(!meshFile)
			throw StoppingException("Can not open the mesh file for output.\n");

	}
	else{
		getFullFilePath(&fullFilePath, fName, ".bmesh");
		meshFile = fopen(fullFilePath, "wb");
		if(!meshFile)
			throw StoppingException("Can not open the mesh file for output.\n");
		fclose(meshFile);
		meshFile = fopen(fullFilePath, "ab");
		if(!meshFile)
			throw StoppingException("Can not open the mesh file for output.\n");

	}

	cout<<"getVtx....."<<endl;

	getVtx();

	processMultiVticesCells();

		
	int zero = 0;
	if(binary)
		fwrite(&zero, sizeof(int), 1, meshFile);



	cout<<"getElements....."<<endl;

 	getElements();

	if(binary){
		fwrite(&numVtx,  sizeof(__int64), 1, meshFile);
		fwrite(&numElem, sizeof(__int64), 1, meshFile);
	}
	else{
		fprintf(meshFile, "%lld %lld\n", numVtx, numElem);
	}

	
	free(fullFilePath);
	fclose(meshFile);


	cout<<"number of Vtx="<<numVtx<<endl;

	cout<<"number of Element="<<numElem<<endl;
}

void Contour::getVtx(){


    numVtx = 0;

	ocCellArr::ocCell *cur;
	__int64 curOcId, curLvl;

	for(__int64 i=0; i<ot->cells->length; i++){

		cur = ot->cells->getCell(i);
//		if(cur->coll != 0)
//			continue;
		if(cur->zone == -1)
			continue;
		curOcId = cur->ocId;
		curLvl = ot->getLvlOfOcId(curOcId);


		if( i%10000==0){
        	cout<<"get vtx for "<<curOcId<<" cell id: "<<i<<" of "<<ot->cells->length<<endl;
			fflush(meshFile);
		}


		__int64 ii=76;
		ii *= 1000000;
		ii +=  944193;
		if(curOcId == ii){
			cout<<"getting vtx of "<<cur->ocId<<endl;
			cout<<"bnd= "<<cur->bnd<<endl;
	//		cout<<"coll= "<<cur->coll<<endl;
			if(cur->fc){
				cout<<"fc_length= "<<cur->fc->length<<endl;
				cout<<"face are: "<<endl;
				for(__int64 i=0; i<cur->fc->length; i++){
					cout<<(*cur->fc)[i]<<endl;
				}
			}
			else
				cout<<"fc_length= "<<0<<endl;
			if(cur->nd)
				cout<<"nd_length= "<<cur->nd->length<<endl;
			else
				cout<<"nd_length= "<<0<<endl;
			cout<<"zone= "<<cur->zone<<endl;

			double base[3], size;
			ot->getBaseCellNodeCoord(cur->ocId, base);
			size = ot->getSizeOfCellAtLvl(curLvl);
			cout<<"Base: "<<base[0]<<", "<<base[1]<<", "<<base[2]<<endl;
			cout<<"Size: "<<size<<endl;

			cout<<"Vtx length: "<<vtx->length<<endl;

			cout<<endl;

		} 


		double v[3];
		if(!(cur->fc) && !(cur->nd)){ // not on the boundary will add center point
			double x[3], size;
			ot->getBaseCellNodeCoord(curOcId, x);
			size = ot->getSizeOfCellAtLvl(curLvl)/2;
			v[0] = x[0] + size;
			v[1] = x[1] + size;
			v[2] = x[2] + size;

			addVtx(v,	 			//coordinate
					curOcId, 	    //ocId
					i);            //cellId
			continue;

		}

		if(cur->nd && cur->nd->length == 1){// has node inside, then use the node as vtx
			GM->getNode( (*(cur->nd))[0], v);

			addVtx(v,
					curOcId,
					i); 
						 
			continue;
		}
		else if(cur->fc && cur->fc->length == 1){ //has one face

			findVtx41Face(cur);
			continue;

		}
		else if(cur->fc && cur->fc->length == 2) {//two faces, should work this issue further
			                                      //how about multiple faces. it is possible?
			findVtx42Face(cur);
			continue;

		}
		else if(cur->fc && cur->fc->length > 2) {//two faces, should work this issue further
			                                      //how about multiple faces. it is possible?
			findVtx4MultiFace(cur);
			continue;

		}
		else{

			fprintf(stdout, "ocId = %lld, numFC= %lld,  numND = %lld\n", cur->ocId, cur->fc->length, cur->nd->length);
    		throw StoppingException("Error: in getVtx, multiple faces or nodes in one cell, "
   				                           "not known situation.\n");
		}
	} 


	//fclose(meshFile);

}





/**
 * This function will calculate the vtx position when there is one face passing through
 * the cube
 * Method:
 * 	>find all the intersection points between the cube and the face, (will use an approximation method)
 * 		>> calculate the distance and then, find all sign change edges, and find the intersection points
 * 	>calculate the gravity center of all the intersection points
 * 	>project the gravity center to the face
 *
 *
 */


void Contour::findVtx41Face(ocCellArr::ocCell *cur){


	__int64 curOcId = cur->ocId;


	if(!cur->fc){
		printf("Error: in findVtx41Face, no face found: %lld\n", curOcId);
		throw StoppingException("\n");
	}

	__int64 fc = (*(cur->fc))[0];

	double v[3];
	__int64 bnd;
    findVtx41FaceHelper(curOcId, fc, v, bnd);


	addVtx(v, cur);



}

void Contour::findVtx41FaceHelper(__int64 ocId, __int64 fc, double v[3], __int64& bnd){

	/*

	double vals[8], cx[8][3];
	__int64 curOcId = ocId;

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


	ot->getFullCellNodeCoord(curOcId, cx);

	// calculate the intersect point
	// since the face can be any form the number of intersection point is not determined
	// However, for plane, the intersection point can be 3, 4, 5, 6 for regular case,
	// or can be a corner, an edge, or a face of the cube
	// assumption for now is that there will be maximumly 6 intersection points (not quite?)

	double corner[12][3];
	__int64 cnt=0;
	for(__int64 i=0; i<8; i++)
		vals[i] = GM->getDistance2Face(fc, cx[i]);

	for(__int64 i=0; i<12; i++){

		if(vals[edge[i][0]]*vals[edge[i][1]]<0){
			double t = fabs(vals[edge[i][0]])/fabs(vals[edge[i][0]]-vals[edge[i][1]]);
			corner[cnt][0] = cx[edge[i][0]][0] + t*(cx[edge[i][1]][0] - cx[edge[i][0]][0]);
			corner[cnt][1] = cx[edge[i][0]][1] + t*(cx[edge[i][1]][1] - cx[edge[i][0]][1]);
			corner[cnt][2] = cx[edge[i][0]][2] + t*(cx[edge[i][1]][2] - cx[edge[i][0]][2]);
			cnt++;
		}
	}

	double mid[3]= {0};
	for(__int64 i=0; i<cnt; i++){
		mid[0] += corner[i][0];
		mid[1] += corner[i][1];
		mid[2] += corner[i][2];
	}

	mid[0] /= cnt;
	mid[1] /= cnt;
	mid[2] /= cnt;
	*/

	double mid[3];
	double cb[3];
	ot->getBaseCellNodeCoord(ocId, cb);
	double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(ocId));

	GM->getCubeInterSectMidPoint(cb, size, fc, mid);

	GM->project2Face(mid, fc, v, bnd);


}




/**
 * This function will calculate the vtx position where there are two faces in the
 * cube, this two face shuold be intersecting face, since the octree bottom level
 * choice is based minimum distance in the model, so there are multiple faces in the
 * cube the faces must be intersecting each other.
 *
 * This method will first calculate the intersection line between the two face
 * then try to find the line intersection point with the cube, and then calculate the
 * mid point of the two intersection point, and then project the mid point to the
 * inter section line
 *
 *
 *
 */

void Contour::findVtx42Face(ocCellArr::ocCell *cur){


	__int64 curOcId = cur->ocId;


	if(!cur->fc){
		printf("Error: in findVtx42Face, no face found: %lld\n", curOcId);
		throw StoppingException("\n");
	}
	if(cur->fc->length<2){
		printf("Error: in findVtx42Face, less than two faces found: id=%lld numFc=%lld\n", curOcId, cur->fc->length);
		throw StoppingException("\n");
	}


	geoNodeArr v(5);


	if (! findVtx42FaceHelper(curOcId, (*(cur->fc))[0], (*(cur->fc))[1], v) ){ // helper failed to find the intersection

//		twoDisconFaceCells->add(curOcId);


		multiVtxCells->add(cur->ocId);

		return;
	}

	if(v.length != 1){
		printf("Error: in findVtx42Face, number of vertex found not equal to 1: %lld\n", curOcId);
		printf("        face are:");
		for(__int64 i=0; i<2; i++)
			printf("[%lld], ", (*(cur->fc))[i]);
		printf("\n");
		double cx[3];
		ot->getBaseCellNodeCoord(curOcId, cx);
		printf("Cell base is: (%f, %f, %f)\n", cx[0], cx[1], cx[2]);
		printf("Cell level is: %lld\n Cell size: %f\n", ot->getLvlOfOcId(curOcId), ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(curOcId)));

		throw StoppingException("\n");

	}


	addVtx(v.getNode(0), cur);


}





/*
void Contour::findVtices4MultiVtxCell(ocCellArr::ocCell *cur){


	__int64 curOcId = cur->ocId;
	twoVtxCells->add(curOcId);  // add this cell to the twoVtxCell array


	intArr f;
	geoNodeArr v(2); // to store the vtx found
	intArrSorted vface[2]; // array to hold all the faces that the vertex is attached to
	intArrSorted pp, np; // array to hold the faces that have intersection lines and the faces do not have any intersection line with others

	for(__int64 j=0; j<ot->getFc(curOcId)->length; j++){
		f[j] = ( * ot->getFc(curOcId) )[j];
	}

    __int64 vCnt=0, vId[2];
	for(__int64 j=0; j<f.length; j++){
		for(__int64 k=j+1; k<f.length; k++ ){
			geoNodeArr vv(5);//temp array to store all the vertices found
			if( findVtx42FaceHelper(curOcId, f[j], f[k], vv) ){//there are intersections
				                                               //put j, and k into processed array
				                                               //put the vtx to vtxarray
				pp.insert(f[j]);
				pp.insert(f[k]);
                for(__int64 ll=0; ll<vv.length; ll++){
					__int64 vIdTemp = v.addNonDup(vv.getNode(ll));
                    if( vCnt==0 ){
                        vId[0] = vIdTemp;
                        vCnt++;
                    }
                    for(__int64 mm=0; mm<vCnt; mm++){//find which vface array to put in the current faces
                        if( vId[mm] == vIdTemp ){
                           vface[mm].insert(f[j]);
                           vface[mm].insert(f[k]);
                           break;
                        }
                        else if(mm==1){
                            printf("Error in findVtices4MultiVtxCell, number of vertex found larger than 2, ocId=%lld.\n", curOcId);
                    		printf("face are: [");
                    		for(int p=0; p<f.length; p++)
                    			cout<<f[p]<<", ";
                    		cout<<endl;
                    		double cx[3];
                    		ot->getBaseCellNodeCoord(curOcId, cx);
                    		double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(curOcId));
                    		printf("Cell base is: (%f, %f, %f)\n", cx[0], cx[1], cx[2]);
                    		printf("Cell level is: %lld\nCell size: %f\n", ot->getLvlOfOcId(curOcId), size);
                                     throw StoppingException("\n");
                        }
                        else if(mm==0 && vCnt==1){
                            vCnt++;
                            vId[1] = vIdTemp;
                        }
                    }
				}
			}
		}
	}
	if(v.length>2){
    	printf("Error in findVtices4MultiVtxCell, situation not handled, cells has more than 2 vtx, ocId=%lld.\n", curOcId );
		printf("face are: [");
		for(int p=0; p<f.length; p++)
			cout<<f[p]<<", ";
		cout<<endl;
		double cx[3];
		ot->getBaseCellNodeCoord(curOcId, cx);
		double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(curOcId));
		printf("Cell base is: (%f, %f, %f)\n", cx[0], cx[1], cx[2]);
		printf("Cell level is: %lld\nCell size: %f\n", ot->getLvlOfOcId(curOcId), size);
    	throw StoppingException("\n");
	}
	//check if there are faces not processed (no intersections)
	if( pp.length != f.length ){//there are faces not processed
		for(__int64 j=0; j<f.length; j++){
			if(pp.has(f[j]))
				continue;
			np.insert(f[j]);
		}
	}
	if(np.length>2){
    	printf("Error in findVtices4MultiVtxCell, situation not handled, cells has more than 2 discon faces, ocId=%lld.\n", curOcId );
		printf("face are: [");
		for(int p=0; p<f.length; p++)
			cout<<f[p]<<", ";
		cout<<endl;
		double cx[3];
		ot->getBaseCellNodeCoord(curOcId, cx);
		double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(curOcId));
		printf("Cell base is: (%f, %f, %f)\n", cx[0], cx[1], cx[2]);
		printf("Cell level is: %lld\nCell size: %f\n", ot->getLvlOfOcId(curOcId), size);
    	throw StoppingException("\n");

	}

	if(np.length + v.length >2){
    	printf("Error in findVtices4MultiVtxCell, situation not handled, cell has more than 2 discon faces and vtx, ocId=%lld.\n", curOcId );
		printf("face are: [");
		for(int p=0; p<f.length; p++)
			cout<<f[p]<<", ";
		cout<<endl;
    	double cx[3];
		ot->getBaseCellNodeCoord(curOcId, cx);
		double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(curOcId));
		printf("Cell base is: (%f, %f, %f)\n", cx[0], cx[1], cx[2]);
		printf("Cell level is: %lld\nCell size: %f\n", ot->getLvlOfOcId(curOcId), size);
    	throw StoppingException("\n");

	}

	//now there should be no more than 2 vtx

	if(np.length>0){
		for(__int64 j=0; j<np.length; j++){

			double vv[3];
			__int64 vIdTemp;
		    findVtx41FaceHelper(curOcId, np[j], vv, vIdTemp);
			v.addNonDup(vv);

            if( vCnt==0 ){
                vId[0] = vIdTemp;
                vCnt++;
                vface[0].insert(np[j]);
            }

            for(__int64 mm=0; mm<vCnt; mm++){//find which vface array to put in the current faces
                if( vId[mm] == vIdTemp ){
                   vface[mm].insert(f[j]);
                   break;
                }
                else if(mm==1){
                    printf("Error in findVtices4MultiVtxCell, number of vertex found larger than 2, ocId=%lld.", curOcId);
            		printf("face are: [");
            		for(int p=0; p<f.length; p++)
            			cout<<f[p]<<", ";
            		cout<<endl;
            		double cx[3];
            		ot->getBaseCellNodeCoord(curOcId, cx);
            		double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(curOcId));
            		printf("Cell base is: (%f, %f, %f)\n", cx[0], cx[1], cx[2]);
            		printf("Cell level is: %lld\nCell size: %f\n", ot->getLvlOfOcId(curOcId), size);
                    throw StoppingException("\n");
                }
                else if(mm==0){
                    vCnt++;
                    vId[1] = vIdTemp;
                }
            }
		}
	}

	double* x;
	intArr bnds;

	for(int i=0; i<v.length; i++){

		x = v.getNode(i);
		for(int j=0; j<vface[i].length; j++){
			bnds.add( GM->getBndOfFace(vface[i][j]) );
		}
		vtx->add(x);
		vtx->setOcId(vtx->length-1, curOcId);
		vtx->setZone(vtx->length-1, ot->getZone(curOcId));
		vtx->setBnd(vtx->length-1, &bnds); // deep copy
		twoVtxCells->add(vtx->length-1); // push_back the id of the vtx in the vtx array
		bnds.clear();
	}

}

*/

bool Contour::findVtx42FaceHelper(__int64 ocId, __int64 f1, __int64 f2, geoNodeArr& v){



	double cx[3];
	ot->getBaseCellNodeCoord(ocId, cx);

	double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(ocId));

	return GM->getMidPointOfIntersectionLineWithCube(f1, f2, cx, size, v);


}



void Contour::findVtx4MultiFace(ocCellArr::ocCell *cur){

	__int64 curOcId = cur->ocId;

	if(!cur->fc){
		printf("Error: in findVtx4MultiFace, no face found: %lld\n", curOcId);
		throw StoppingException("\n");
	}
	if(cur->fc->length<3){
		printf("Error: in findVtx4MultiFace, less than three faces found: id=%lld numFc=%lld\n", curOcId, cur->fc->length);
		throw StoppingException("\n");
	}

	__int64 numFc = cur->fc->length;

	geoNodeArr intPnt(3);

	findVtx4MultiFaceHelper(cur, intPnt);

	if( intPnt.length < 1 ){
		printf("Error: in findVtx4MultiFace, no intersection point found: ocId=%lld\n", curOcId);
		printf("        face are:");
		for(__int64 i=0; i<numFc; i++)
			printf("[%lld], ", (*(cur->fc))[i]);
		printf("\n");
		double cx[3];
		ot->getBaseCellNodeCoord(curOcId, cx);
		printf("Cell base is: (%f, %f, %f)\n", cx[0], cx[1], cx[2]);
		printf("Cell level is: %lld\n Cell size: %f\n", ot->getLvlOfOcId(curOcId), ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(curOcId)));
		return findVtx41Face(cur);
	}

	if(intPnt.length != 1 )
		multiDisconFaceCells->add(cur->ocId);
	else
		addVtx( intPnt.getNode(0), cur);

}

void Contour::findVtx4MultiFaceHelper(ocCellArr::ocCell *cur, geoNodeArr &intPnt){

	__int64 curOcId = cur->ocId;

	double cx[3];
	ot->getBaseCellNodeCoord(curOcId, cx);

	double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(curOcId));

	__int64 numFc = cur->fc->length;


	geoNodeArr tempPnt(3);

	intArrSorted pp, np;// array to hold the faces that have intersection lines and the faces do not have any intersection line with others

	for(__int64 i=0; i<numFc; i++){
		for(__int64 j=i+1; j< numFc; j++){
			if (GM->getMidPointOfIntersectionLineWithCube((*(cur->fc))[i], (*(cur->fc))[j], cx, size, tempPnt)){
				pp.insert((*(cur->fc))[i]);
				pp.insert((*(cur->fc))[j]);
				for(__int64 k=0; k<tempPnt.length; k++){
					intPnt.addNonDup(tempPnt.getNode(k));
					if(intPnt.length>1){ // one than one vtx found

						multiVtxCells->add(curOcId);
						//findVtices4MultiVtxCell(cur);
						return;
					}

				}
			}
		}
	}

	if( pp.length != cur->fc->length ){//there are faces not processed
		for(__int64 j=0; j<cur->fc->length; j++){
			if(pp.has((*(cur->fc))[j]))
				continue;
			np.insert((*(cur->fc))[j]);
		}
	}
	if(np.length+intPnt.length>1){ // more than one face not processed, then there should be more than one vtx for this cell
	//	multiDisconFaceCells->add(cur->ocId);
		multiVtxCells->add(cur->ocId);
		return;
	}




}



/**
 * This function will check the zone id for each vertex
 *
 * Methodology:
 *
 * if zone id not set (-1),
 * 		then call checkZoneId in GeoModel
 * if not on the boundary
 *      then update the neighboring vertices, who is not on the boundary (no inner node, no inter face)
 *
 *
 *
 */
/*
void Contour::getVxtZoneId(){

	for(__int64 i=0; i<vtx->length; i++){

		__int64 curZ = vtx->getZone(i);
		if(curZ == -2 ){//not set yet
            __int64 ocId = vtx->getOcId(i);
            curZ = ot->getZone(ocId);
            vtx->setZone(i, curZ);
		}

	}
}
*/

/**
 *
 * @param x the coordinates of the vertex
 * @param ocId the ocId of the cell
 * @param fc the faces in this cell
 *
 */


void Contour::addVtx(double* x, __int64 ocId, intArr* fc){

	numVtx++;

	__int64 zone = ot->getZone(ocId);

	ot->setVtx(ocId, numVtx-1);
	intArr bnd;
	if(!fc){
		bnd.add(-1);
	}
	else {
		__int64 b;
		for(__int64 i=0; i<fc->length; i++){
			b = GM->getBndOfFace( (*fc)[i] );
			bnd.add(b);
		}
	}
	ot->setBnd(ocId, &bnd); // deep copy;
	
	if(binary){
		double vBuf[3] ={x[0], x[1], x[2]};
		int bBuf[50];
		int numD = 1 + bnd.length;
		bBuf[0]= zone;
		for(__int64 j=0; j<bnd.length; j++)
			bBuf[j+1] = bnd[j] ;
		fwrite(&numD, sizeof(int), 1, meshFile);
		fwrite(vBuf, sizeof(double), 3, meshFile);
		fwrite(bBuf, sizeof(int), numD, meshFile);
	}
	else {
		
		fprintf(meshFile, "%f %f %f %lld ",	x[0], x[1], x[2], zone);
			
		for(__int64 j=0; j<bnd.length; j++)
			fprintf(meshFile, "%lld ", bnd[j] );
		fprintf(meshFile, "\n");
	}
	
		
	
		
}

void Contour::addVtx(double* x, ocCellArr::ocCell *cur){

	numVtx++;

	__int64 zone = cur->zone;

	cur->vtx = numVtx-1;
	
	if(binary){
		double vBuf[3] ={x[0], x[1], x[2]};
		int bBuf[50];

		bBuf[0]= zone;
		
		int numD;
		
		if(cur->fc){
			numD = 1 + cur->fc->length; // there is face in this cell will set the bnd as the same as the faces
			for(__int64 j=0; j<cur->fc->length; j++)
				bBuf[j+1] = GM->getBndOfFace( (*cur->fc)[j] );
		}
		else{
			numD = 2; // there is no faces in this cell, will set bnd as -1 (not on any bnd)
			bBuf[1] = -1;
		}

		fwrite(&numD, sizeof(int), 1, meshFile);
		fwrite(vBuf, sizeof(double), 3, meshFile);
		fwrite(bBuf, sizeof(int), numD, meshFile);
	}
	else {
		
		fprintf(meshFile, "%f %f %f %lld ",	x[0], x[1], x[2], zone);
				
		if(cur->fc){
			 // there is face in this cell will the set the bnd the same as the faces
			for(__int64 j=0; j<cur->fc->length; j++)
				fprintf(meshFile, "%lld ", GM->getBndOfFace( (*cur->fc)[j] )); 
		}
		else{
			fprintf(meshFile, "-1 ");  // there is no faces in this cell, will set bnd as -1 (not on any bnd)
		}	
		
		fprintf(meshFile, "\n");
	}
	
		
		
}



/** This function will set the appropriate data for both the current cell
  * This function does not take any bnd sign, it is assumed that the bnds are the same as the face array
  */

void Contour::addVtx(double* x, __int64 ocId, __int64 cellId){

	ocCellArr::ocCell *cur;

	cur = ot->cells->getCell(cellId);

	addVtx(x, cur);
	
		
}





void Contour::getElements(){

	char* fullFilePath = NULL;

	numElem = 0;

	ocCellArr::ocCell *cur;
	__int64 curOcId, curLvl;

	for(__int64 i=0; i<ot->cells->length; i++){


		cur = ot->cells->getCell(i);
//		if(cur->zone == -1 || cur->coll != 0)
		if(cur->zone == -1 )
			continue;
		curOcId = cur->ocId;
		curLvl = ot->getLvlOfOcId(curOcId);

     	__int64 ii, jj, kk;

        ot->ocId2ijk(curOcId, curLvl, ii, jj, kk);

        if( ii==0 || jj ==0 || kk ==0  ){ //node on the bottom and side faces, no elements around them
			continue;
        }
        generateElemForOneNode(curOcId);

        if( i%10000==0){
        	cout<<"get element for "<<curOcId<<" cell id: "<<i<<" of "<<ot->cells->length<<endl;
			fflush(meshFile);
		}

		

        if(ii%2!=0 || jj%2!=0 || kk%2!=0){

			__int64 diags[3][3]={ {0, 1, 1},
									{1, 0, 1},
									{1, 1, 0},
			};
			__int64 rep[3];
			__int64 d[3];
			bool added = false;
			__int64 dim = 1<<curLvl;
			for(__int64 j=0; j<3; j++){
				added = false;
				d[0] = ii+diags[j][0];
				d[1] = jj+diags[j][1];
				d[2] = kk+diags[j][2];
				rep[0] = ot->ijk2OcId(d[0], d[1]-1, d[2]-1, curLvl);
				rep[1] = ot->ijk2OcId(d[0]-1, d[1], d[2]-1, curLvl);
				rep[2] = ot->ijk2OcId(d[0]-1, d[1]-1, d[2], curLvl);
				for(__int64 k=0; k<j; k++){
					if(ot->getColl(rep[k]) == 0){
						added = true;
						break;
					}
				}
				if(added)
					continue;
				if(d[0]>=dim || d[1]>=dim || d[2]>=dim)
					continue;
				if(d[0]%2!=0 || d[1]%2 != 0 || d[2]%2 != 0){
					__int64 diag = ot->ijk2OcId(d[0], d[1], d[2], curLvl);
					if(ot->getColl(diag) == -1)
						generateElemForOneNode(diag);
				}
			}
        }
	}



//    fclose(meshFile);
 //   free(fullFilePath);

}


void Contour::generateElemForOneNode(__int64 ocId){


	__int64 cell[8];

    if( !ot->getNeighLeafCellVtx(ocId, cell) ) //if number of positive vtx is less than 5
			return;

//    cout<<"getting element of coId: "<<ocId<<endl;
//    if ( checkAndUpdateAllVtxOnBndButOnDiffentBndElem(cell) ){
 //   	return;
 //   }


	if(binary){
		int ret = fwrite(cell, sizeof(__int64), 8, meshFile);
		if(ret!=8)
			throw StoppingException("Error in writing element to binary file");
	}
	else {
		fprintf(meshFile, "%lld %lld %lld %lld %lld %lld %lld %lld \n", 
							cell[0], cell[1], cell[2], cell[3], cell[4], cell[5], cell[6], cell[7]);
	}
	
	numElem++;


}

/**
 * This function will check if the cell is all on one face
 *
 * if it is then return true.
 *
 *
*/
/*
bool Contour::checkAndUpdateAllVtxOnSameBndElem(__int64 cell[8]){

	if(!cell){
		printf("Error: in getNumberVtxOnBnd, cell array is NULL\n");
		throw StoppingException("\n");
	}

	intArr *bnd[8];
	intArr bDup;
	intArr bDupCnt;

	for(__int64 i=0; i<8; i++){
		bnd[i] = ot->getBnd(cell[i]);
	}

	getDup(bnd,  8, -3, &bDup, &bDupCnt);

	__int64 common = -3;
	bool flg= false;//found common bnd sign;
	bool moreThanOneCommon = false;
	for(__int64 i=0; i<bDupCnt.length; i++){
		if(bDupCnt[i]==8){
			if(flg){//already found one common
				moreThanOneCommon = true;
			}

			common = bDup[i];
			flg = true;
		}
	}



	return false;

}

*/

/**
  * This method will process all the cells with two disconneting faces
  *
  * The method will first try to "borrow" the neighboring vertex
  * if the following condition meets:
  *           1. there is a neighbor containing the two faces, and has a vertex with a bnd sign of -2 (on multiple bnd)
  *           2. the distance between the neighboring vertex to the two "local" vetex is smaller than the cellSize*1.3
  *           3. the neighboring cell has a clear bnd-sign after its own vertex is gone.
  *
  *
  * If the cell can not "borrow" neighboring vertex, then have to move one of the face to one neighboring cell
  *	method to find the neighbor to move is as following:
  *           1. find two vertex on each of the face
  *           2. decide with vertex to move based on the distance to cell boundaries
  *           3. find the neighbor, which is on the opposite of the face remaining in the current cell
  *           4. move the face to that neighbor
  *           
  *
  */



void Contour::processTwoDisconFaceCells(){
	
	__int64 f[2];
    __int64 bnd[2];
    __int64 curOcId;
    double  v[2][3];
 

    cout<<"number of twoDisconFaceCells is: "<<twoDisconFaceCells->length<<endl;

	for(__int64 i=0; i<twoDisconFaceCells->length; i++){

		cout<<"processing twoDisconFaceCell ["<<i<<"]"<<endl;

		curOcId = (*twoDisconFaceCells)[i];
//		cout<<"cellId="<<ot->getCellId(curOcId)<<" ocId="<<curOcId<<" lvl="<<ot->getLvlOfOcId(curOcId)<<endl;

        f[0] =( * ot->getFc(curOcId) )[0];
        f[1] =( * ot->getFc(curOcId) )[1];
		cout<<"faces are: "<<f[0] << ", "<<f[1]<<endl;



        //get the vertex for the two face
        findVtx41FaceHelper( curOcId, f[0], v[0], bnd[0] );
        findVtx41FaceHelper( curOcId, f[1], v[1], bnd[1] );
        intArr ff;
        ff.add(f[0]);
        ff.add(f[1]);

	//	if(borrowVtxHelper( curOcId, ff, v ) ){
	//		continue;
	//	}
		if(moveVtxHelper_two(curOcId, f, v) ){
			continue;
		}

		cout<<"In processTwoDisconFace, can not borrow or move for the cell: "<<endl;
		cout<<">>>>>>>>>>>>>>>> ocId="<<curOcId<<" fc:("<<f[0]<<", "<<f[1]<<") "<<endl;
		double cx[3];
		ot->getBaseCellNodeCoord(curOcId, cx);
		double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(curOcId));
		cout<<"cell base: ("<<cx[0]<<", "<<cx[1]<<", "<<cx[2]<<"), size="<<size<<endl;





	//	collapseOtCell(curOcId);

	}



}








bool Contour::moveVtxHelper_two( __int64 ocId, __int64 f[2], double v[2][3] ){


		double cx[8][3];


		ot->getFullCellNodeCoord( ocId, cx );



	    //calculate the distance
        //and find the vertex with min distance
	    //and put all the signs

	    double size = cx[1][0]-cx[0][0];
        double minDist[2] = {size, size};
        double sumDist[2] = {0, 0};

        for(__int64 j=0; j<2; j++){

            for(__int64 k=0; k<3; k++){
                double t = v[j][k] - cx[0][k];
                if(t<0 || t>size){
                	printf("Error in moveVtxHelper_two: vertex not in the cell, ocId=%lld.\n", ocId);
            		printf("        face are: [%lld], [%lld]\n", f[0], f[1]);
            		printf("Cell base is: (%f, %f, %f)\n", cx[0][0], cx[0][1], cx[0][2]);
            		printf("Cell level is: %lld\nCell size: %f\n", ot->getLvlOfOcId(ocId), size);
            		printf("vtx find: (%f, %f, %f)\n", v[j][0], v[j][1], v[j][2]);

                	throw StoppingException("\n");
                }
                else{
                	t = size-t;
                }
                sumDist[j] += t;
                if(t<minDist[j]){
                	minDist[j] = t;
                }
           }

        }
        __int64 movF;
        if(minDist[0]<minDist[1])
        	movF = 0;
        else if(minDist[0]==minDist[1])
        	if(sumDist[0]<sumDist[1])
        		movF = 0;
        	else
        		movF = 1;
        else
        	movF = 1;

        bool flg = false;


		// move the "moveF" to the beginning of the face array and vertex array;
		if(movF==1){
			__int64 movtt = f[0];
			f[0] = f[1];
			f[1] = movtt;
			double vtt;
			for(__int64 k=0; k<3; k++){
				vtt = v[0][k];
				v[0][k] = v[1][k];
				v[1][k] = vtt;
			}
		}
		__int64 neiVId;
		flg = findMoveNeighbor( ocId, f, v, size, neiVId );
		if(!flg){
			__int64 movtt = f[0];
			f[0] = f[1];
			f[1] = movtt;
			double vtt;
			for(__int64 k=0; k<3; k++){
				vtt = v[0][k];
				v[0][k] = v[1][k];
				v[1][k] = vtt;
			}
			flg = findMoveNeighbor( ocId, f, v, size, neiVId );
		}
		
        if(!flg){
        	printf("In moveVtxHelper, can not find empty neighbor cells for ocId=%lld.", ocId);
         	return false;
		}

        //update the neighbor's vertex
        vtx->updateVtx( neiVId, v[0] );
        intArr newBnd;
        newBnd.add(f[0]);
        vtx->setBnd(neiVId, &newBnd);

        //add the other vertex to this cell
        intArr tempmovf;
        tempmovf.add(f[1]);
    	addVtx(v[1],
    		   ocId,
    		   &tempmovf);
       
		printf("In moveVtxHelper_two: face are: [%lld], [%lld]\n", f[0], f[1]);
		printf("Cell base is: (%f, %f, %f)\n", cx[0][0], cx[0][1], cx[0][2]);
		printf("Cell level is: %lld\nCell size: %f\n", ot->getLvlOfOcId(ocId), size);
	//	printf("vtx find: (%f, %f, %f)\n", v[j][0], v[j][1], v[j][2]);


    	return true;

}

bool Contour::findMoveNeighbor( __int64 ocId, __int64 f[2], double v[2][3], double size, __int64 &neiVId ){

	double diff;
	__int64 sign[3];
	for(__int64 k=0; k<3; k++){
		diff = v[1][k]-v[0][k];
		if( fabs(diff)<size*0.2 )
			sign[k] = 0;
		else if( diff > 0 )
			sign[k] = -1;

		else
			sign[k] = 1;			
			
	}

	__int64 neiOcId = ot->getNeighOcId(ocId, sign);
	__int64 neiCellId = ot->getCellId(neiOcId);

	if( neiCellId == -1){ // not in the cell array, could be because
		                               // the cell is not in the meshing zone
		                               // have to try to get other neighbors
		

		return false;
	}
	else if( twoDisconFaceCells->has(neiOcId)){
		return false;
	}
	else if( multiDisconFaceCells->has(neiOcId)){
		return false;
	}
	else
		neiVId = ot->getVtx(neiOcId);

	return true;


}








bool Contour::borrowVtxHelper(__int64 ocId, intArr &f, double v[2][3]){
	
	
    double mid[3] = { (v[0][0]+v[1][0])/2,
    		          (v[0][1]+v[1][1])/2,
    		          (v[0][2]+v[1][2])/2};

    __int64 neiOcId[27];
    ot->getFullNeighOcId(ocId, neiOcId, ot->getLvlOfOcId(ocId));
    __int64 neiVOcId[27];

    __int64 vCnt=0;
    intArr* fc;
    for(__int64 j=0; j<27; j++){

    	neiVOcId[j] = -1;
    	if(j==13)
    		continue;
    	if(ot->getCellId(neiOcId[j])==-1)//not in the cell array
    		continue;
    	fc = ot->getFc(neiOcId[j]);
    	if(!fc)
    		continue;
    	if(fc){
    		__int64 cnt=0;
    		for(__int64 k=0; k<fc->length; k++){
				if( f.has( (*fc)[k] ) )
					cnt++;
			}
    		if(cnt < f.length)
    			continue;
			__int64 temp = ot->getVtx(neiOcId[j]);
    		if(temp!=-1)
				neiVOcId[vCnt++] = j;
    	}
    }

    if(vCnt==0){
    	return false;
	}
    __int64 min=-1;
    double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(ocId));
    double minD = size*size*4;
    for(__int64 j=0; j<vCnt;j++){
    	double* vv = vtx->getCoord(ot->getVtx(neiOcId[neiVOcId[j]]));
    	double temp = (mid[0]-vv[0])*(mid[0]-vv[0])
    			     +(mid[1]-vv[1])*(mid[1]-vv[1])
    			     +(mid[2]-vv[2])*(mid[2]-vv[2]);
    	if(temp<minD){
    		min = neiVOcId[j];
    		minD = temp;
    	}
    }

	if(minD>size*size*1.5)
		return false;
		

	intArr *neifc = ot->getFc(neiOcId[min]);
    ot->setVtx(ocId, ot->getVtx(neiOcId[min]));
    ot->setBnd(ocId, neifc);


/*
    // add the center point for the neighbor cell
    double midNei[3];
 	ot->getBaseCellNodeCoord( neiOcId[min], midNei );

 	midNei[0] += size/2;
 	midNei[1] += size/2;
 	midNei[2] += size/2;

 	__int64 sign[3] = {0, 0, 0};

    if(min<9)
    	sign[2] = -1;
    if(min>17)
    	sign[2] = 1;
    if(min%3==0)
    	sign[0] = -1;
    if(min%3==2)
    	sign[0] = 1;
    if((min/3)%3==0)
    	sign[1] = -1;
    if((min/3)%3==2)
    	sign[1] = 1;

    __int64 refOcId = ot->getNeighOcId(neiOcId[min], sign );
    if(ot->getCellId(refOcId)==-1)
    	refOcId = ot->getParentId(refOcId);

    __int64 refBnd = ot->getBnd(refOcId);

	if(refBnd==-2)
		return false;

	if(refBnd!=-1){

   		double newv[3];
		__int64 newBnd;

		GM->project2Face(midNei, refBnd, newv, newBnd);

    }

	addVtx(midNei,
           neiOcId[min],
           refBnd);

*/
	return true;



}


bool Contour::moveVtxHelper_multi( __int64 ocId, intArrSorted f[2], double v[2][3] ){


		double cx[8][3];


		ot->getFullCellNodeCoord( ocId, cx );

	    //calculate the distance
        //and find the vertex with min distance
	    //and put all the signs
        __int64 movF;
		if(f[0].length==1)
			movF = 0;
		else if(f[1].length==1)
			movF = 1;
		else{

			double size = cx[1][0]-cx[0][0];
			double minDist[2] = {size, size};
			double sumDist[2] = {0, 0};

			for(__int64 j=0; j<2; j++){

				for(__int64 k=0; k<3; k++){
					double t = v[j][k] - cx[0][k];
					if(t<0 || t>size){
						printf("Error in processMultiDisconFaceCells: vertex not in the cell, ocId=%lld.", ocId);
						throw StoppingException("\n");
					}
					else{
						t = size-t;
					}
					sumDist[j] += t;
					if( t<minDist[j] ){
						minDist[j] = t;
					}
			   }
			}



			if(minDist[0]<minDist[1])
				movF = 0;
			else if(minDist[0]==minDist[1])
				if(sumDist[0]<sumDist[1])
					movF = 0;
				else
					movF = 1;
			else
				movF = 1;

		}


		// move the "moveF" to the beginning of the face array and vertex array;

		if(movF==1){
			intArr movtt;

			for(__int64 i=0; i<f[0].length; i++)
				movtt.add(f[0][i]);
			f[0].clear();
			for(__int64 i=0; i<f[1].length; i++)
				f[0].insert(f[1][i]);
			f[1].clear();
			for(__int64 i=0; i<movtt.length; i++)
				f[1].insert(movtt[i]);

			double vtt;
			for(__int64 k=0; k<3; k++){
				vtt = v[0][k];
				v[0][k] = v[1][k];
				v[1][k] = vtt;
			}
		}
		__int64 neiVId, neiOcId;

		double normal[3];


		if(f[0].length==1){//only one face need to moved out
			GM->getPlanarFaceFunction(f[0][0], normal);

			double sign = v[1][0]*normal[0] + v[1][1]*normal[1] + v[1][2]*normal[2] + normal[3];
			if(sign>0){
				normal[0] = -normal[0]; normal[1] = -normal[1]; normal[2] = -normal[2];
			}
			neiOcId = findNeighOcIdWithDirection(ocId, normal);

			if(neiOcId == -1)
				return false;
			if( ot->getCellId(neiOcId) == -1 )
				return false;
			intArr* neiBnd = ot->getBnd(neiOcId);
			if( neiBnd->length > 1 )
				return false; // on multiple face, can not use this one
			intArr* neifc = ot->getFc(neiOcId);
			if( neifc && !f[0].has( (*neifc)[0] ) )
				return false;
			//the neighbor cell is either empty or has the same faces as the current cell
			//thus should use this cell
			neiVId = ot->getVtx(neiOcId);

			//update the neighbor's vertex
	        vtx->updateVtx( neiVId, v[0] );
	        intArr newBnd;

	        for(__int64 i=0; i<f[0].length; i++){
	        	newBnd.add(f[0][i]);
	        }
	        vtx->setBnd( neiVId, &newBnd);


	        //add the other vertex to this cell
	        intArr curbnd;
	        for(__int64 i=0; i<f[1].length; i++){
	        	curbnd.add(f[1][i]);
	        }

			addVtx(v[1], ocId, &curbnd);


		}

		else{ // more than one face need to be moved out
			  // will move one with all the
			__int64 fcpair[2];
			bool flg =false;
			for( __int64 i=0; i<f[0].length; i++ ){//find the face pair for the vertex

				fcpair[0] = f[0][i];

				for( __int64 j=0; j<f[1].length; j++){

					if( f[1][j] == fcpair[0] )
						continue; //have to make sure that the face from the two face
					              //array for each of the two vertex are different
					fcpair[1] = f[1][j];
					neiOcId = findNeighOcIdOppositeToFace(ocId, fcpair[0], v);

					if(neiOcId == -1)
						continue;
					if( ot->getCellId(neiOcId) == -1)
						continue;
					intArr* neiBnd = ot->getBnd(neiOcId);
					if( neiBnd->length >  1)
						continue; // on multiple face, can not use this one

					intArr* neifc = ot->getFc(neiOcId);
					if( neifc && !f[0].has( (*neifc)[0] ) )
						continue;
					//the neighbor cell is either empty or has the same faces as the current cell
					//thus should use this cell
					neiVId = ot->getVtx(neiOcId);
					flg= true;
					break;

				}
				if(flg)
					break;


			}



	        if(!flg){
	        	printf("In moveVtxHelper_multi, can not find empty neighbor cells for ocId=%lld.", ocId);
	         	return false;
			}

	        //update the neighbor's vertex
	        vtx->updateVtx( neiVId, v[0] );
	        intArr newBnd;

	        for(__int64 i=0; i<f[0].length; i++){
	        	newBnd.add(f[0][i]);
	        }
	        vtx->setBnd( neiVId, &newBnd);


	        //add the other vertex to this cell
	        intArr curbnd;
	        for(__int64 i=0; i<f[1].length; i++){
	        	curbnd.add(f[1][i]);
	        }

			addVtx(v[1], ocId, &curbnd);
		}
    	return true;

}

__int64 Contour::findNeighOcIdMidLine(__int64 ocId, __int64 f[2], double v[2][3]){


	double n1[4], n2[4];
	GM->getPlanarFaceFunction(f[0], n1);
	GM->getPlanarFaceFunction(f[1], n2);

	double midLNorm[3];

	midLNorm[0] = (n1[0] + n2[0]);
	midLNorm[1] = (n1[1] + n2[1]);
	midLNorm[2] = (n1[2] + n2[2]);

	__int64 sign[3];
	
	for(__int64 i=0; i<3; i++) {
	
		if( fabs(midLNorm[i]) < 0.0001 )
			sign[i] = 0;
		else if(midLNorm[i] < 0 )
			sign[i] = -1;
		else
			sign[i] = 1;

	}


	return ot->getNeighOcId(ocId, sign);

	

}


__int64 Contour::findNeighOcIdOppositeToFace(__int64 ocId, __int64 f, double v[2][3]){


	double n[4];
	GM->getPlanarFaceFunction(f, n);


	double dist2f = GM->getDistance2Face(f, v[1]);
	if(fabs(dist2f)<0.0000001)
		return -1;
	__int64 direcsign = dist2f > 0 ? -1 : 1;



	__int64 sign[3];
	
	for(__int64 i=0; i<3; i++) {
	
		if( fabs(n[i]) < 0.0001 )
			sign[i] = 0;
		else if(n[i] < 0 )
			sign[i] = -1;
		else
			sign[i] = 1;

		sign[i] *= direcsign;

	}

	return ot->getNeighOcId(ocId, sign);

	

}

__int64 Contour::findNeighOcIdWithDirection(__int64 ocId, double n[3]){

	__int64 sign[3];

	for(__int64 i=0; i<3; i++) {

		if( fabs(n[i]) < 0.001 )
			sign[i] = 0;
		else if(n[i] < 0 )
			sign[i] = -1;
		else
			sign[i] = 1;
	}

	return ot->getNeighOcId(ocId, sign);


}

/*

void Contour::processMultiVticesCells(){

	if(twoVtxCells->length==0)
		return;
	ot->incrementBotLvl();

	__int64 numCells = twoVtxCells->length /3; // assumption hear is there are only two vertices in these cells
	__int64 curOcId, curLvl;
	__int64 vId[2];
	__int64 v4Cell[8];
	double *vCoord[2];


	for(__int64 i=0; i<numCells; i++){  // iterate through each of the multi-vtx cells


		curOcId = (*twoVtxCells)[i*3];
		vId[0]  = (*twoVtxCells)[i*3+1];
		vId[1]  = (*twoVtxCells)[i*3+2];

		vCoord[0] = vtx->getCoord(vId[0]);
		vCoord[1] = vtx->getCoord(vId[1]);

		curLvl = ot->getLvlOfOcId(curOcId);

		intArr *vFace[2];
		vFace[0] = vtx->getBnd(vId[0]);
		vFace[1] = vtx->getBnd(vId[1]);


		cout<<"in process multi vtices cells: i="<<i<<" ocId="<<curOcId<<" fc= ";
		intArr* fc = ot->getFc(curOcId);
		for(__int64 i=0; i<fc->length; i++){
			cout<<(*fc)[i]<<", ";
		}

		__int64 ll(240547);
		ll *= 10000000;
		ll +=  2498487;

//		if(curOcId == ll){
	//		cout<<"faces are:"<<endl;
	//		intArr* fc = ot->getFc(curOcId);
	//		fc->print();
			double base[3];
			ot->getBaseCellNodeCoord(curOcId, base);
			cout<<"base=("<<base[0]<<", "<<base[1]<<", "<<base[2]<<")";
			cout<<"\tvFace[0]=";
			for(__int64 i=0; i<vFace[0]->length; i++){
				cout<<(*vFace[0])[i]<<", ";
			}
			cout<<"\tvFace[1]=";
			for(__int64 i=0; i<vFace[1]->length; i++){
				cout<<(*vFace[1])[i]<<", ";
			}

	//	}


		//look at the neiborghing element
		//for each element get all the neighbor vtx bnd
		//count the number of common bnds between the vtx and neighboring element
		//select the higher count between the two vtx
		//if there is no common bnd then keep it -1
		//after all the assignable cell have been assigned,
		//one more iteration will go over all the subcells
		//trying to figure out it is vtx, by looking at the neighboring cells


		double diff[3];
		diff[0] = vCoord[1][0] - vCoord[0][0];
		diff[1] = vCoord[1][1] - vCoord[0][1];
		diff[2] = vCoord[1][2] - vCoord[0][2];

		int max = 0;
		if(fabs(diff[1])> fabs(diff[0]))
			max = 1;
		if(fabs(diff[2]) > fabs(diff[max]))
			max = 2;

		switch (max){
		case 0: // split along x (left-right)

			if(diff[0]>0){//1 on the right
				v4Cell[1] = v4Cell[3] = v4Cell[5] = v4Cell[7] = 1;
				v4Cell[0] = v4Cell[2] = v4Cell[4] = v4Cell[6] = 0;
			}
			else{ // 1 on the left
				v4Cell[1] = v4Cell[3] = v4Cell[5] = v4Cell[7] = 0;
				v4Cell[0] = v4Cell[2] = v4Cell[4] = v4Cell[6] = 1;
			}

			break;

		case 1: //split along y (front-back)

			if(diff[0]>0){//1 on the back
				v4Cell[2] = v4Cell[3] = v4Cell[6] = v4Cell[7] = 1;
				v4Cell[0] = v4Cell[1] = v4Cell[4] = v4Cell[5] = 0;
			}
			else{ // 1 on the front
				v4Cell[2] = v4Cell[3] = v4Cell[6] = v4Cell[7] = 0;
				v4Cell[0] = v4Cell[1] = v4Cell[4] = v4Cell[5] = 1;
			}

			break;

		case 2: // split along z (top-bottom)

			if(diff[0]>0){//1 on the top
				v4Cell[4] = v4Cell[5] = v4Cell[6] = v4Cell[7] = 1;
				v4Cell[0] = v4Cell[1] = v4Cell[2] = v4Cell[3] = 0;
			}
			else{ // 1 on the front
				v4Cell[4] = v4Cell[5] = v4Cell[6] = v4Cell[7] = 0;
				v4Cell[0] = v4Cell[1] = v4Cell[2] = v4Cell[3] = 1;
			}

			break;
		}




		__int64 ii, jj, kk;
		ot->ocId2ijk(curOcId, ii, jj, kk);
		for(int j=0; j<8; j++){ // iterator for each of the subcells
								// iterate through each of the neighboring elements to
			                    // figure out the vtx assignment for each of the subcells

			__int64 thisOcId; // the ocId used to find the cell using getNeighLeafCellVtx

			thisOcId = ot->ijk2OcId( ii+j%2, jj+(j/2)%2, kk+(j/4), curLvl);
			__int64 elem[8]; // hold the vtx id of the 8 vtices of the cell

			ot->getNeighOcId4Elem(thisOcId, elem, curLvl);

			checkBndSignWithElem(j, v4Cell, elem, vId, vFace);

		}

		//collapse the current cell, and return the 8 sub cells
		__int64 cldOcId[8];
		ot->collapseOnlyThisCellForMultiVtxCellProc(curOcId, cldOcId);

		cout<<"\tv4Cell: {";

		for(int j=0; j<8; j++){
			cout<<v4Cell[j]<<", ";
			ot->setVtx(cldOcId[j], vId[v4Cell[j]]);
			ot->setBnd(cldOcId[j], vtx->getBnd(vId[v4Cell[j]]));
		}

		cout<<"}"<<endl;

	}

}


*/


/**
 *
 * The new multivticesCell processor
 *
 * This function will try to move the grid surfaces in such a way that each of the newly collapsed subcells will
 * have one vtx.
 *
 *
 *
 *
 */

void Contour::processMultiVticesCells(){

	if(multiVtxCells->length==0)
		return;

//	char* fullFilePath = NULL;
//    getFullFilePath(&fullFilePath, fName, ".mesh");
//    meshFile = fopen(fullFilePath, "a");

	if(ot->uniform){

		__int64 curOcId, curLvl;

		for(__int64 mvc=0; mvc<multiVtxCells->length; mvc++){  // iterate through each of the multi-vtx cells

			curOcId = (*multiVtxCells)[mvc];
			curLvl = ot->getLvlOfOcId(curOcId);

			intArr *fc = ot->getFc(curOcId);

			geoNodeArr mv;
			pointerArr vf;
			findVtxForMultiVticesCell(curOcId, mv, vf);

			for(int i=0; i<vf.length; i++){

				delete(vf[i]);

			}

			double fv[3] ={0,0,0};
			double *x;
			for(int vs=0; vs<mv.length; vs++){

				x = mv.getNode(vs);
				fv[0] += x[0];
				fv[1] += x[1];
				fv[2] += x[2];

			}
			fv[0] /= mv.length;
			fv[1] /= mv.length;
			fv[2] /= mv.length;

			addVtx(fv, curOcId, fc);


		}

	//	fclose(meshFile);
		return;

	}


		for(__int64 mvc=0; mvc<multiVtxCells->length; mvc++){
			double x[3], size, v[3];
			int curOcId = (*multiVtxCells)[mvc];

			intArr *fc = ot->getFc(curOcId);
			fc->print();
			cout<<"****";

			ot->getBaseCellNodeCoord(curOcId, x);
			int curLvl =  ot->getLvlOfOcId(curOcId);
			size = ot->getSizeOfCellAtLvl(curLvl)/2;
			v[0] = x[0] + size;
			v[1] = x[1] + size;
			v[2] = x[2] + size;

			addVtx(v,	 			//coordinate
					curOcId, 	    //ocId
					mvc);            //cellId
		}

	/*



	ot->incrementBotLvl();



	__int64 curOcId, curLvl;

	for(__int64 mvc=0; mvc<multiVtxCells->length; mvc++){  // iterate through each of the multi-vtx cells
		
		curOcId = (*multiVtxCells)[mvc];
		curLvl = ot->getLvlOfOcId(curOcId);

		if(mvc%1000==0) 
			cout<<"processing mvc="<<mvc<<" of "<<multiVtxCells->length<<endl;
/*		cout<<"processing "<<mvc<<endl;
		printf("ocId=%lld.\n", curOcId);
		double base[3];
		ot->getBaseCellNodeCoord(curOcId, base);
		cout<<"base=("<<base[0]<<", "<<base[1]<<", "<<base[2]<<")";
		cout<<"\nface="; */
/***		intArr *fc = ot->getFc(curOcId);
//		fc->print();
//		cout<<endl;

/*		if( fc->length>2){//more than two faces in this cell, then the method follows will not work
			              // will put the average of the vtx and assign all the faces as bnd signs
			geoNodeArr mv;
			pointerArr vf;
			findVtxForMultiVticesCell(curOcId, mv, vf);

			double fv[3] ={0,0,0};
			double *x;
			for(int vs=0; vs<mv.length; vs++){

				x = mv.getNode(vs);
				fv[0] += x[0];
				fv[1] += x[1];
				fv[2] += x[2];

			}
			fv[0] /= mv.length;
			fv[1] /= mv.length;
			fv[2] /= mv.length;

			addVtx(fv, curOcId, fc);

			for(int i=0; i<vf.length; i++){

				delete(vf[i]);

			}

			continue;

		}

		//collapse the current cell, and return the 8 sub cells
		__int64 cldOcId[8];
		ot->collapseOnlyThisCellForMultiVtxCellProc(curOcId, cldOcId); // this function will also set the interface and internode

		if(curOcId == 641463173)
			printf("KKKKKKKKKK\n");

		geoNodeArr v[8];
		pointerArr vFace[8]; // array to hold the bnd sign of the vtx
		                     // if multiple vtx found for the subcell

		for(__int64 j=0; j<8; j++){//iterator of each of the subcell

			findVtxForMultiVticesCell(cldOcId[j], v[j], vFace[j]);
			if(cldOcId[j] == 641463173)
				printf("KKKKKKKKKK\n");

		/*	for(int k=0; k<v[j].length; k++){
				double *tt = v[j].getNode(k);
				intArrSorted * tf = (intArrSorted*) vFace[j][k];
				cout<<"vtx found for subcell ["<<j<<"]: ("<<tt[0]<<", "<<tt[1]<<", "<<tt[2]<<") fc= ";
				tf->print();
				cout<<endl<<endl; 
			}
		
		}*/

		

/***		__int64 pii[3];
		ot->ocId2ijk(curOcId, pii[0], pii[1], pii[2]);
		intArr choice[8]; // store the appropriate bnd sign and inappropriate bnd sign
		 	 	 	 	  // @choice the array to hold the appropriate vtx and inappropriate bnd sign
		                  //         assumption are: appropriate vtx will be put at the beginning, if no one found, then put -1
		                  //                         inappropriate bnd sign may be multiple will be put after the appropriate ones
		                  //                         with minus sign the formular is (-bnd-1)

		for(__int64 j=0; j<8; j++){//iterator for each of the subcell

			if( v[j].length<2 )
				continue;

			checkBndSignWithElem(j, curOcId, v[j], vFace[j], choice[j]);

			//the situation is that there are limited neighbor cell can be used to put the other vtx
			//the vtx can not be moved to the opposite direction
			//some time, some axis-aligned vtx may be moved, then have to make sure the other vtx be moved at the same time.

			struct movInfo{
				int movDir[3];
				intArr movTogether;
				bool movable;
			};

			movInfo myMovInfo[8];
			int numCannotMove = 0;

            for(int k=0; k<v[j].length; k++){//iterate through all the vtx in this subcell
            	int dir[3];
            	intArr movTogether;
            	bool movable;
            	getSubCellVtxMovInfo(j, k, curOcId, v, vFace, dir, movTogether, movable, cldOcId);

            	myMovInfo[k].movable = movable;
            	myMovInfo[k].movDir[0] = dir[0];
            	myMovInfo[k].movDir[1] = dir[1];
            	myMovInfo[k].movDir[2] = dir[2];
            	myMovInfo[k].movTogether.copy(movTogether);

            	if( !myMovInfo[k].movable)
            		numCannotMove ++;
            }

            //got all the move information, now have to decide which vtx to move and which cell to mov.
            if( numCannotMove > 1 ){//
                printf("Error: in processMultivticesCells, subcell vtx more than one can not move, subCell=%lld.\n", j);
                throw StoppingException("\n");
            }

        	int ii[3]; // the position of this subcell
        	ii[0] =  j%2;
        	ii[1] = (j/2)%2;
        	ii[2] = (j/4)%2;

        	int cnt=0;
            while ( v[j].length>1 && cnt<16){ //iterate through all the vtx in this subcell
            	                          // need to move the vtx to a neigh
            	                          // clean up the current cell


            	cnt++;
            	int mov=-1;
            	for(int vv=0; vv<v[j].length; vv++){
            		if ( !myMovInfo[vv].movable )
            			continue;
                    if (myMovInfo[vv].movTogether.length==0){
                        mov = vv;
                        break;
                    }
                }
            	if(mov==-1){
                	for(int vv=0; vv<v[j].length; vv++){
                		if( myMovInfo[vv].movable ){
                			mov = vv;
                			break;
                		}
                	}
            	}

                bool flg = false;
                for(int cc=0; cc<3; cc++){
                	if(myMovInfo[mov].movDir[cc] == 0)
                		continue;
                	int movSubCell;
                	switch (cc) {
					case 0:
    					movSubCell =    ii[0]+myMovInfo[mov].movDir[0]
    								 + (ii[1])*2
    								 + (ii[2])*4;
					case 1:
						movSubCell =    ii[0]
									 + (ii[1]+myMovInfo[mov].movDir[1])*2
									 + (ii[2])*4;
					case 2:
						movSubCell =    ii[0]+myMovInfo[mov].movDir[0]
									 + (ii[1]+myMovInfo[mov].movDir[1])*2
									 + (ii[2]+myMovInfo[mov].movDir[2])*4;

                	}

                	if(vFace[movSubCell].length!=0){ // not empty
                		//if one vtx in this cell, then if the vtx share the same bnd signs, then this vtx can be deleted
                		intArrSorted* thisBnd = (intArrSorted*)vFace[j][mov];
                		if(v[movSubCell].length ==1){
                			int cnt = 0;
                			intArrSorted* thisBB = (intArrSorted*)vFace[movSubCell][0];
                			for(int bb=0; bb<thisBnd->length; bb++){
                				if(thisBB->has((*thisBnd)[bb]))
                					cnt++;
                			}
                			if(cnt== thisBnd->length){
                				v[j].deleteNode(mov);
                				vFace[j].shallowDelete(mov);

                				//worry about the movetogether vtx
                				//TODO


                				flg = true;
                				break;
                			}

                		}
                		//if multiple vtices, then it is more complicated, will have to check the remining vtx
                		else{

                			for(int mm=0; mm<v[movSubCell].length; mm++){
                       			int cnt = 0;
                       			intArrSorted* thisBB = (intArrSorted*)vFace[movSubCell][mm];
                       			for(int bb=0; bb<thisBnd->length; bb++){
                       				if(thisBB->has((*thisBnd)[bb]))
                       					cnt++;
                       			}
                       			if(cnt== thisBnd->length){
                       				v[j].deleteNode(mov);
                       				vFace[j].shallowDelete(mov);

                       				//worry about the movetogether vtx
                       				//TODO


                       				flg = true;
                       				break;
                       			}
                			}
                			if(flg)
                				break;
                		}

                	}
                	else{//moveNeighbor is empty


						double * vv = v[j].getNode(mov);
						v[movSubCell].add(vv);
						v[j].deleteNode(mov);
						vFace[movSubCell].add(vFace[j][mov]);
						vFace[j].shallowDelete(mov); //only delete the pointer in the array, not the object.
						break;
                	}
                }//end of the iterator for the three direction

            }
            if( v[j].length > 1 ){
             	printf("Error in processMultiVticesCells, can not move the current subcell.\n");
            }

   //         for(int k=0; k<vFace[j].length; k++){
   //         	delete(vFace[j][k]);
	//			vFace[j][k]=NULL;
	//		}
	

		}



		checkUpdateSnappedElement(cldOcId, v, vFace);

		for(__int64 sId=0; sId<8; sId++){//iterator for each of the subcell to update vtx

			if(v[sId].length==0){
				double subV[3];
				double base[3];
				ot->getBaseCellNodeCoord(cldOcId[sId], base);
				double size = ot->getSizeOfCellAtLvl(ot->botLvl);
				subV[0] = base[0]+size/2;
				subV[1] = base[1]+size/2;
				subV[2] = base[2]+size/2;


				__int64 z, b;
				GM->getZoneOrBndId(subV, z, b);

				ot->setZone( cldOcId[sId], z );
				if(z==-1) 
					continue;

				intArr* temp = NULL;
				addVtx(subV, cldOcId[sId], temp);
/*****
				vtx->add(subV);
//				vtx->setOcId(vtx->length-1, cldOcId[sId]);

				vtx->setZone(vtx->length-1, ot->getZone(cldOcId[sId]));

				ot->setVtx(cldOcId[sId], vtx->length-1);
				intArr bnd;
				bnd.add(-1);
				ot->setBnd(cldOcId[sId], &bnd); // deep copy
				vtx->setBnd(vtx->length-1, &bnd); //deep copy */
/********			}
			else if(v[sId].length ==1){

				double * x = v[sId].getNode(0);
				intArrSorted* bnds = (intArrSorted*)vFace[sId][0];
				intArr bb;
				for(int b=0; b<bnds->length; b++){
					
					bb.add(GM->getBndOfFace(  (*bnds)[b] ));
				//	bb.add( (*bnds)[b] );
				}

				__int64 z, b;
				GM->getZoneOrBndId(x, z, b);

				ot->setZone( cldOcId[sId], z );
				if(z==-1) 
					continue;
				addVtx(x, cldOcId[sId], &bb);
/*				vtx->add(x);
//				vtx->setOcId(vtx->length-1, cldOcId[sId]);
				vtx->setZone(vtx->length-1, ot->getZone(cldOcId[sId]));
				vtx->setBnd(vtx->length-1, &bb);
				ot->setVtx(cldOcId[sId], vtx->length-1);
				ot->setBnd(cldOcId[sId], &bb);*/

/****			}
			else{
				printf("Error: still got more than one vtx in the sub cell, prtOcId=%lld, subId=%lld", curOcId, sId);
			//	throw StoppingException("\n");
			}
			
		}//iterator for each of the subcell to update vtx
		

	}*/

	//fclose(meshFile);
}


/**
 * This function will check and update the snapped elements
 *
 */
void Contour::checkUpdateSnappedElement(__int64 cldOcId[8], geoNodeArr *v, pointerArr *vFace){

	if(!cldOcId){
		printf("Error: in checkAndUpdateSnappedElement, cldOcId array is NULL\n");
		throw StoppingException("\n");
	}

	__int64 pyrFormingOcId[6][3]={
			{1,1,0},
			{1,0,1},
			{0,1,1},
			{1,0,0},
			{0,1,0},
			{0,0,1}
	};


	__int64 curOcId;
	for(int i=0; i<6; i++){ // iterate for the six surfaces of the hex

		if(i<3)
			curOcId = ot->getNeighOcId(cldOcId[0], pyrFormingOcId[i]);
		else
			curOcId = ot->getNeighOcId(cldOcId[7], pyrFormingOcId[i]);
		
//		ot->getNeighLeafCellVtx(curOcId, cell); // can not use the generic get vtx function, because the sub cell vts has not been set
		__int64 lvl = ot->getLvlOfOcId(curOcId);
		__int64 neiOcId[8];
		

		ot->getNeighOcId4Elem(curOcId, neiOcId, lvl);


		__int64 prtCnt=0;
		//check if it is a pyramid, by looking at how many parent node are there, if not equal to 4 then not a pyramid
		for(__int64 j=0; j<8; j++){
		
			if(neiOcId[j] == -1){
				fprintf(stderr, "Error: in getNeighLeafCellVtx, ocId=%lld, %lldth neighbor is -1\n", curOcId, i);
				throw StoppingException("\n");
			}
			__int64 curColl = ot->getColl(neiOcId[j]);
			if( curColl == -1 ){
				prtCnt++;
				neiOcId[j] = ot->getParentId(neiOcId[j]);
				curColl = ot->getColl(neiOcId[j]);
				if (curColl != 0){
					fprintf(stderr, "Error: in getNeighLeafCellVtx, ocId=%lld, neighbor %lldth neighbor is not a leaf nor does its parent: [%lld]=%lld\n",
													 curOcId, j, neiOcId[j], curColl);
					throw StoppingException("\n");
			//		continue;
				}
			}
			else if( curColl == 1 ) {//because the max different level is 1 then this value can only be -1, 0 or 1

				neiOcId[j] = ot->getChildLeafId(neiOcId[j], j, lvl);
				curColl = ot->getColl(neiOcId[j]);
				if (curColl != 0){
					fprintf(stderr, "Error: in getNeighLeafCellVtx, ocId=%lld, neighbor %lldth neighbor has child but the child is not leaf: [%lld]=%lld\n",
													 curOcId, j, neiOcId[j], curColl);
					throw StoppingException("\n");
			//		continue;
				}
			}
			else if( curColl < -1 || curColl > 1){
				fprintf(stderr, "Error: in getNeighLeafCellVtx, ocId=%lld, neighbor %lldth neighbor out of allowable max diff: [%lld]=%lld\n",
												 curOcId, j, neiOcId[j], curColl);
				throw StoppingException("\n");
			//	continue;
			}
		}

		if(prtCnt!=4){ // not a pyramid
			continue;  // look at next surface's hex
		}

		__int64 dup[8][2]= {{-2,0},{-2,0},{-2,0},{-2,0},{-2,0},{-2,0},{-2,0},{-2,0}}, cnt=0;
		getDup(neiOcId, 8, -2, dup, cnt);
		if(cnt!=5){// not a pyramid
			continue;
		}
		__int64 tipId=-1;
		for(int j=0; j<5; j++){
			if(dup[j][1]==4){
				tipId = j;
				break;
			}
		}
		if(tipId==-1){ // not a pyramid
			continue;
		}
		if(tipId != 4){ // move the tip to the end of the array
			__int64 temp = dup[4][0];
		    dup[4][0] = dup[tipId][0];
		    dup[tipId][0] = temp;
		    dup[tipId][1] = 1;
		    dup[4][1] = 4;
		}

		__int64 subId[4];// array to hold the corresponging subCellId of the four distinct ocId of the pyramid
		bool pymFlg = true; // flg for checking if it is a snapped pyramid 
		for(int j=0; j<4; j++){
			subId[j]=-1;
			for(int k=0; k<8; k++){
				if(cldOcId[k] == dup[j][0]) {
					subId[j] = k;
					break;
				}
			}
			if(subId[j]==-1){
				pymFlg = false;
				break;
			}
		}
		if(!pymFlg)//not a snapped pyramid
			continue;
		

		//got all the subId of this pyramid in the cldOcId array

		intArr *bnd[5];
		intArr bDup;
		intArr bDupCnt;

		bnd[4] = new intArr();
		bnd[4]->copy(ot->getFc(dup[4][0]));
		if( (*bnd[4])[0] ==-1){//tip is not on the boundary, then do nothing
			delete(bnd[0]);
			continue;
		}
		pymFlg = true;
		for(__int64 j=0; j<4; j++){
			bnd[j] = new intArr();
			if(vFace[subId[j]].length>0){
				intArrSorted *bb = (intArrSorted*)vFace[subId[j]][0];
				if(!bb){ // not on the boundary then must not be a snapped pyramid ???????????????????????
					continue; 
				}
				for(__int64 k=0; k<bb->length; k++){
					bnd[j]->add( (*bb)[k]);
				}
			}
			else{
				pymFlg = false;
				break;
			}
			
		}
		if(!pymFlg)
			continue;

		getDup(bnd,  5, -3, &bDup, &bDupCnt);

		delete (bnd[0]); delete(bnd[1]); delete(bnd[2]); delete(bnd[3]); delete(bnd[4]);

		bool flg = false;
		for(int i=0; i<bDup.length; i++){
			if(flg==false && bDupCnt[i]==5){
				flg=true;
			}
			else if (flg== true && bDupCnt[i]==5){
				return; // it is the complicated part, will do nothing and return
			}
		}
		if(!flg)
			continue; // not a snapped element
		

		//calculate the distance to the tipvtx
		double dist[4];
		double tipV[3];
		if(!getVtxHelper(dup[4][0], tipV))
			continue;
		for(int k=0; k<4; k++){

			double *cv = v[subId[k]].getNode(0);
			dist[k] = pow(tipV[0]-cv[0], 2) + pow(tipV[1]-cv[1], 2) + pow(tipV[2]-cv[2], 2);
		}

		int sort[4]={0, 1, 2, 3};// the array (small to large) to hold the index sequence of the sorted distance
		for(int k=0; k<4; k++ ){
			int min = k;
			for(int m=k+1; m<4; m++){
				if( dist[m]<dist[min] )
					min = m;
			}
			if(min!=k){
				double temp = dist[k];
				dist[k] = dist[min];
				dist[min] = temp;
				int tm = sort[k];
				sort[k] = sort[min];
				sort[min] = tm;
			}
		}


		 for(int k=0; k<2; k++){

			 __int64 updateSubId = subId[ sort[k] ];
		
			 v[updateSubId].deleteNode(0);

			 intArrSorted* oldBnd = (intArrSorted*) vFace[updateSubId][0];
			 delete(oldBnd);
			 vFace[updateSubId].shallowDelete(0);



		 }

	}


}


void Contour::getSubCellVtxMovInfo(int subId, int vId, __int64 prtOcId, geoNodeArr v[8], pointerArr vFace[8],
		                              int movDir[3], intArr &movTogether, bool &movable, __int64 subOcId[8]){



	double * curV = v[subId].getNode(vId);
	intArrSorted * curB = (intArrSorted*) vFace[subId][vId];
	movDir[0] = movDir[1] = movDir[2] = 2; // both direction

	int ii[3];
	ii[0] = subId%2;
	ii[1] = (subId/2)%2;
	ii[2] = (subId/4)%2;

	double conVec[3] = {2,2,2}; // to store the moving direction vector for vtices that are connected with the cuurent vtx
	geoNodeArr disConVec; // to store the moving direction vector for vtices that are not connected with the current vtx


	//find the relative position to others connected vtices

	for(int j=0; j<v[subId].length; j++){ // iterator of all the vtx in this subCell
		if(j==vId)
			continue;
		double * curVV = v[subId].getNode(j);
		intArrSorted * curBB = (intArrSorted*) vFace[subId][j];
		bool flg = false;
		for( int k=0; k<curB->length; k++){
			if( curBB->has( (*curB)[k]) ){
				flg= true;
				break;
			}
		}
		if(flg){ // share same bnd sign

				 // should move along the vector from VV to V
			double vec[3] = {curV[0]-curVV[0], curV[1]-curVV[1], curV[2]-curVV[2]};
			conVec[0] = conVec[0] + vec[0];
			conVec[1] = conVec[1] + vec[1];
			conVec[2] = conVec[2] + vec[2];


		}
		else{// does not share same bnd sign
			 // then should move along the normal vector of its own face
			 // but have to make sure that the other vtx will always to at the same side the moving face
			 //
			double equa[4];
			for( int k=0; k<curB->length; k++){
				GM->getPlanarFaceFunction( (*curB)[k], equa );
				double dist = GM->getDistance2Face((*curB)[k], curVV);
				if(dist>0){
					equa[0] = -equa[0];
					equa[1] = -equa[1];
					equa[2] = -equa[2];
				}
				disConVec.add(equa);
			}
		}
	}

	for(int cc=0; cc<3; cc++){

		if( conVec[cc]==2){
			if(movDir[cc]==2)
				movDir[cc] =2;
		}
		else if(conVec[cc]<-0.001){//negative
			if(movDir[cc]==2){
				movDir[cc] = -1;
			}
			else if(movDir[cc]==1){ //positive, should turn 0
				movDir[cc] = 0;
			};
			//otherwise, 0 or negative, do not need to do anything
		}
		else if( conVec[cc] > 0.001 && conVec[cc]<1.001 ){//positive
			if(movDir[cc]==2){ //both, turn to positive
				movDir[cc] = 1;
			}
			else if(movDir[cc]== -1){ // negative, should turn 0
				movDir[cc] = 0;
			};
			//otherwise, 0 or positive, do not need to do anything

		}
		else{
			movDir[cc] = 0;
		}

		//iterate through all the discon faces, should make changes accordingly
		for(int discon=0; discon<disConVec.length; discon++){
			double * vec = disConVec.getNode(discon);
			if(vec[cc]<-0.001){
				if(movDir[cc]==2){
					movDir[cc] = -1;
				}
				else if(movDir[cc]==1){
					movDir[cc] = 0;
				}
			}
			else if( vec[cc]> 0.001){
				if(movDir[cc]==2){
					movDir[cc] = 1;
				}
				else if(movDir[cc]==-1){
					movDir[cc] = 0;
				}

			}
			else{
				movDir[cc] = 0;
			}
		}

	}

	for(int cc=0; cc<3; cc++) {// iterator for the three coordinates direction, to determine the move direction

		if ( ii[cc]==1 && movDir[cc] == 1) // on top, and move upwards, can not move then should set as 0
			movDir[cc] = 0;
		else if( ii[cc]==0 && movDir[cc] == -1) // on bottom, and move downwards, can not move, then should set 0
			movDir[cc] = 0;
		else if(ii[cc]==1 && movDir[cc] == 2)// on top, and move both direction, then move downwards
			movDir[cc] = -1;
		else if(ii[cc]==0 && movDir[cc] == 2)// on top, and move both direction, then move upwards
			movDir[cc] = 1;
	}



	if( movDir[0] == 0 && movDir[1] == 0 && movDir[2] == 0){
		movable = false;
		return;
	}
	else{
		movable = true;
	}

	//find if there is movetogether vtx

	for(int cc=0; cc<3; cc++){
		if(movDir[cc]==0)
			continue;
		__int64 subNei[3];
		int off[3];

		off[cc] = 0;
		off[(cc+1)%3] = 1;
		off[(cc+2)%3] = 0;
		subNei[0] = (ii[0]+off[0])%2 + (ii[1]+off[1])%2*2 + (ii[2]+off[2])%2*4;

		off[(cc+1)%3] = 1;
		off[(cc+2)%3] = 1;
		subNei[1] = (ii[0]+off[0])%2 + (ii[1]+off[1])%2*2 + (ii[2]+off[2])%2*4;

		off[(cc+1)%3] = 0;
		off[(cc+2)%3] = 1;
		subNei[2] = (ii[0]+off[0])%2 + (ii[1]+off[1])%2*2 + (ii[2]+off[2])%2*4;


		intArrSorted* curBnd = (intArrSorted*) vFace[subId][vId];
		for(int nn=0; nn<3; nn++){ // the three direct neighbor

			int subNeiId = subNei[nn];

			if(v[subNeiId].length==0)
				continue;

			bool flg = true;

			for(int j=0; j<v[subNeiId].length; j++){
				intArrSorted* neiBnd = (intArrSorted*)vFace[subNeiId][j];
				if(!neiBnd)
					break;
				for(int k=0; k<curBnd->length; k++){
					if( ! neiBnd->has( (*curBnd)[k] ) ){
						flg = false;
						break;
					}
				}
				if(flg){// same bnd sign vtx found
					movTogether.add(subNeiId);
					movTogether.add(j);
				}
				else
					flg = true;
			}
		}

	}



}




void Contour::sortMultiVticesCellWithFc(){

	intArr numF;
	__int64 curOcId, n;


	for(__int64 i=0; i<multiVtxCells->length; i++){  // iterate through each of the multi-vtx cells
		int max = 0;
		int id= -1;
		for(__int64 j=i; j<multiVtxCells->length; j++) {

			curOcId = (*multiVtxCells)[j];
		    n       = ot->getFc(curOcId)->length;

			if(n>max){
				max = n;
				id = j;
			}
		}

		if(max==2) // the smallest number is 2 if current biggest is 2 then all the rest are all eaqual to 2, no more iteration is needed.
			break;
		if(i==id) // current one is the largest not need to swap
			continue;
		
		//swap
		curOcId = (*multiVtxCells)[i];
		(*multiVtxCells)[i]  = (*multiVtxCells)[id];
		(*multiVtxCells)[id] = curOcId;

	}

}


/**
 *
 * This function will calculate the vtx on the face (or faces) in the cell
 *
 * If there is no face in the cell, then no action will be taken
 *
 * array v and vFace will remain empty
 */


void Contour::findVtxForMultiVticesCell(__int64 ocId, geoNodeArr& v, pointerArr & vFace){


	intArr* fc =  ot->getFc(ocId);

	intArrSorted pp, np; // array to hold the faces that have intersection lines and the faces do not have any intersection line with others

	if(!fc)
		return;

	for(__int64 j=0; j<fc->length; j++){

		for(__int64 k=j+1; k<fc->length; k++ ){

			geoNodeArr vv(5);//temp array to store all the vertices found
			
			if( findVtx42FaceHelper(ocId, (*fc)[j], (*fc)[k], vv) ){ //there are intersections
				                                                //put j, and k into processed array
				                                                //put the vtx to vtxarray
				pp.insert( (*fc)[j] );
				pp.insert( (*fc)[k] );
                
				for(__int64 ll=0; ll<vv.length; ll++){
				
					__int64 vIdTemp = v.addNonDup(vv.getNode(ll));
                    
                    if( vFace.length<vIdTemp+1 ){
                        intArrSorted * ff = new intArrSorted(2);
                        vFace.add(ff);
                    }
                    intArrSorted *ff = (intArrSorted*)vFace[vIdTemp];
                    ff->insert((*fc)[j]);
                    ff->insert((*fc)[k]);
				}
			}
		}
	}

	if( pp.length != fc->length ){//there are faces not processed
		for(__int64 j=0; j<fc->length; j++){
			if(pp.has((*fc)[j]))
				continue;
			np.insert((*fc)[j]);
		}

		for(__int64 j=0; j<np.length; j++){

			double vv[3];
			__int64 bnd;
		    findVtx41FaceHelper(ocId, np[j], vv, bnd);
		    __int64 vIdTemp = v.addNonDup(vv);

            if( vFace.length<vIdTemp+1 ){
                intArrSorted * ff = new intArrSorted(2);
                vFace.add(ff);
            }
            intArrSorted *ff = (intArrSorted*)vFace[vIdTemp];
            ff->insert((*fc)[j]);
		}
	}


}

void Contour::getRemainVtx(__int64 ocId, __int64 cldPos, geoNodeArr & v, __int64 &remainVId){



	int ii[3];

	ii[0] = cldPos%2;
	ii[1] = (cldPos/2)%2;
	ii[2] = (cldPos/4)%2;


	double base[3];

	ot->getBaseCellNodeCoord(ocId, base);

	double size = ot->getSizeOfCellAtLvl(ot->getLvlOfOcId(ocId));

	double ref[3]; // the reference point to cal the distance to
	               // the vtx with minDist should remain in this cell
	ref[0] = base[0] + ii[0]*size;
	ref[1] = base[1] + ii[1]*size;
	ref[2] = base[2] + ii[2]*size;

	double minDist = 4* pow(size, 2);

	__int64 min = -1;
	for(__int64 i=0; i<v.length; i++){


		double *node = v.getNode(i);
		double temp = pow( (node[0]-ref[0]), 2);
		temp += pow( (node[1]-ref[1]), 2);
		temp += pow( (node[2]-ref[2]), 2);

		if(temp<minDist){
			minDist = temp;
			min = i;
		}
	}

	if(min==-1){
		printf("Error in getRemainVtx, could find the remaining vtx with distance less than the longest distance possible.");
		throw StoppingException("\n");
	}

	remainVId = min;

}



/**
 * This function will try to find the appropriate vtx for this sub cell
 *
 * This function will go over all the 3 faces that the vtx is attached to
 *
 * select the vtx that will form a face with all the
 *
 * @param subCellId the position of the current subcell in one of the eighth
 * @param elem the element that will be formed with the vtx of this subcell
 * @param vId the list of all the vtices
 * @param vFace the faces bnds for the corresponding vtx
 * @choice the array to hold the appropriate vtx and inappropriate bnd sign
 *         assumption are: appropriate vtx will be put at the beginning, if no one found, then put -1
 *                         inappropriate bnd sign may be multiple will be put after the appropriate ones
 *                         with minus sign the formular is (-bnd-1)
 *
 */

void Contour::checkBndSignWithElem(int subId, __int64 prtOcId, geoNodeArr &v, pointerArr &vFace, intArr &choice){


	__int64 pii[3];
	ot->ocId2ijk(prtOcId, pii[0], pii[1], pii[2]);
	__int64 curLvl = ot->getLvlOfOcId(prtOcId);

	// hold the vtx id of the 8 vtices of the cell
	__int64 thisOcId; // the ocId used to find the cell using getNeighLeafCellVtx

	__int64 elem[8];
	thisOcId = ot->ijk2OcId( pii[0]+subId%2, pii[1]+(subId/2)%2, pii[2]+(subId/4), curLvl);

	ot->getNeighOcId4Elem(thisOcId, elem, curLvl);

	for(__int64 k=0; k<8; k++){
		if(k+subId==7)
			continue;
		int coll = ot->getColl(elem[k]);
		if(coll==1)
			elem[k] = ot->getChildLeafId(elem[k], k, ot->botLvl-1);

	}


    intArr cannotbe, canbe;
    cannotbe.copy(ot->getBnd(elem[subId]));

	int pos = 7- subId;
	int i[3];
	i[0] = pos%2;
	i[1] = (pos/2)%2;
	i[2] = (pos/4);

	int neiFace[9][3]={
			{0, 1, 0},
			{0, 1, 1},
			{0, 0, 1},
			{1, 0, 0},
			{1, 0, 1},
			{0, 0, 1},
			{1, 0, 0},
			{1, 1, 0},
			{0, 1, 0},
	};

	int vv[4]; // the four vtx that will form a face
	intArr  *fb[4];
	vv[0] = pos;

	for(int f=0; f<3; f++){//iterate through the three neighbor faces of the given vtx
		vv[1] =   (  i[0]+neiFace[f*3  ][0])%2
				+ ( (i[1]+neiFace[f*3  ][1])%2 )*2
				+ ( (i[2]+neiFace[f*3  ][2])%2 )*4;
		vv[2] =   (  i[0]+neiFace[f*3+1][0])%2
				+ ( (i[1]+neiFace[f*3+1][1])%2 )*2
				+ ( (i[2]+neiFace[f*3+1][2])%2 )*4;
		vv[3] =   (  i[0]+neiFace[f*3+2][0])%2
				+ ( (i[1]+neiFace[f*3+2][1])%2 )*2
				+ ( (i[2]+neiFace[f*3+2][2])%2 )*4;


		fb[1] = ot->getBnd(elem[vv[1]]);
		fb[2] = ot->getBnd(elem[vv[2]]);
		fb[3] = ot->getBnd(elem[vv[3]]);

		//find if the two immediate neighbor vtx share same bnd sign
		//if there is common sign, and it is the same with the vtx selected with this subcell
		//then the vtx with this common bnd sign should be the appropriate one
        //
        //
        //
        //
        //
        //The algo is to find the appropriate bnd sign for this cell
        //logic is: if one of the vtx will form a quad on the bnd face, 
        //             then this vtx is appropriate
        //          if one of the vtx share the same sign as the hex diagonal vtx bnd
        //             then this vtx is not appropriate for this cell 
		//             this part is done by the calling function
		//          if the vtx share the same sign of quad diagonal but not the same
		//             as the edge vtx sign, then this bnd sign is not approperiat
        //
        //
        //The assumption is that there is only one appropriate vtx 
        //and it will be put at the beginning of the choice array
        //if there is no definite appropriate vtx 
        //then -1 will be put at the beginning of the choice array
        //
        //inappropriate vtx bnd sign can be multiple, 
        //and they will put after the appropriate vtx in the choice array
        //if no definite inappropriate vtx found,
        //then nothing will be put after the appropriate vtx in the choice array
        //

        __int64 common = -1;

		if(!fb[1] && fb[3] && fb[2]){//one edge neigh is not set
			for(int j=0; j<fb[3]->length; j++){//find the common face bnd (if there is one)
				if( (*fb[3])[j]==-1)
					continue;
				if( fb[2]->has((*fb[3])[j])){ // found common
					common = (*fb[3])[j];
					break;
				}
				else { // not the same as the edge vtx, then have to check 
					   // if it is the same with the vtx
					   // if the same then have to put the bnd sign into the choice array 
					   // as inappropriate sign
					for(__int64 vv=0; vv<vFace.length; vv++){
						intArrSorted * thisFace = (intArrSorted*) vFace[vv];
               			if(thisFace->has( (*fb[3])[j]) )
							choice.add((*fb[3])[j]);
					}
				}
			}
		}
		else if( !fb[3] && fb[1] && fb[2]){
			for(int j=0; j<fb[2]->length; j++){
				if( (*fb[2])[j]==-1)
					continue;
				if( fb[1]->has((*fb[2])[j])){
					common = (*fb[2])[j];
					break;
				}
			}
		}

		else if( fb[1] && fb[2] && fb[3]){ // the diagonal vtx
			for(int j=0; j<fb[2]->length; j++){
				__int64 fcur = (*fb[2])[j];
				for(int k=0; k<vFace.length; k++){//iterator for the each of the vtx's bnd signs
					intArrSorted* thisFace = (intArrSorted*) vFace[k];
					if(thisFace->has(fcur)){
						if( (! fb[1]->has(fcur) && (*fb[1])[0] != -1)
								|| (! fb[3]->has(fcur) && (*fb[3])[0] != -1) ){ // conflict, same as diagonal, but adjcent edge is not the same
							cannotbe.add(fcur);
						}
					}
				}
			}

			for(int j=0; j<fb[1]->length; j++){
				if( (*fb[1])[j]==-1)
					continue;
				if( fb[3]->has((*fb[1])[j])){
					common = (*fb[1])[j];
					break;
				}
			}
		}

		if(common == -1)
    		continue;
		else{
			for(__int64 vv=0; vv<vFace.length; vv++){

				intArrSorted * bnds = (intArrSorted*)vFace[vv];
                if(bnds->has(common)){
                	canbe.add(vv);
				}
			}
        }
    }

	if(canbe.length>0){
		choice.appendArr(&canbe);
	}

	if(cannotbe.length>0){
		for(int i=0; i<cannotbe.length; i++){
			choice.add(-cannotbe[i]-1);
		}
	}

}



void Contour::vtxToFile(){

	char* fullFilePath = NULL;
    getFullFilePath(&fullFilePath, fName, ".mesh");
    meshFile = fopen(fullFilePath, "w");
	
    fprintf(meshFile, "%lld \n", vtx->length);
    for( __int64 i =0; i<vtx->length; i++){
        double *x;
        x = vtx->getCoord(i);
        fprintf(meshFile, "%f %f %f %lld ",
                x[0], x[1], x[2], vtx->getZone(i));
        intArr* bnd = vtx->getBnd(i);
        if(bnd){
			for(__int64 j=0; j<bnd->length; j++){
				fprintf(meshFile, "%lld ", (*bnd)[j] );
			}
 		}
 		else {
 			fprintf(meshFile, "%lld ", -1 );
 		}

 		fprintf(meshFile, "\n");

    }
 
    fclose(meshFile);
    free(fullFilePath);
	delete(vtx);


}

bool Contour::getVtxHelper(__int64 ocId, double v[3]){



	__int64 cellId = ot->getCellId(ocId);
	if(cellId == -1)
		return false;
	ocCellArr::ocCell* cur = ot->cells->getCell(cellId);
	__int64 curOcId = ocId;
	__int64 curLvl = ot->getLvlOfOcId(curOcId);


	if(!(cur->fc) && !(cur->nd)){ // not on the boundary will add center point
		double x[3], size;
		ot->getBaseCellNodeCoord(curOcId, x);
		size = ot->getSizeOfCellAtLvl(curLvl)/2;
		v[0] = x[0] + size;
		v[1] = x[1] + size;
		v[2] = x[2] + size;

		return true;

	}

	if(cur->nd && cur->nd->length == 1){// has node inside, then use the node as vtx
		GM->getNode( (*(cur->nd))[0], v);

		return true;
	}
	else if(cur->fc && cur->fc->length == 1){ //has one face


		__int64 bnd;
	    findVtx41FaceHelper(curOcId, (* (cur->fc))[0], v, bnd);

		return true;

	}
	else if(cur->fc && cur->fc->length == 2) {//two faces, should work this issue further
		                                      //how about multiple faces. it is possible?
		geoNodeArr vv(1);
		findVtx42FaceHelper(curOcId, (*(cur->fc))[0], (*(cur->fc))[1], vv);
		if(vv.length!=1)
			return false;
		double *nn = vv.getNode(0);
		v[0] = nn[0];
		v[1] = nn[1];
		v[2] = nn[2];
		return true;

	}
	else if(cur->fc && cur->fc->length > 2) {//two faces, should work this issue further
		                                      //how about multiple faces. it is possible?
		geoNodeArr vv;
		findVtx4MultiFaceHelper(cur, vv);
		if(vv.length>1)
			return false;
		double *nn = vv.getNode(0);
		v[0] = nn[0];
		v[1] = nn[1];
		v[2] = nn[2];
		return true;

	}
	else{

		fprintf(stdout, "ocId = %lld, numFC= %lld,  numND = %lld\n", cur->ocId, cur->fc->length, cur->nd->length);
		throw StoppingException("Error: in getVtx, multiple faces or nodes in one cell, "
				                           "not known situation.\n");
	}




}



bool Contour::checkAndUpdateAllVtxOnBndButOnDiffentBndElem(__int64 cell[8]){



	return false;

}
