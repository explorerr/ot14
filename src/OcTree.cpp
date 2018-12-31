/*
 * OcTree.cpp
 *
 *  Created on: Apr 20, 2009
 *      Author: zrui
 *      major revision: June 1, 2010
 */
#include "stdafx.h"
#include "OcTree.h"



OcTree::OcTree(geoModel& GM, double userElementSize[2],  __int64 maxLevelDifference, int domMode, int extMode, bool uniform){


	this->GM = &GM;
	this->extMode = extMode;
	this->domMode = domMode;

	elemFaces[0][0] = 0; elemFaces[0][1] = 2; elemFaces[0][2] = 6; elemFaces[0][3] = 4;
    elemFaces[1][0] = 0; elemFaces[1][1] = 4; elemFaces[1][2] = 5; elemFaces[1][3] = 1;
    elemFaces[2][0] = 0; elemFaces[2][1] = 1; elemFaces[2][2] = 3; elemFaces[2][3] = 2;
    elemFaces[3][0] = 7; elemFaces[3][1] = 3; elemFaces[3][2] = 1; elemFaces[3][3] = 5;
    elemFaces[4][0] = 7; elemFaces[4][1] = 6; elemFaces[4][2] = 2; elemFaces[4][3] = 3;
    elemFaces[5][0] = 7; elemFaces[5][1] = 5; elemFaces[5][2] = 4; elemFaces[5][3] = 6;

    pCurFeatureCells = NULL;
    pNxtFeatureCells = NULL;

    maxDiff = maxLevelDifference;


    double myEPSILON = EPSILON;
	double minDist = GM.shortestDistanceInModel();

	if(minDist < myEPSILON ){
		throw StoppingException("Error in OcTree constructor, shortest distance in domain is zero.\n");

	}

	this->uniform = uniform;

	if(uniform){

		if(userElementSize[0] < minDist) {
			otCellSizeMax = userElementSize[0];
			otCellSizeMid = userElementSize[0];
			otCellSizeMin = userElementSize[0];		
		}

		else {
			otCellSizeMax = minDist;
			otCellSizeMid = minDist;
			otCellSizeMin = minDist;		

		}
	}

	else{

		if(minDist> userElementSize[1]){
			otCellSizeMax = userElementSize[1];
			otCellSizeMid = userElementSize[1];
			otCellSizeMin = userElementSize[0];
		}
		else if(minDist > userElementSize[0]){
			otCellSizeMax = userElementSize[1];
			otCellSizeMid = minDist;
			otCellSizeMin = userElementSize[0];
		}
		else{
			otCellSizeMax = userElementSize[1];
			otCellSizeMid = userElementSize[0];
			otCellSizeMin = minDist;
		}


	}

	getLevelLimits();
	
	strLvl = topLvl<4 ? topLvl : 4;

    numCellAtLvl 	= (__int64 *)   malloc(sizeof(__int64 )    * (botLvl+1));
    startOcIdAtLvl 	= (__int64 *)	malloc(sizeof(__int64 )    * (botLvl+1));
    cellSize     	= ( double *)   malloc(sizeof( double )    * (botLvl+1));
    dim          	= (__int64 *)   malloc(sizeof(__int64 )    * (botLvl+1));

    for(__int64 i = 0; i <= botLvl; i++){
    	numCellAtLvl[i] = ((unsigned __int64 )1 )<<(i*3);
    	cellSize[i]     = span/( ((unsigned __int64 ) 1)<<i);
    	dim[i]          = ((unsigned __int64 )1)<<i;
    }
	startOcIdAtLvl[0] = 0;
    for(__int64 i=1; i<=botLvl; i++){
    	startOcIdAtLvl[i] = numCellAtLvl[i-1] + startOcIdAtLvl[i-1];
    }

    totlNumCell = startOcIdAtLvl[botLvl]+numCellAtLvl[botLvl];

    printf("Cell size at top=%f\nCell size at mid=%f\nCell size at bottom=%f\n", cellSize[topLvl], cellSize[midLvl], cellSize[botLvl]);

//    cout<<"totalNumCell="<<totlNumCell<<endl;
    tree = NULL;
    cells = NULL;
	
	

}

OcTree::~OcTree() {

	fclose(addFile);

	addFile = fopen("add", "rb");

	Tree** buffer = (Tree**) malloc(sizeof(Tree*)*1000);
	int ret = fread(buffer, 1000, sizeof(Tree*), addFile); 

	while(ret<=1000 && ret!=0){
		for(int i=0; i<ret; i++)
			free( buffer[i]);
		ret = fread(buffer, 1000, sizeof(Tree*), addFile); 
	}
	fclose(addFile);

	remove("add");

 /*   if(tree != NULL){
    	for (__int64 i=0; i< numCellAtLvl[topLvl]; i++){
    		if(tree[i].cld != NULL)
    			deleteTree(tree[i].cld);
    	}
    	free(tree);
    }*/
    if(cellSize != NULL)
        free(cellSize);
    if(dim != NULL)
        free(dim);
    if(numCellAtLvl != NULL)
        free(numCellAtLvl);
    if(startOcIdAtLvl != NULL)
    	free(startOcIdAtLvl);

}

void OcTree::deleteTree(Tree *t){


	if(t){
		for(__int64 i=0; i<8; i++){
			if(t[i].cld){
				deleteTree( t[i].cld);
			}
		}
		free(t->cld);

	}
}

void OcTree::OTConstruct(){

   //initialize the otArr

	cells = new ocCellArr();

    tree = new Tree[numCellAtLvl[strLvl]];
    for(__int64 i=0; i<numCellAtLvl[strLvl]; i++){
    	tree[i].r = -1;
    	tree[i].cld = NULL;
    }

    modelOnGridCheck();

    collapseTree();

	cout<<"length of cell: "<<cells->length<<endl;


}

void OcTree::collapseTree(){

    //collapse all the cells, the decision is based on whether there are features in the cell,
	//but not on whether the cell is in the domain
	//the tree structure is designed to hold the cell id of a leaf
	//leaf is a octree elements, which may have vtx or may not have a vtx, based on if it is in the domain
	//the "tree Array" will hold the top level octree ids, whether it is a leaf or not
	//the tree structure is also a map between ocId and the cellId in the ocCellIdArr
	//ocCellId will store all the tree nodes, which have been collapsed, either a leaf or parent of leaves
	//the cell may have vtx or not, depending on if it is within the domain and if it is a leaf id.
	//in other words, the tree is a mas between ocid and cellId
	//the cellArr is a holder for leaf cells and their parents


	addFile = fopen("add", "wb");

	if(!addFile){

		throw StoppingException("add file open failed.\n");

	}

	__int64 curOcId;


	pCurFeatureCells =  new intArr(1000);
	pNxtFeatureCells =  new intArr(1000);

	pCurCellNeedFullNeighColl = new intArr(100);
	pNxtCellNeedFullNeighColl = new intArr(100);
	pCurCellNeedNeighCheck    = new intArr(100);
	pNxtCellNeedNeighCheck    = new intArr(100);


	cout<<"start creating tree"<<endl;


	__int64 curZ;


	__int64 curCellId;
	
	for(__int64 i=0; i<numCellAtLvl[strLvl]; i++){ // first run at top level
    	                                       // set all the node as leaf


    	curOcId = startOcIdAtLvl[strLvl] + i;

		cells->add();
    	tree[i].r = cells->length - 1;
	   	cells->setOcId( cells->length-1, curOcId );
    	
		bool feature = findFcNdInGrdCell(curOcId, strLvl);
		curZ = getZoneIdForOneCell(curOcId, strLvl);

		if( feature )
    		pCurFeatureCells->add(cells->length-1);
		else if(uniform && curZ!=-1)
			pCurFeatureCells->add(cells->length-1);

		
		cells->setZone( cells->length-1, curZ );
 
    }

	if(uniform){

		for(__int64 lvl=strLvl; lvl<botLvl; lvl++){
			
			cout<<"collapsing level="<<lvl<<"  numFeatureCells="<<pCurFeatureCells->length<<endl;

			for(__int64 i=0; i<pCurFeatureCells->length; i++){
				
				curCellId = (*pCurFeatureCells)[i];
			   	curOcId = cells->getOcId(curCellId);
				

				if(i%1000==0)
					cout<<"collapsing ocId="<<curOcId<<" lvl="<<getLvlOfOcId(curOcId)<<" ("<<i<<" out of "<<pCurFeatureCells->length<<")"<<endl;
		//		if(curCellId==-1)
			//		continue;				

				createChildForTheOcNodeUniform(curOcId, lvl, curCellId);
				
			}
			
			pCurFeatureCells->copy(pNxtFeatureCells);
			pNxtFeatureCells->clear();

		}
		

		delete(pCurCellNeedFullNeighColl);
		delete(pNxtCellNeedFullNeighColl);
		delete(pCurFeatureCells);
		delete(pNxtFeatureCells);

		return;
	}




/*	for(__int64 i=0; i<numCellAtLvl[topLvl]; i++){ // first run at top level
    	                                       // set all the node as leaf
    	curOcId = startOcIdAtLvl[topLvl] + i;

    	if(i == 0)
    		cout<<"getting first tree root: "<<curOcId<<endl;

		cells->add();
    	tree[i].r = cells->length - 1;
//    	cells->setColl(cells->length -1, 0);
    	cells->setOcId(cells->length -1, curOcId);
    	if( findFcNdInGrdCell(curOcId, curLvl) )
    		pCurFeatureCells->add(curOcId);
		
    	curZ = getZoneIdForOneCell(curOcId, topLvl);
    	cells->setZone( cells->length-1, curZ );

    }
*/

    for(__int64 lvl=strLvl; lvl<midLvl; lvl++){ // iterate through topLvl to midLvl
    	                                                 // mid level is designed to control the cell sizes, for cells with simple features
    	                                                 // e.g. cells with single face
    	                                                 // cells with unresolved features should be further collapsed under midLvl

		cout<<"collapsing level="<<lvl<<"  numFeatureCells="<<pCurFeatureCells->length<<endl;

    	collapseFeatureCellFromStrToMidLvl();

		pCurFeatureCells->copy(*pNxtFeatureCells);
		pNxtFeatureCells->clear();


    }


    for(__int64 lvl=midLvl; lvl<botLvl; lvl++){//further collapse the tree,
    	                                                //if there are cells with unresolved features, then they will be further collpased
    	                                                //at the same time, its full neighbor will be also checked, to make sure the neighboring cells are at the same level
    	                                                // in order to prepare for the unresolved feature cell processing method in the contouring class
		cout<<"collapsing level="<<lvl<<"  numFeatureCells="<<pCurFeatureCells->length<<endl;
    	collapseFeatureCellBeneathMidLvl();

		pCurFeatureCells->copy(*pNxtFeatureCells);
		pNxtFeatureCells->clear();


	}


    while(pCurFeatureCells->length>0){

 //   	cout<<"fnial clean up"<<endl;
    	collapseFeatureCellBeneathMidLvl();
		pCurFeatureCells->copy(*pNxtFeatureCells);
		pNxtFeatureCells->clear();


    }
	delete(pCurCellNeedFullNeighColl);
	delete(pNxtCellNeedFullNeighColl);
	delete(pCurFeatureCells);
	delete(pNxtFeatureCells);


    cout<<"finish create tree"<<endl;

}





/**
  * This function will collapse the cells in the curFeatureCell array 
  * for level from top to mid lvl
  * This function will not stop iterating util all the cells are callapsed and
  * all the neighbors are check with level difference less than 1
  *
  *
  */

void OcTree::collapseFeatureCellFromStrToMidLvl(){

    	__int64 curCellId;
		__int64 curOcId;
		__int64 curLvl;


		pNxtCellNeedNeighCheck->clear();
		pNxtCellNeedFullNeighColl->clear();

		for(__int64 i = 0; i<(__int64)pCurFeatureCells->length; i++){ //iterate through the leaf cells only




    		curCellId =(*pCurFeatureCells)[i];
			curOcId	  = cells->getOcId(curCellId);


			if(curCellId==-1)//already collapsed
				continue;

    		curLvl = getLvlOfOcId(curOcId);

			if(i%1000==0)
					cout<<"collapsing ocId="<<curOcId<<" lvl="<<curLvl<<" ("<<i<<" out of "<<pCurFeatureCells->length<<")"<<endl;

//    		if(i == 0)
//	    		cout<<"getting child for first tree root: "<<curOcId<<endl;

    		if(getColl(curOcId, curLvl) <0)
                continue;
			if(curLvl<topLvl) { //no need to check feature, will collapse
				
				createChildForTheOcNode(curOcId, curLvl, curCellId);
				cellNeighUpdate();
				
			}
			else if(hasFeatureInGrdCell(curOcId, curLvl)){

				createChildForTheOcNode(curOcId, curLvl, curCellId);

				cellNeighUpdate();

			}
		}

}


/** 
  * This function will collapse the cells in the curFeatureCell array 
  * for cells in level through mid to bot
  * This function will not stop iterating util all the cells are callapsed and
  * all the neighbors are check with level difference less than 1
  * 
  * The difference of cell callpsing for cells above mid from that beneath of the bot are:
  * the full neighbor of the cell beneath the mid will also be collapsed 
  */

void OcTree::collapseFeatureCellBeneathMidLvl(){

    	__int64 curCellId;
		__int64 curOcId;
		__int64 curLvl;


		pNxtCellNeedNeighCheck->clear();
		pNxtCellNeedFullNeighColl->clear();

		for(__int64 i = 0; i<(__int64)pCurFeatureCells->length; i++){ //iterate through the leaf cells only

   		
			curCellId =(*pCurFeatureCells)[i];
			curOcId	  = cells->getOcId(curCellId);
			if(curOcId == 10155240)
				cout<<"KKK"<<endl;
			if(i%1000==0)
					cout<<"collapsing ocId="<<curOcId<<" lvl="<<getLvlOfOcId(curOcId)<<" ("<<i<<" out of "<<pCurFeatureCells->length<<")"<<endl;
 
			if(curCellId==-1)//already collapsed
				continue;
			curLvl = getLvlOfOcId(curOcId);

			if(getColl(curOcId, curLvl) <0 || getZone(curOcId, curLvl) ==-1)
				continue;
			if(getColl(curOcId, curLvl) ==1 )//may have already been collapsed because of neighbors
				continue;
			if(curLvl==botLvl)
				continue;

			if( featureCheck4CellsBelowMidLvl(curOcId, curLvl) ){

				createChildForTheOcNode(curOcId, curLvl, curCellId);

				cellNeighUpdate();

				__int64 *nei = (__int64*)malloc(sizeof(__int64)*27);
			    getFullNeighOcId(curOcId, nei, curLvl);

			    for(__int64 k=0; k<27; k++ ){
			    	if(nei[k]==-1)
			    		continue;
			    	__int64 coll=getColl(nei[k]);
			    	if( coll == 1)
			    		continue;
			    	else if( coll == 0){
			    		curCellId = getCellId(nei[k]);
			    		createChildForTheOcNode(nei[k], curLvl, curCellId);
			    		cellNeighUpdate();
			    	}
			    	else{
			    		free(nei);
			    		printf("Error: in collapseTree, neigh of hasNoIntersectFaceCell is not a leaf.");
			    		throw StoppingException("\n");
			    	}
			    }

			    free(nei);
			}
			else{
				collapseFullNeigh(curOcId, curLvl);
	    		cellNeighUpdate();
			}

		}



}


void OcTree::cellNeighUpdate(){

	__int64 curOcId, curLvl;

	pCurCellNeedNeighCheck->copy( pNxtCellNeedNeighCheck );
	pNxtCellNeedNeighCheck->clear();

	pCurCellNeedFullNeighColl->copy( pNxtCellNeedFullNeighColl );
	pNxtCellNeedFullNeighColl->clear();

	int cnt=0;


	while(pCurCellNeedFullNeighColl->length>0 || pCurCellNeedNeighCheck->length >0){



			for(__int64 i=0; i<pCurCellNeedNeighCheck->length; i++){

				curOcId =  (*pCurCellNeedNeighCheck)[i];
				curLvl  =  getLvlOfOcId(curOcId);


				if(getColl(curOcId, curLvl) <1)
					continue;

				checkAndUpdateNeiCollWithDiffLimit(curOcId, curLvl);

			}


			for(__int64 i=0; i<pCurCellNeedFullNeighColl->length; i++){
				curOcId =  (*pCurCellNeedFullNeighColl)[i];
				curLvl  =  getLvlOfOcId(curOcId);
				collapseFullNeigh(curOcId, curLvl);

			}



		cnt++;
		pCurCellNeedNeighCheck->copy( pNxtCellNeedNeighCheck );
		pNxtCellNeedNeighCheck->clear();
		pCurCellNeedFullNeighColl->copy( pNxtCellNeedFullNeighColl );
		pNxtCellNeedFullNeighColl->clear();

	}

}


/**
 * This function will return the pointer to the tree node of the ocId
 */


Tree * OcTree::getTreeNode(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getTreeRoot, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in getTreeRoot, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           lvl, topLvl, botLvl);
		throw StoppingException("\n");
    }

    if(lvl==strLvl)
    	return &tree[ocId- startOcIdAtLvl[strLvl]];

    __int64 *prt = (__int64*)malloc(sizeof(__int64) * (botLvl +1 ));
    __int64 *pos = (__int64*)malloc(sizeof(__int64) * (botLvl +1 ));

    prt[lvl] = ocId;
    
	for(__int64 i=lvl; i>strLvl; i--){
    	getParentAndPosition(prt[i], i, prt[i-1], pos[i]);
    }
    
	Tree* curT = &tree[prt[strLvl]- startOcIdAtLvl[strLvl]];

    for(__int64 i=strLvl+1; i<=lvl; i++){
    	if(!curT->cld){
    		curT = NULL; break;
    	}
    	curT = &curT->cld[pos[i]];
    }

   	free(prt);
   	free(pos);
   	return curT;


}


void OcTree::checkAndUpdateNeiCollWithDiffLimit(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in checkAndUpdateNeiCollWithDiffLimit, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in checkAndUpdateNeiCollWithDiffLimit, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 *nei = (__int64*) malloc(sizeof(__int64)*27);
    getFullNeighOcId(ocId, nei, lvl);


    __int64 prtOcId, prtCellId;
    for(__int64 i=0; i<27; i++){

    	if(nei[i]==-1)
    		continue;

    	if(getColl((__int64)nei[i], lvl) >= 0)
    		continue;

    	//objective is to make sure that neighbors are >=0

    	prtOcId = getParentId((__int64)nei[i], lvl);

    	prtCellId = getCellId(prtOcId, lvl-1);

    	if(getColl(prtOcId, lvl-1) == -1){
    			cout<<"unpredicted situation, node "<<ocId<<" "<<i<<"th neighbor CollLvl is below -1. "<<nei[i]<<endl;
    	}

    	createChildForTheOcNode(prtOcId, lvl-1, prtCellId);


    }

    free(nei);
}

void OcTree::collapseFullNeigh(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in collapseFullNeigh, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in collapseFullNeigh, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    
    __int64 *nei = (__int64*) malloc(sizeof(__int64)*27);
    getFullNeighOcId(ocId, nei, lvl);

//   cout<<"collapsingFullNeigh "<<ocId<<endl;

    __int64 prtOcId, prtCellId;
    for(__int64 i=0; i<27; i++){

    	if(nei[i]==-1)
    		continue;

    	if(getColl(nei[i], lvl) >= 0)
    		continue;

     	//objective is to make sure that neighbors are >=0

    	prtOcId = getParentId(nei[i], lvl);

    	prtCellId = getCellId(prtOcId, lvl-1);

    	if(getColl(prtOcId, lvl-1) == -1){
    			cout<<"unpredicted situation in collapseFullNeigh, node "<<ocId<<" "<<i<<"th neighbor CollLvl is below -1. "<<nei[i]<<endl;
    	}
 	    createChildForTheOcNode(prtOcId, lvl-1, prtCellId);
    }
    free(nei);
}

/**
 * This function will update the coll level of the ocId
 * find all the child id
 * add to the cell array
 * and find features for the child
 *
 */
void OcTree::createChildForTheOcNode(__int64 ocId, __int64 lvl, __int64 cellId){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in creatChildForTheOcNode, ocId out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in creatChildForTheOcNode, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

	//cout<<"Create child for: "<<ocId<<endl;

	if(ocId==583523127)
		cout<<"JJJ"<<endl;

	Tree *curT = getTreeNode(ocId, lvl);
	if(curT==NULL || getColl(ocId, lvl) !=0 ){
		printf("Error: in creatChildForTheOcNode, ocId is not a leaf: %lld, can only create child for leaf node.\n",
			           ocId);
		cout<<"tree="<<curT<<" coll="<<getColl(ocId, lvl)<<endl;
		cout<<"tree.r="<< curT->r ;
		cout<<" tree.cld[0]=" << curT->cld <<endl;
		throw StoppingException("\n");
	}

	pNxtCellNeedNeighCheck->add(ocId);


	__int64 *cld = (__int64*) malloc(sizeof(__int64)*8);
	getChildOcId(ocId, cld, lvl);
//	setColl(ocId, lvl, 1);
    curT->cld = (Tree*) malloc(sizeof( Tree ) * 8 );
	Tree* buf[1];
	buf[0] = (curT->cld);
	fwrite(buf, 1, sizeof( Tree *), addFile); 

    intArr *pnd, *pfc;
	pnd = new intArr(2);
	pfc = new intArr(2);
	pnd->copy( getNd(ocId, lvl));
	pfc->copy( getFc(ocId, lvl));
    

    __int64 curZ;
    __int64 prtZ = cells->getZone(cellId);


    //put the first child at the prt cell,  then there are only leaf cells in the cell array.
    __int64 pCId = curT->r;

	setColled(ocId, lvl);
//    curT->r = -1;//set root as -1, because the current grid cell will no longer be a leaf cell,
                 //thus there will be no cell for this grid cell in the cell array


	__int64 curCId;
    for(__int64 j=0; j<8; j++){

    	if(j==0){
    		curCId = pCId;
    	}
    	else{
    		cells->add();
    		curCId = cells->length-1;
    	}

		curT->cld[j].cld = NULL;
		curT->cld[j].r = curCId;


//		cells->setColl(cells->length-1, 0);
		cells->setOcId(curCId, cld[j]);

		findFcNdInGrdCell(cld[j], lvl+1, pnd, pfc);
		//figure out the zone sign
		if (prtZ == -2)
			curZ = getZoneIdForOneCell(cld[j], lvl+1);
		else
			curZ = prtZ;
		cells->setZone(curCId, curZ);
		//put the cell id into the feature cells list for the curLvl
		if( lvl<topLvl){// smaller than the top lvl will not check feature, will collapse
			pNxtFeatureCells->add(curCId);
		}
		else if(lvl<=midLvl ){ // different feature checking criterion for cell above the midLvl or cells below the midLvl
			if(hasFeatureInGrdCell(cld[j],lvl+1) )
				pNxtFeatureCells->add(curCId);
		}
		else if(lvl<botLvl-1 && lvl>midLvl ) {
			if( featureCheck4CellsBelowMidLvl(cld[j],lvl+1)){
				pNxtFeatureCells->add(curCId);
			}
		}
		else if(lvl==botLvl-1){
			if( featureCheck4CellsBelowMidLvl(cld[j],lvl+1)){
//				cout<<"adding to pCellNeedFullNeighColl "<<cld[j]<<"\tlen="<<pNxtCellNeedFullNeighColl->length<<endl;
				pNxtCellNeedFullNeighColl->add(cld[j]);
			}
		}

	}

    free(cld);
	delete(pnd);
	delete(pfc);

}

void OcTree::createChildForTheOcNodeUniform(__int64 ocId, __int64 lvl, __int64 cellId){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in creatChildForTheOcNode, ocId out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in creatChildForTheOcNode, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

	Tree *curT = getTreeNode(ocId, lvl);
	if(curT==NULL || getColl(ocId, lvl) !=0 ){
		printf("Error: in creatChildForTheOcNode, ocId is not a leaf: %lld, can only create child for leaf node.\n",
			           ocId);
		cout<<"tree="<<curT<<" coll="<<getColl(ocId, lvl)<<endl;
		cout<<"tree.r="<< curT->r ;
		cout<<" tree.cld[0]=" << curT->cld <<endl;
		throw StoppingException("\n");
	}


	__int64 cld[8];
	getChildOcId(ocId, cld, lvl);

    curT->cld = (Tree*) malloc(sizeof( Tree ) * 8 );
	Tree* buf[1];
	buf[0] = (curT->cld);
	fwrite(buf, 1, sizeof( Tree *), addFile); 
    intArr *pnd, *pfc;

	pnd = new intArr(2);
	pfc = new intArr(2);
	pnd->copy( getNd(ocId, lvl));
	pfc->copy( getFc(ocId, lvl));
    

    __int64 curZ;
    __int64 prtZ = cells->getZone(cellId);


    //put the first child at the prt cell,  then there are only leaf cells in the cell array.
    __int64 pCId = curT->r;

	setColled(ocId, lvl);
//    curT->r = -1;//set root as -1, because the current grid cell will no longer be a leaf cell,
                 //thus there will be no cell for this grid cell in the cell array


	__int64 curCId;
    for(__int64 j=0; j<8; j++){

    	if(j==0){
    		curCId = pCId;
    	}
    	else{
    		cells->add();
    		curCId = cells->length-1;
    	}

		curT->cld[j].cld = NULL;
		curT->cld[j].r = curCId;

		cells->setOcId(curCId, cld[j]);

		findFcNdInGrdCell(cld[j], lvl+1, pnd, pfc);
		//figure out the zone sign
		if (prtZ == -2)
			curZ = getZoneIdForOneCell(cld[j], lvl+1);
		else
			curZ = prtZ;
		cells->setZone(curCId, curZ);

		if(curZ!=-1)
			pNxtFeatureCells->add(curCId);
		//put the cell id into the feature cells list for the curLvl

	}

    
	delete(pnd);
	delete(pfc);

}

/**
 *  This function will find the nd and fc in the cell
 *
 *  assumptions:
 *      prt id is already processed, with nd and fc set
 *
 *      if nd or fc of the prt is NULL, then it means there is
 *      no fc or nd
 *
 *      if nd or fc is no NULL, then it means there is nd or fc
 *
 */


bool OcTree::findFcNdInGrdCell(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in findFcNdInGrdCell, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in findFcNdInGrdCell, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    intArr *pnd, *pfc;
    double x[3];
    double size = cellSize[lvl];
    getBaseCellNodeCoord(ocId, x);

    if(lvl==strLvl){
        pnd = NULL;    pfc = NULL;
    }
    else{
        __int64 prt = getParentId(ocId, lvl);
        pnd = getNd(prt, lvl-1);
        pfc = getFc(prt, lvl-1);
    }
    intArr *nd = NULL, *fc = NULL;
	GM->getInnerNodeId(x, size, &nd, pnd);
	GM->getInterFaceId(x, size, &fc, pfc);

	bool ret= false;
    if(fc && fc->length>0 ){
    	setFc(ocId, lvl, fc);
    	ret = true;
    }
    if(nd && nd->length>0 ){
    	setNd(ocId, lvl, nd);
    	ret = true;
    }

    return ret;
}


bool OcTree::findFcNdInGrdCell(__int64 ocId, __int64 lvl, intArr* pnd, intArr* pfc){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in findFcNdInGrdCell, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in findFcNdInGrdCell, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }



    double x[3];
    double size = cellSize[lvl];
    getBaseCellNodeCoord(ocId, x);

    intArr *nd = NULL, *fc = NULL;
	GM->getInnerNodeId(x, size, &nd, pnd);
	GM->getInterFaceId(x, size, &fc, pfc);

	bool ret= false;
    if(fc && fc->length>0 ){
    	setFc(ocId, lvl, fc);
    	ret = true;
    }
	else if(!fc){
		setFc(ocId, lvl, NULL);
	}
    if(nd && nd->length>0 ){
    	setNd(ocId, lvl, nd);
    	ret = true;
    }
	else if(!nd){
		setNd(ocId, lvl, nd);
	}

    return ret;
}


/**
 * This fucntion will check if there is feature in the cell
 * Feature means face not on the domain bnd (bnd>=-2)
 *
 */

bool OcTree::hasFeatureInGrdCell(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in findFcNdInGrdCell, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in findFcNdInGrdCell, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    intArr *fc, *nd;
    fc = getFc(ocId, lvl);
    nd = getNd(ocId, lvl);

	if(lvl<topLvl-1)
		cout<<"KKK"<<endl;


    if((!fc && !nd) )
    	return false;
    if(nd)
    	if( GM->featureNodeCheck(nd) )
    		return true;
    if(fc){
		bool flg = GM->featureFaceCheck(fc);
    	if( GM->featureFaceCheck(fc) && (fc->length >=1 ) )
    		return true;
	}
    return false;
}



bool OcTree::featureCheck4CellsBelowMidLvl(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in findFcNdInGrdCell, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in findFcNdInGrdCell, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }
    intArr *fc, *nd;
    fc = getFc(ocId, lvl);
    nd = getNd(ocId, lvl);


    if(!nd && !fc) //no face or node, then not a feature cell
    	return false;

    if(nd) //has one node, thus a feature cell
    	return true;

	if(fc->length >2)
		return true;

 //   if(nd && nd->length>1)
 //   	return true;

    bool ret= false;



    if(fc){

		if( fc->length>0 ){
			double equa[4];
			for(__int64 j=0; j<fc->length; j++){
				GM->getPlanarFaceFunction((*fc)[j], equa);
				for(__int64 i=0; i<3; i++){
					if( fabs(equa[i])>0.5 )
						equa[i] = 1-fabs(equa[i]);
					else
						equa[i] = fabs(equa[i]);
					if( equa[i]>0.01 && equa[i]<0.4 )
						return true;
				}
			}
		}
		if( fc->length==1)
			return false;



		double cx[3];
		geoNodeArr intPnt(5);
		getBaseCellNodeCoord(ocId, cx);

		double size = getSizeOfCellAtLvl(lvl);
		
		for(int i=0; i<fc->length; i++){
			for(int j=i+1; j<fc->length; j++){
				if (GM->getMidPointOfIntersectionLineWithCube( (*fc)[i], (*fc)[j], cx, size, intPnt ) ) {
					
					if( intPnt.length>2 ){
						ret = true;
						break;
					}
					
				}

				else { // no intersection point between the two face, then there must be multiple vertices in this cell, then must be a feature cell
					return true;
				}
			}
		}
		if( intPnt.length != 1 )
			ret = true;
    }

	return ret;

}




/**
 * This function will calculate octree top levels and bottom level
 * Prerequisites of the this function are otCellSizeMax and otCellSizeMin
 * This function will also set the offset value for the Octree
 */
void OcTree::getLevelLimits(){
	if(otCellSizeMin <0 || otCellSizeMax <0 || otCellSizeMin> otCellSizeMax){
		printf("Error: in getLevelLimits, not valide or conflicts sizes:\n"
			   "	   Min: %f \n"
			   "       Max: %f \n", otCellSizeMin, otCellSizeMax);
		throw StoppingException("\n");
	}
	double length = GM->length;
	__int64 i=0;
	if(otCellSizeMax > length || otCellSizeMin > otCellSizeMax){
		printf("Error: in getLevelLimits, celSize out of range:\n"
			   "	   Min: %f \n"
			   "       Max: %f \n", otCellSizeMin, otCellSizeMax);
		throw StoppingException("\n");
	}

	span = 0;
//	if(!uniform)
//		otCellSizeMin /= 2;
	while( span  < length )
		span = (1<<i++)*otCellSizeMin;
	botLvl = i-1;

	i=0;
	while( span/(1<<i) > otCellSizeMid)
		i++;
	midLvl = i;

	i=0;
	while ( span/(1<<i) > otCellSizeMax )
		i++;
	topLvl = i;


	double *center = GM->domainCenter;

	offset[0] = center[0] - span/2;
	offset[1] = center[1] - span/2;
	offset[2] = center[2] - span/2;


	double mod[3];
	mod[0] = fmod( (-offset[0]), 0.15);
	mod[1] = fmod( (-offset[1]), 0.15);
	mod[2] = fmod( (-offset[2]), 0.15);
	
	if(topLvl==midLvl && topLvl==botLvl)
		uniform= true;
	else if(midLvl==botLvl)
		midLvl--;

	printf("tree levels: top=%lld, mid=%lld, bottom=%lld\n", topLvl, midLvl, botLvl);

//	printf("offset= (%f, %f, %f)\n", offset[0], offset[1], offset[2]);

//	printf("fmod offset: (%f, %f, %f)\n", mod[0], mod[1], mod[2]);

}



void OcTree::modelOnGridCheck(){

	if(GM->modelOnGridCheck(cellSize[botLvl])){
		for(__int64 i=0; i<3; i++)
			offset[i] += cellSize[botLvl]*0.1;
		for(__int64 i=0; i<3; i++)
			if(offset[i]>=0)
				offset[i] -= cellSize[botLvl]*0.499;
	}

}

void OcTree::moveGrid(double x[3]){

	for(__int64 i=0; i<3; i++)
		offset[i] += x[i];
	for(__int64 i=0; i<3; i++)
		if(offset[i]>=0)
			offset[i] -= cellSize[botLvl]*0.499;

}



/**
 * This function will update the zone id that cell belongs to
 * if this cell is on the boundary, then its zId will remain -2
 * other wise it will be updated with an zone id
 *
 * The code for exterior zone is -1
 *
 * Then this can be used for checking if the vtx need to be calculated
 * and added to the vtx array
 *
 * also will be used for elements construction, if the cell is in the
 * exterior, then no need to add the element
 *
 * Methodology:
 *
 * First check if there are face or node inside the cell,
 * the operation of getting innerNode and interFace is done in collapseTree
 * if there are face or node, then the zone is on the boundary,
 * the zId will remain -2
 * else then check the basePoint and diagonal pnt's zone id
 *
 *
 */
void OcTree::getOtCellZoneId(){

	__int64 curZ;
	__int64 ocId;
	__int64 start = startOcIdAtLvl[topLvl];
    for(__int64 i=0; i<numCellAtLvl[topLvl]; i++){//start with the top level to get the zoneId
    	ocId = i + start;
    	if( cells->getFc(tree[i].r) || cells->getNd(tree[i].r) ){// boundary cell
    		curZ = -2;
    	}
    	else {
    		curZ = getZoneIdForOneCell(ocId, topLvl);
    		cells->setZone(i, curZ);
    	}
    	getOtCellZoneIdHelper( &(tree[i]), ocId, topLvl, curZ);
    }

}

void OcTree::getOtCellZoneIdHelper(Tree* root, __int64 rootOcId, __int64 rootLvl, __int64 rootZ){
	if(!root){
		printf("Error: in getOtCellZoneIdHelper, tree is NULL\n");
		throw StoppingException("\n");
	}

    if(rootLvl<0 || rootLvl>botLvl){
		printf("Error: in getOtCellZoneIdHelper, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           rootLvl, topLvl, botLvl);
		throw StoppingException("\n");
    }

    if(!root->cld)
    	return;

    __int64 *cld = (__int64*) malloc(sizeof(__int64)*8);
    __int64 curZ;
    getChildOcId(rootOcId, cld, rootLvl);
    for(__int64 i=0; i<8; i++){

    	if(rootZ == -2)
    		curZ = getZoneIdForOneCell(cld[i], rootLvl+1);
    	else
    		curZ = rootZ;
    	setZone(cld[i], curZ);
    	Tree *newRoot = &root->cld[i];
    	getOtCellZoneIdHelper( newRoot, cld[i], rootLvl+1, curZ);
    }

    free(cld);

}



__int64 OcTree::getZoneIdForOneCell(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getZoneIdForOneCell, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
	if(lvl<0 || lvl>botLvl){
		printf("Error: in getZoneIdForOneCell, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           lvl, topLvl, botLvl);
		throw StoppingException("\n");
    }
//	__int64 ii= 719;
//	ii *= 1000000;
//	ii +=  980725;
//	if(ocId==ii)
//		cout<<"KKK"<<endl;


	__int64 cellId = getCellId(ocId, lvl);

	if(cellId != -1){

		if( cells->getFc(cellId) || cells->getNd(cellId) )// boundary cell
			return -2;

	}

	double base[3], diag[3];
    __int64 z1=-1, z2=-1, b1=-1, b2=-1;
    getBaseCellNodeCoord(ocId, base);

    for(__int64 ii=0; ii<3; ii++)
        diag[ii] = base[ii] + cellSize[lvl];

//	if(base[0]<0 && base[0]>-2 && base[1] <3 && base[1]>2 && base[2]>0 && base[2]<1)
//		cout<<"KKKK"<<endl;

    GM->getZoneOrBndId(base, z1, b1);
    GM->getZoneOrBndId(diag, z2, b2);

    if(b1 != -1 || b2 != -1){
        fprintf(stderr,"Error: in getZoneIdForOneCell, zone should be in zone, "
                "but found on the boundary: ocId=%lld\n", ocId);
        fprintf(stderr,"Base of cell: (%f, %f, %f), size=%f\n", base[0], base[1], base[2], cellSize[lvl]);
        throw StoppingException("\n");
    }

    if(z1==z2)
    	return z1;
    else
    	return -2;


}


/**
 *  This function assumes that all the ocId passed is not on the bnd of the ocTree
 *  Thus, should have eight neighboring leaf node, among which there may be duplicates
 *  Thus, it will throw exceptions if there are -1 in the nei arr
 *
 *  return  value:
 *  true: if every leaf is in the interior, and number of positive vtx is greater than 3
 *  false: if there is leaf in the exterior
 *
 *  @param ocId the leaf octree id
 *  @param nei the eight neighbor of the leaf id
 *  @param lvlDiff the lvl difference between the ith neighbor id and the ocId,
 *         1 = child lvl; 0 = cur lvl; -1 = parent lvl.
 */

bool OcTree::getNeighLeafCellVtx(__int64 ocId, __int64 nei[8]){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getNeighLeafCellVtx, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}


	__int64 coll = getColl(ocId);

	if( coll !=0 && coll != -1 ){
		printf("Error: in getNeighLeafCellVtx, the ocId is not a leaf nor a leaf-beneath: [%lld].coll= %lld\n",
			           ocId, coll);
		throw StoppingException("\n");
	}
	__int64 lvl = getLvlOfOcId(ocId);
	__int64 neiOc[8];

	getNeighOcId4Elem(ocId, neiOc, lvl);


	__int64 prtCnt=0, prtId[8]={0,0,0,0,0,0,0,0};

	if(uniform){

		__int64 curColl;
		for(__int64 i=0; i<8; i++){
			if(neiOc[i] == -1){
				fprintf(stderr, "Error: in getNeighLeafCellVtx, ocId=%lld, %lldth neighbor is -1\n", ocId, i);
				throw StoppingException("\n");
			}
			curColl = getColl (neiOc[i]);
			if(curColl==-1)
				return false;
			nei[i] = getVtx( neiOc[i]);
			if(nei[i]==-1)
				return false;
		}
	}

	else{

		for(__int64 i=0; i<8; i++){
			if(neiOc[i] == -1){
				fprintf(stderr, "Error: in getNeighLeafCellVtx, ocId=%lld, %lldth neighbor is -1\n", ocId, i);
				throw StoppingException("\n");
			}
			__int64 curColl = getColl(neiOc[i]);
			if( curColl == -1 ){
				prtId[i]=1; prtCnt++;
				neiOc[i] = getParentId(neiOc[i]);
				curColl = getColl(neiOc[i]);
				if (curColl != 0){
					fprintf(stderr, "Error: in getNeighLeafCellVtx, ocId=%lld, neighbor %lldth neighbor is not a leaf nor does its parent: [%lld]=%lld\n",
						ocId, i, neiOc[i], curColl);
					throw StoppingException("\n");
				}
			}
			else if( curColl == 1 ) {//because the max different level is 1 then this value can only be -1, 0 or 1

				neiOc[i] = getChildLeafId(neiOc[i], i, lvl);
				curColl = getColl(neiOc[i]);
				if (curColl != 0){
					fprintf(stderr, "Error: in getNeighLeafCellVtx, ocId=%lld, neighbor %lldth neighbor has child but the child is not leaf: [%lld]=%lld\n",
						ocId, i, neiOc[i], curColl);
					throw StoppingException("\n");
				}
			}
			else if( curColl < -1 || curColl > 1){
				fprintf(stderr, "Error: in getNeighLeafCellVtx, ocId=%lld, neighbor %lldth neighbor out of allowable max diff: [%lld]=%lld\n",
					ocId, i, neiOc[i], curColl);
				throw StoppingException("\n");

			}
		}
		for(__int64 i=0; i<8; i++){
			if (getZone(neiOc[i]) == -1)//outside the domain
				return false;
		}

		intArrSortedWithCnt vvs(2);
		for(__int64 i=0; i<8; i++){
			if(neiOc[i]==-1)
				throw StoppingException("Error: in getNeighLeafCellVtx, -1 found in the neigh id arr.\n");
			__int64 temp = getVtx( neiOc[i]);
			if(temp==-1)
				return false;
			else 
				nei[i] = temp;
			vvs.insert(temp);
		}
		if(vvs.length<5)
			return false;

/*		__int64 dup[8][2]= {{-2,0},{-2,0},{-2,0},{-2,0},{-2,0},{-2,0},{-2,0},{-2,0}}, cnt=0;
		for(__int64 i=0; i<8; i++){
			if(neiOc[i]==-1)
				throw StoppingException("Error: in getNeighLeafCellVtx, -1 found in the neigh id arr.\n");
			__int64 temp = getVtx( neiOc[i]);
			if(temp==-1)
				return false;
			else 
				nei[i] = temp;

			for(__int64 j=0; j<8; j++){
				if( dup[j][0] == nei[i]){
					dup[j][1]++; 
					break;
				}
				else if(dup[j][0]==-2){
					dup[j][0] =  nei[i];
					dup[j][1]++; cnt++;
					break;
				}
			}
		}

		if(cnt<5)
			return false; */
	

	}

	//check if the all the vtx on bnd, but on different bnds, should return false


	intArrSortedWithCnt bnds(2);
	__int64 curBId;
	intArr* curBnd;
	bool ret = true;
	for(__int64 i=0; i<8; i++){

		curBnd = getBnd(neiOc[i]);
		if(!curBnd){
			curBnd = getFc(neiOc[i]);
		}
		if(!curBnd) // one vtx not on the bnd
			return true;
		else if((*curBnd)[0]==-1)
			return true;
		
		for(int j=0; j<curBnd->length; j++){
			bnds.insert((*curBnd)[j]);
		}
	}


	// all the vtx are on the bnd
	for(int i=0; i<bnds.length; i++){
		if (bnds.getCnt(i)==8) // found one common bnd sign between all the vtices, this element is snapped on to one of the geometry surfaces
			return false;
	}
	//check if all the faces are zoneDef
	//if any one is not zoneDef, then return true
	//else return false;
	bool flg = false; // flg to check if there is any face not zoneDef
	for(int i=0; i<bnds.length; i++){
		
		if ( !GM->isFaceZoneDef(bnds[i]) ){
			 flg = true; break;
		}

	}
	if(!flg){
		//have to find the a point in the cell and check if the point is in the domian
		//how to find the point?
		double x[3];
		getBaseCellNodeCoord(ocId, x);
		__int64 z, b;
		GM->getZoneOrBndId(x, z, b);
		if(z!=-1) 
			flg = true;
	}
	
	return flg;
	
}

/**
 * This function is designed to facilitate for getNeighLeafOcId4Elem
 * and will return the child leaf id that should be added to the element
 * @param ocId the parent id
 * @param i the index of the parent id in the neighbor cells
 * @param lvl is the level of the parent id
 */
__int64 OcTree::getChildLeafId(__int64 ocId, __int64 i, __int64 lvl){
	__int64 cld;
    getChildOcId(ocId, cld, 7-i , lvl);
	return cld;
}

/**
 * The returned neigh follows right hand rule, and from bottom to top
 *
 * will be 0 1 2 3 for bottom face and 4 5 6 7 for top face following right hand rule
 *
 */
void OcTree::getNeighOcId4Elem(__int64 ocId, __int64 nei[8], __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getNeighOcId4Elem, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
	__int64 i, j, k, x, y, z;
	ocId2ijk(ocId, lvl, i, j, k);
	memset(nei, 0, sizeof(__int64)*8);

	if(i==0){
		nei[0] = -1; nei[2] = -1; nei[4] = -1; nei[6] = -1;
	}
	if(j == 0){
		nei[0] = -1; nei[1] = -1; nei[4] = -1; nei[5] = -1;
	}
	if(k == 0){
		nei[0] = -1; nei[1] = -1; nei[2] = -1; nei[3] = -1;
	}
  	for(__int64 ii=0; ii<8; ii++){
		if( nei[ii]!= -1){
			x = (ii%2) -1;
			y = (ii/2)%2 -1;
			z = (ii/4)%2 -1;
			nei[ii]= ijk2OcId(i+x, j+y, k+z ,lvl);
		}
	}
}

/**
 * This function will find the "Left-most" leaf node of the tree
 */

__int64 OcTree::getLeafStartOcId(){


	Tree * t = tree[0].cld;
	__int64 ocId = startOcIdAtLvl[topLvl];
	__int64 lvl = topLvl;
	__int64 firstCld;
	getChildOcId(ocId, firstCld, 0, topLvl);

	while(t[0].cld){
		t = t[0].cld;
		getChildOcId(firstCld, firstCld, 0, ++lvl);
	}
	return firstCld;

}


intArr* OcTree::getNd(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getNd, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in getNd, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           lvl, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getNd, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }
   	return cells->getNd(cellId);

}

intArr* OcTree::getNd(__int64 ocId){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getNd, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 lvl = getLvlOfOcId(ocId);
    return getNd(ocId, lvl);

}

intArr* OcTree::getFc(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getFc, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in getFc, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getFc, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }
    return cells->getFc(cellId);


}

intArr* OcTree::getFc(__int64 ocId){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getFc, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}


    __int64 lvl = getLvlOfOcId(ocId);
    return getFc(ocId, lvl);

}

__int64  OcTree::getColl(__int64 ocId, __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getColl, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in getColl, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }


    
	Tree *tn = getTreeNode(ocId, lvl);

    if(!tn){
    	return -1; // no such cell, not collapsed yet
    }
    else{

    	if(tn->r==-1){
    		return 1; // a prt cell
    	}

    	else
    		return 0; // a leaf cell
    }

/*

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
    	return -1;
    }
    else
    	cell->getColl(cellId);
*/

}

__int64 OcTree::getColl(__int64 ocId){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getColl, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 lvl = getLvlOfOcId(ocId);
    return getColl(ocId, lvl);

}


 __int64  OcTree::getZone(__int64 ocId,  __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getZone, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in getZone, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getZone, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }

    return cells->getZone(cellId);

}


 __int64 OcTree::getZone(__int64 ocId){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getZone, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

     __int64 lvl = getLvlOfOcId(ocId);
    return getZone(ocId, lvl);

}

 intArr*  OcTree::getBnd(__int64 ocId,  __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getBnd, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in getBnd, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getBnd, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }

    return cells->getBnd(cellId);

}


 intArr* OcTree::getBnd(__int64 ocId){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getBnd, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

     __int64 lvl = getLvlOfOcId(ocId);
    return getBnd(ocId, lvl);

}


__int64  OcTree::getVtx(__int64 ocId,  __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getVtx, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in getVtx, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		return -1;
    }


    return cells->getVtx(cellId);

}

__int64 OcTree::getVtx(__int64 ocId){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getVtx, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

     __int64 lvl = getLvlOfOcId(ocId);
    return getVtx(ocId, lvl);

}



void OcTree::setNd  (__int64 ocId,  __int64 lvl, intArr* nd){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setNd, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in setNd, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getFc, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }
    cells->setNd(cellId, nd);


}


void OcTree::setFc  (__int64 ocId,  __int64 lvl, intArr* fc){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getFc, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in getFc, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getFc, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }
    cells->setFc(cellId, fc);

}




void OcTree::setZone(__int64 ocId,  __int64 lvl,  __int64 z){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setZone, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in setZone, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getFc, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }
    cells->setZone(cellId, z);

}

/*
void OcTree::setColl(__int64 ocId,  __int64 lvl,  __int64 c){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setColl, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in setColl, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getFc, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }
    cells->setColl(cellId, c);


}*/








void OcTree::setBnd (__int64 ocId,  __int64 lvl,  intArr* b){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setBnd, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in setBnd, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getFc, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }
    cells->setBnd(cellId, b);

}
void OcTree::setVtx (__int64 ocId,  __int64 lvl, __int64 v){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setVtx, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in setVtx, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

    __int64 cellId = getCellId(ocId, lvl);
    if(cellId == -1){
		printf("Error: in getFc, the ocId=%lld does not exist in the cell Array\n",
			           ocId);
		throw StoppingException("\n");
    }
    cells->setVtx(cellId, v);

}


void OcTree::setNd  (__int64 ocId, intArr* nd){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getNd, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 lvl = getLvlOfOcId(ocId);
    return setNd(ocId, lvl, nd);

}

void OcTree::setFc(__int64 ocId, intArr* fc){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setFc, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 lvl = getLvlOfOcId(ocId);
    return setFc(ocId, lvl, fc);

}

void OcTree::setZone(__int64 ocId,  __int64 z){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setZone, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 lvl = getLvlOfOcId(ocId);
    return setZone(ocId, lvl, z);

}


/**
void OcTree::setColl(__int64 ocId,  __int64 c){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setColl, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 lvl = getLvlOfOcId(ocId);
    return setColl(ocId, lvl, c);

}*/
void OcTree::setBnd (__int64 ocId,  intArr* b){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setBnd, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 lvl = getLvlOfOcId(ocId);
    return setBnd(ocId, lvl, b);

}
void OcTree::setVtx (__int64 ocId, __int64 v){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setVtx, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 lvl = getLvlOfOcId(ocId);
    return setVtx(ocId, lvl, v);

}



__int64 OcTree::getCellId(__int64 ocId){

	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getCellId, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}


    __int64 lvl = getLvlOfOcId(ocId);

    return getCellId(ocId, lvl);
}



__int64 OcTree::getCellId(__int64 ocId,  __int64 lvl){

	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getCellId, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in getCellId, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }


    Tree *tn = getTreeNode(ocId, lvl);

    if(tn)
    	return tn->r;
    else
    	return -1;

}

void OcTree::setColled(__int64 ocId,  __int64 lvl){
	
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in setColled, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in setColl, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }


	Tree *tn = getTreeNode(ocId, lvl);
	


	if(tn){
		tn->r = -1;
	}
	else{
		printf("Error: in setColled, id is not a leaf %lld\n",
			           ocId);
		throw StoppingException("\n");

	}


	

}

void OcTree::setColled(__int64 ocId){

    __int64 lvl = getLvlOfOcId(ocId);

	if(ocId == 583523127)
		cout<<"LLLL"<<endl;

	return setColled(ocId, lvl);
}


void OcTree::getParentAndPosition(__int64 ocId,  __int64 lvl, __int64 &parent,  __int64 &position){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in findFeatureInGrdCell, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    if(lvl<0 || lvl>botLvl){
		printf("Error: in findFeatureInGrdCell, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }
	 __int64 i, j, k;
	ocId2ijk(ocId, i, j, k);
	i /=2; j /=2; k /=2;
	parent = ijk2OcId(i, j, k, lvl-1);

	ocId2ijk(ocId, i, j, k);
	unsigned __int8 pos=0;


	pos = (pos | (unsigned __int8)(k%2)) << 1;
	pos = (pos | (unsigned __int8)(j%2)) << 1;
	pos = (pos | (unsigned __int8)(i%2));
	position = (__int64) pos;

}


__int64 OcTree::getParentId(__int64 ocId){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getParentId, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

	 __int64 lvl = getLvlOfOcId(ocId);
	 __int64 i, j, k;
	ocId2ijk(ocId, i, j, k);
	i /=2; j /=2; k /=2;
	return ijk2OcId(i, j, k, lvl-1);
}

__int64 OcTree::getParentId(__int64 ocId,  __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getParentId, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
    if( lvl<1 || lvl>botLvl ){
		printf("Error: in findFeatureInGrdCell, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

	__int64 i, j, k;
	ocId2ijk(ocId, i, j, k);
	i /=2; j /=2; k /=2;
	return ijk2OcId(i, j, k, lvl-1);
}


void OcTree::ocId2ijk(__int64 ocId,  __int64 &i,  __int64 &j,  __int64 &k){
	if(ocId<0 ){
		printf("Error: in ocId2ijk, ocId less than 0: ocId=%lld\n", ocId);
		throw StoppingException("\n");
	}
	__int64 level = getLvlOfOcId(ocId);
	ocId2ijk( ocId, level, i, j, k);
}

void OcTree::ocId2ijk( __int64 ocId,  __int64 myLvl,  __int64 &i,  __int64 &j,  __int64 &k){
	if(ocId<0 || myLvl<0 || myLvl> botLvl){
		printf("Error: in ocId2ijk, lvl or ocId less than 0: lvl=%lld, ocId=%lld\n", myLvl, ocId);
		throw StoppingException("\n");
	}
	__int64 inLvlId = ocId - startOcIdAtLvl[myLvl];
	i = inLvlId % dim[myLvl];
	j = (inLvlId / dim[myLvl]) % dim[myLvl];
	k = (inLvlId / (dim[myLvl] * dim[myLvl]))%dim[myLvl];
}

__int64 OcTree::ijk2OcId( __int64 i,  __int64 j,  __int64 k,  __int64 lvl){

	if( i <0 || j<0 || k<0 || lvl <0){
		printf("Error: invalid information for ijk2OcId: %lld, %lld, %lld, %lld\n", i, j, k, lvl);
		throw StoppingException("\n");
	}

	__int64 start = startOcIdAtLvl[lvl];
	__int64 id = start;
	id += (__int64)k *(__int64) dim[lvl]* (__int64)dim[lvl];
	id += (__int64)j * (__int64)dim[lvl];
	id += (__int64)i;
	return id;
}


void OcTree::getBaseCellNodeCoord(__int64 ocId,  __int64 lvl, double x[3]){

	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getBaseCellNodeCoord, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
    if( lvl<0 || lvl>botLvl ){
		printf("Error: in getBaseCellNodeCoord, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }
	double size = cellSize[lvl];
	double base[3];
	__int64 ijk[3];
	ocId2ijk(ocId, lvl, ijk[0], ijk[1], ijk[2]);
	for( __int64 i=0; i<3; i++){
		base[i] = size * ijk[i];
		x[i] = base[i] + offset[i];
	}

}


void OcTree::getBaseCellNodeCoord(__int64 ocId, double x[3]){

	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getBaseCellNodeCoord, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
	__int64 lvl = getLvlOfOcId(ocId);
	getBaseCellNodeCoord(ocId, lvl, x);


}


void OcTree::getFullCellNodeCoord(__int64 ocId,  __int64 lvl, double x[8][3]){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getFullCellNodeCoord, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
    if( lvl<0 || lvl>botLvl ){
		printf("Error: in getFullCellNodeCoord, lvl out of range: %lld\n"
			   "       expecting range: (%lld, %lld)\n",
			           ocId, topLvl, botLvl);
		throw StoppingException("\n");
    }

	double size = cellSize[lvl];
	double base[3];
	getBaseCellNodeCoord(ocId, lvl, base);

	x[0][0] = base[0];			x[0][1] = base[1]; 			x[0][2] = base[2];
	x[1][0] = base[0] + size;	x[1][1] = base[1]; 			x[1][2] = base[2];
	x[2][0] = base[0];	        x[2][1] = base[1] + size; 	x[2][2] = base[2];
	x[3][0] = base[0] + size;	x[3][1] = base[1] + size;	x[3][2] = base[2];
	x[4][0] = base[0];			x[4][1] = base[1]; 			x[4][2] = base[2] + size;
	x[5][0] = base[0] + size;	x[5][1] = base[1]; 			x[5][2] = base[2] + size;
	x[6][0] = base[0];			x[6][1] = base[1] + size; 	x[6][2] = base[2] + size;
	x[7][0] = base[0] + size;	x[7][1] = base[1] + size; 	x[7][2] = base[2] + size;


}

void OcTree::getFullCellNodeCoord(__int64 ocId, double x[8][3]){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getFullCellNodeCoord, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
	__int64 lvl = getLvlOfOcId(ocId);
	getFullCellNodeCoord(ocId, lvl, x);


}


double OcTree::getSizeOfCellAtLvl( __int64 lvl){
	if(lvl<0){
		printf("Error: in getSizeOfCellAtLvl, lvl is negative: %lld\n", lvl);
		throw StoppingException("\n");

	}
	return cellSize[lvl];
}


 __int64 OcTree::getLvlOfOcId(__int64 ocId){
	if(ocId<0){
		printf("Error: in getLvlOfId, id less than 0: %lld\n", ocId);
		throw StoppingException("\n");
	}
	__int64 i=0;
	while ( i<= botLvl && ocId >= startOcIdAtLvl[i])
		i++;
	return i-1;
}

/**
 *  This function will return the ith child of the parent
 *  @param ocId the parent id
 *  @param childId the ith child's oc Id
 *  @param idx the index of the child in query
 *  @param lvl the parent level
 */
void OcTree::getChildOcId(__int64 ocId, __int64& childId,  __int64 idx,  __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getChildId, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 i, j, k;
    ocId2ijk(ocId, lvl, i, j, k);
    i *=2;  j *=2;  k *=2;
    i += idx%2;
    j += (idx/2)%2;
    k += (idx/4)%2;
    childId = ijk2OcId(i, j, k, lvl+1);
}

/**
 *  This function will return all the eight child of the parent
 *  @param ocId the parent id
 *  @param childId the children ocId
 *  @param lvl the parent level
 */
void OcTree::getChildOcId(__int64 ocId, __int64* childId,  __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getChildId, id out of range: %lld\n"
			   "       expecting range: (0, %lld)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}

    __int64 i, j, k, ii, jj, kk;
    ocId2ijk(ocId, lvl, i, j, k);
    i *=2;  j *=2;  k *=2;
    for( __int64 idx=0; idx<8; idx++){
        ii = i + idx%2;
        jj = j + (idx/2)%2;
        kk = k + (idx/4)%2;
        childId[idx] = ijk2OcId(ii, jj, kk, lvl+1);
    }
}


void OcTree::getFullNeighOcId(__int64 ocId, __int64 nei[27],  __int64 lvl){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getNeighOcId4Elem, id out of range: %lld\n"
			   "       expecting range: (0, %lldd)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
	__int64 i, j, k, x, y, z;

	ocId2ijk(ocId, lvl, i, j, k);
	memset(nei, 0, sizeof(__int64)*27);

	if(i==0){
		nei[0] = -1;  nei[3] = -1;  nei[6] = -1;
		nei[9] = -1;  nei[12] = -1; nei[15] = -1;
		nei[18] = -1; nei[21] = -1; nei[24] = -1;
	}
	if(j == 0){
		nei[0] = -1;  nei[1] = -1;  nei[2] = -1;
		nei[9] = -1;  nei[10] = -1; nei[11] = -1;
		nei[18] = -1; nei[19] = -1; nei[20] = -1;
	}
	if(k == 0){
		nei[0] = -1;  nei[1] = -1;  nei[2] = -1;
		nei[3] = -1;  nei[4] = -1;  nei[5] = -1;
		nei[6] = -1;  nei[7] = -1;  nei[8] = -1;
	}

	if(i== dim[lvl]-1){
		nei[2] = -1;  nei[5] = -1;  nei[8] = -1;
		nei[11] = -1; nei[14] = -1; nei[17] = -1;
		nei[20] = -1; nei[23] = -1; nei[26] = -1;
	}
	if(j == dim[lvl]-1){
		nei[6] = -1;  nei[7] = -1;  nei[8] = -1;
		nei[15] = -1; nei[16] = -1; nei[17] = -1;
		nei[24] = -1; nei[25] = -1; nei[26] = -1;
	}
	if(k == dim[lvl]-1){
		nei[18] = -1; nei[19] = -1; nei[20] = -1;
		nei[21] = -1; nei[22] = -1; nei[23] = -1;
		nei[24] = -1; nei[25] = -1; nei[26] = -1;
	}



  	for( __int64 ii=0; ii<27; ii++){
		if( nei[ii]!= -1){
			x = (ii%3) -1;
			y = (ii/3)%3 -1;
			z = (ii/9)%3 -1;
			nei[ii]= ijk2OcId(i+x, j+y, k+z ,lvl);
		}
	}

}


void OcTree::printOtInfo(vtxArr* vtx){
    char* fn= NULL;
 	getFullFilePath(&fn, "ot", ".info");
 	FILE * file = fopen(fn, "w");
 	intArr* bnd=NULL;
 	fprintf(file, "number of cells is: [%d]\n", cells->length);

 	for(__int64 i=0; i<cells->length; i++){

 
 		char f[500];
 		intArr* fc = cells->getFc(i);
 		memset(f, '\0', sizeof(char)*500);
 		 __int64 len;
 		if(!cells->getFc(i))
 			len = 0;
 		else
 			len = (cells->getFc(i))->length;

 		char tt[10];
 		for( __int64 j=0;j<len; j++){
 			sprintf(tt, "%lld, ",(*fc)[j]);
 			strcat(f, tt);
 		}

 		if(strlen(f)<30)
 			for( __int64 j= strlen(f)-1; j<30; j++)
 				strcat(f, " ");

 		 __int64 nd;
 		if(cells->getNd(i) )
 			nd = (*cells->getNd(i))[0];
 		else
 			nd = -1;
 		if(cells->getVtx(i) != -1)
			bnd = cells->getBnd(i);
 		else
 			bnd = NULL;



 		fprintf(file, "[%lld]: ocId= %lld,\t\tfc= %s,\t\tnd= %lld,\t\t zId= %lld, \t vtxId= %lld, \t bnd=  ",
 				i, cells->getOcId(i), f, nd, cells->getZone(i), cells->getVtx(i));

        if(bnd){
			for(__int64 j=0; j<bnd->length; j++){
				fprintf(file, "%d ", (*bnd)[j] );
			}
 		}
 		else {
 			fprintf(file, "%d ", -1 );
 		}

 		fprintf(file, "\n");


 	}
 	fclose(file);
 	free(fn);


}


void OcTree::printTree(__int64 rId){
	Tree* t;
	t = &tree[rId];
	__int64 cId = t->r;
	ocCellArr::ocCell *cur = cells->getCell(cId);
	cout<<"grid cell of "<<cur->ocId<<endl;
	cout<<"bnd= "<<cur->bnd<<endl;
//	cout<<"coll= "<<cur->coll<<endl;
	if(cur->fc){
		cout<<"fc_length= "<<cur->fc->length<<endl;
		cout<<"face are: "<<endl;
		for( __int64 i=0; i<cur->fc->length; i++){
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
	getBaseCellNodeCoord(cur->ocId, base);
	size = getSizeOfCellAtLvl(getLvlOfOcId(cur->ocId));
	cout<<"Base: "<<base[0]<<", "<<base[1]<<", "<<base[2]<<endl;
	cout<<"Size: "<<size<<endl;

	cout<<"Vtx length: "<<vtx->length<<endl;

	cout<<endl;

	for( __int64 i=0; i<8; i++)
		printTreeHelper(&(t->cld[i]));
}
void OcTree::printTreeHelper(Tree * t){


    __int64 cId = t->r;
	ocCellArr::ocCell *cur = cells->getCell(cId);
	cout<<"grid cell of "<<cur->ocId<<endl;
	cout<<"bnd= "<<cur->bnd<<endl;
//	cout<<"coll= "<<cur->coll<<endl;
	if(cur->fc){
		cout<<"fc_length= "<<cur->fc->length<<endl;
		cout<<"face are: "<<endl;
		for( __int64 i=0; i<cur->fc->length; i++){
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
	getBaseCellNodeCoord(cur->ocId, base);
	size = getSizeOfCellAtLvl(getLvlOfOcId(cur->ocId));
	cout<<"Base: "<<base[0]<<", "<<base[1]<<", "<<base[2]<<endl;
	cout<<"Size: "<<size<<endl;

	cout<<"Vtx length: "<<vtx->length<<endl;

	cout<<endl;

	for( __int64 i=0; i<8; i++)
		printTreeHelper(&(t->cld[i]));
}


void OcTree::incrementBotLvl(){
	
//	cout<<"increment botLvl "<<botLvl<<"=>>"<<botLvl+1<<endl;

	botLvl++;
    numCellAtLvl 	= (__int64 *)   realloc(numCellAtLvl, 	sizeof(__int64 )    * (botLvl+1));
    startOcIdAtLvl 	= (__int64  *)	realloc(startOcIdAtLvl, sizeof(__int64 )    * (botLvl+1));
    cellSize     	= (double*) 	realloc(cellSize, 		sizeof(double) 		* (botLvl+1));
    dim          	= ( __int64  *)    	realloc(dim, 			sizeof( __int64 )    	* (botLvl+1));

    for( __int64 i = 0; i <= botLvl; i++){
    	numCellAtLvl[i] = (__int64 )1<<(i*3);
    	cellSize[i]     = span/(1<<i);
    	dim[i]          = 1<<i;
    }

 //   cout<<"new min cell size: "<<cellSize[botLvl]<<endl;
	startOcIdAtLvl[0] = 0;
    for( __int64 i=1; i<=botLvl; i++){
    	startOcIdAtLvl[i] = numCellAtLvl[i-1] + startOcIdAtLvl[i-1];
    }

    totlNumCell = startOcIdAtLvl[botLvl]+numCellAtLvl[botLvl];



}

void OcTree::collapseOnlyThisCellForMultiVtxCellProc(__int64 ocId, __int64 cld[8]){
	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in collapseOnlyThisCell, id out of range: %lld\n"
			   "       expecting range: (0, %lldd)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
	__int64 lvl = getLvlOfOcId(ocId);

	Tree *curT = getTreeNode(ocId, lvl);
	if(curT==NULL || getColl(ocId, lvl) !=0 ){
		printf("Error: in collapseOnlyThisCell, ocId is not a leaf: %lld, can only create child for leaf node.\n",
			           ocId);
		throw StoppingException("\n");
	}

	getChildOcId(ocId, cld, lvl);

//	setColl(ocId, lvl, 1);

	curT->cld = (Tree*)malloc(sizeof( Tree ) * 8);
	__int64 prtCId = curT->r;

	intArr* pnd = getNd(ocId, lvl);

	intArr* pfc = getFc(ocId, lvl);
	__int64 curCId;
	
	//curT->r = -1;
	setColled(ocId, lvl);
	for(__int64 j=7; j>-1; j--){

		if(j==0){
			curCId = prtCId;
		}
		else {
			cells->add();
			curCId = cells->length -1;
		}

		curT->cld[j].cld = NULL;
		curT->cld[j].r = curCId;
//		cells->setColl(cells->length-1, 0);
		cells->setOcId(curCId, cld[j]);
		findFcNdInGrdCell(cld[j], lvl+1, pnd, pfc);
		cells->setZone(curCId, -2);//since all the sub cells will be assigned a vtx, then should all be -2
	}





}


__int64 OcTree::getNeighOcId(__int64 ocId, __int64 nei[3]){

	if(ocId<0 || ocId >= totlNumCell){
		printf("Error: in getNeighOcId, id out of range: %lld\n"
			   "       expecting range: (0, %lldd)\n",
			           ocId, totlNumCell);
		throw StoppingException("\n");
	}
	if(nei[0] > 1 || nei[0]< -1 || nei[1]>1 || nei[1]<-1 || nei[2]>1 || nei[2]<-1 ){
		printf("Error: in getNeighOcId, nei distance out of range: %lld\n"
			   "       expecting range: (-1, 0 , 1)\n",
			           ocId);
		throw StoppingException("\n");
	}
	__int64 i, j, k, lvl;
	lvl = getLvlOfOcId(ocId);
	ocId2ijk(ocId, lvl, i, j, k);
	i += nei[0];
	j += nei[1];
	k += nei[2];
	return ijk2OcId(i, j, k, lvl);



}




