/*
 * OcTree.h
 *
 *  Created on: Apr 20, 2009
 *      Author: zrui
 */

#ifndef OCTREE_H_
#define OCTREE_H_

#include <memory>
#include <stack>
#include "geoModel.h"
#include "Exceptions.h"
#include "intArr.h"
#include "vtxArr.h"
#include "elemArr.h"
#include "faceArr.h"
#include "Polygon.h"
#include "ocCellArr.h"
#include "intArrSortedWithCnt.h"

#include <basetsd.h>/* required typedef __int64 for gcc */




typedef struct Tree{
	__int64 r;  //cell id of the root of the tree
	Tree *cld;  //its child
}Tree;



class OcTree {

public:

	OcTree(geoModel& GM, double userElementSize[2],  __int64 maxLevelDifference, int domMode, int extMode, bool uniform);
	virtual ~OcTree();



	bool findFcNdInGrdCell(__int64 ocId,  __int64 lvl, intArr* pnd, intArr* pfc);


    void OTConstruct();

    void printOtInfo(vtxArr* vtx);


    void moveGrid(double x[3]);

    //utilities

    intArr* 	getNd  (__int64 ocId,  __int64 lvl);
    intArr* 	getFc  (__int64 ocId,  __int64 lvl);
    __int64     getZone(__int64 ocId,  __int64 lvl);
    __int64     getColl(__int64 ocId,  __int64 lvl);
    intArr*     getBnd (__int64 ocId,  __int64 lvl);
    __int64     getVtx (__int64 ocId,  __int64 lvl);
    intArr* 	getNd  (__int64 ocId);
    intArr* 	getFc  (__int64 ocId);
    __int64     getZone(__int64 ocId);
    __int64     getColl(__int64 ocId);
    intArr*     getBnd (__int64 ocId);
    __int64     getVtx (__int64 ocId);
    void    	setNd  (__int64 ocId,  __int64 lvl, intArr* nd);
    void    	setFc  (__int64 ocId,  __int64 lvl, intArr* fc);
    void    	setZone(__int64 ocId,  __int64 lvl,  __int64 z);
    void    	setBnd (__int64 ocId,  __int64 lvl,  intArr* b);
    void    	setVtx (__int64 ocId,  __int64 lvl, __int64 vtx);
    void    	setNd  (__int64 ocId, intArr* nd);
    void    	setFc  (__int64 ocId, intArr* fc);
    void    	setZone(__int64 ocId,  __int64 z);
    void    	setBnd (__int64 ocId,  intArr* b);
    void    	setVtx (__int64 ocId, __int64 vtx);
    __int64 	getCellId(__int64 ocId);
    void    	setColled(__int64 ocId,  __int64 lvl);
    void    	setColled(__int64 ocId);

   	void 		ocId2ijk(__int64 ocId,  __int64 &i,  __int64 &j,  __int64 &k);
	void 		ocId2ijk(__int64 ocId,  __int64 myLvl,  __int64 &i,  __int64 &j,  __int64 &k);
	__int64 	ijk2OcId( __int64 i,  __int64 j,  __int64 k,  __int64 lvl);

	__int64 	getLeafStartOcId();
    __int64 	getLvlOfOcId(__int64 ocId);
    double 		getSizeOfCellAtLvl( __int64 lvl);

 

    void 		getFullCellNodeCoord(__int64 ocId, double x[8][3]);
    void 		getBaseCellNodeCoord(__int64 ocId, double x[3]);
    void 		getFullCellNodeCoord(__int64 ocId,  __int64 lvl, double x[8][3]);
    void 		getBaseCellNodeCoord(__int64 ocId,  __int64 lvl, double x[3]);

    bool 		getNeighLeafCellVtx(__int64 ocId, __int64 cell[8]);
    void 		getNeighOcId4Elem(__int64 ocId, __int64 nei[8],  __int64 lvl);

	void 		getChildOcId(__int64 ocId, __int64* childId,  __int64 lvl);
    __int64		getChildLeafId(__int64 ocId,  __int64 i,  __int64 lvl);


	void 		printTree(__int64 rId);
	void 		printTreeHelper(Tree *t);


	__int64     getNeighOcId(__int64 ocId, __int64 nei[3]);

    friend class OcTreeIterator;


	friend class Contour;

	

private:



	void 		getLevelLimits();
    void 		modelOnGridCheck();
    void 		collapseTree();
    void 		getOtCellZoneId();
    __int64 	getCellId(__int64 ocId,  __int64 lvl);

    void 		getChildOcId(__int64 ocId, __int64& childId,  __int64 idx,  __int64 Lvl);
    __int64 	getParentId(__int64 ocId);
    __int64 	getParentId(__int64 ocId,  __int64 lvl);
    void 		getParentAndPosition(__int64 ocId,  __int64 lvl, __int64 &parent,  __int64 &position);

    Tree * 		getTreeNode(__int64 ocId,  __int64 lvl);
    void 		createChildForTheOcNode(__int64 ocId,  __int64 lvl, __int64 cellId);
	void        createChildForTheOcNodeUniform(__int64 ocId, __int64 lvl, __int64 cellId);

     __int64 	getZoneIdForOneCell(__int64 ocId,  __int64 lvl);
    void 		getOtCellZoneIdHelper(Tree* root, __int64 prtOcId,  __int64 lvl,  __int64 prtZ);



    void 		getFullNeighOcId(__int64 ocId, __int64 nei[27],  __int64 lvl);
    bool 		findFcNdInGrdCell(__int64 ocId,  __int64 lvl);


    bool 		hasFeatureInGrdCell(__int64 ocId,  __int64 lvl);
    void 		checkAndUpdateNeiCollWithDiffLimit(__int64 ocId,  __int64 lvl);
    bool        featureCheck4CellsBelowMidLvl(__int64 ocId, __int64 lvl);

    void        collapseFullNeigh(__int64 ocId, __int64 lvl);

    void 		deleteTree(Tree *t);

    // this two function is dangerous, design only for processing multiVtices cells
    // be careful
    void        incrementBotLvl();
    void        collapseOnlyThisCellForMultiVtxCellProc(__int64 ocId, __int64 cld[8]);

    void        collapseFeatureCellFromStrToMidLvl();
    void        collapseFeatureCellBeneathMidLvl();
    void        cellNeighUpdate();

	__int64 OcTree::getTreeNodeId(__int64 ocId, __int64 lvl);

    geoModel* GM;

	int extMode;
	int domMode;
	bool uniform;

	double    span; // span of octree grid
	double    base[3]; // the coordinates of the orig of the ocTree

	double    *cellSize; // array to hold the cell sizes at each level
	__int64   *dim; // array to hold num of cells in one axis direction at each level
	__int64   *numCellAtLvl; // array to hold num of cells at each level
	__int64   *startOcIdAtLvl; // array to hold num of cells at each level
	__int64   totlNumCell;


	double    otCellSizeMin; // the minimum cell size of the octree
	double    otCellSizeMax; // the maximum cell size of the octree
	double    otCellSizeMid; // the midium  cell size of the octree
	double    maxDiff;

	__int64   strLvl; // the starting level where starts to collapse
	__int64	  topLvl; // the top discretization level of the octree
	__int64   botLvl; // the bottom discretization level of the octree
	__int64   midLvl; // the mid lvl used to collapse cells that is on the face or edges but not on the node


	Tree      *tree; // the data structure to store the tree

    vtxArr    *vtx;  // the data structure to store all the vtx
    elemArr   *elem; // the data structure to store all the elem
    faceArr   *face; // the data structure to store all the faces
	ocCellArr *cells; //the data structure to store all the information of a octree cell

     __int64  elemFaces[6][4];

    double 	  offset[3];

	FILE* addFile;

	intArr *pCurFeatureCells, *pNxtFeatureCells; // there will be two list storing the ocId for the curLvl and nxt Lvl;
	intArr *pCurCellNeedFullNeighColl;
	intArr *pNxtCellNeedFullNeighColl;
	intArr *pCurCellNeedNeighCheck;
	intArr *pNxtCellNeedNeighCheck;

};

#endif /* OCTREE_H_ */
