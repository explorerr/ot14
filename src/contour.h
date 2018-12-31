/*
 * contour.h
 *
 *  Created on: May 31, 2010
 *      Author: zrui
 */

#ifndef CONTOUR_H_
#define CONTOUR_H_

#include "geoModel.h"
#include "Exceptions.h"
#include "vtxArr.h"
#include "elemArr.h"
#include "faceArr.h"
#include "OcTree.h"
#include "OcCellArr.h"
#include "OcTreeIterator.h"
#include "ocCellArr.h"
#include "geoNodeArr.h"





class Contour {
public:
	Contour(geoModel* GM, OcTree *ot, vtxArr* v, elemArr* e, faceArr* f, char* fName, bool binary);
	virtual ~Contour();

	void doContour();

private:

	void getVtx();
    void findVtx41Face(ocCellArr::ocCell *cur);
    void findVtx41FaceHelper(__int64 ocId, __int64 fc, double v[3], __int64& bnd);
    void findVtx42Face(ocCellArr::ocCell *cur);
    bool findVtx42FaceHelper(__int64 ocId, __int64 f1, __int64 f2, geoNodeArr& v);
    void findVtx4MultiFace(ocCellArr::ocCell *cur);
    void findVtx4MultiFaceHelper(ocCellArr::ocCell *cur, geoNodeArr &intPnt);
    void getVxtZoneId();
    void addVtx(double* x, __int64 ocId, intArr* bnd);
	void addVtx(double* x, __int64 ocId, __int64 cellId);
	void addVtx(double* x, ocCellArr::ocCell *cur);


    void getElements();
    void generateElemForOneNode(__int64 ocId);
    bool checkAndUpdateAllVtxOnBndButOnDiffentBndElem(__int64 cell[8]);
    bool getVtxHelper(__int64 ocId, double v[3]);

    void processTwoDisconFaceCells();
    bool moveVtxHelper_two(__int64 ocId, __int64 f[2], double v[2][3]);
    void checkUpdateSnappedElement(__int64 cldOcId[8], geoNodeArr *v, pointerArr *vFace);

    bool borrowVtxHelper(__int64 ocId, intArr& f, double v[2][3]);
	bool findMoveNeighbor(__int64 ocId, __int64 f[2], double v[2][3], double size, __int64 &neiVId );

    void processMultiDisconFaceCells();

    bool moveVtxHelper_multi(__int64 ocId, intArrSorted f[2], double v[2][3]);
    __int64 findNeighOcIdMidLine(__int64 ocId, __int64 f[2], double v[2][3]);
    __int64 findNeighOcIdOppositeToFace(__int64 ocId, __int64 f, double v[2][3]);
    __int64 findNeighOcIdWithDirection(__int64 ocId, double n[3]);

    void findVtices4MultiVtxCell(ocCellArr::ocCell *cur);

    void processMultiVticesCells();
    void checkBndSignWithElem(int subCellId, __int64 prtOcId, geoNodeArr &v, pointerArr &vFace, intArr &choice);

    void sortMultiVticesCellWithFc();

    void getSubCellVtxMovInfo(int subCellId, int vId, __int64 prtOcId, geoNodeArr v[8], pointerArr vFace[8],
    		                              int movDir[3], intArr &movTogether, bool &movable, __int64 subOcId[8]);

	void vtxToFile();


	void processMultiVticesCells_2();
	void findVtxForMultiVticesCell(__int64 ocId, geoNodeArr& v, pointerArr & vFace);
	void getRemainVtx(__int64 ocId, __int64 cldPos, geoNodeArr & v, __int64 &remainVId);



	FILE* meshFile;

    geoModel  *GM;
    OcTree    *ot;

    vtxArr    *vtx;
    elemArr   *elem;
    faceArr   *face;

	bool binary;

	char *fName;

	__int64 numElem;
	__int64 numVtx;

    __int64 elemFaces[6][4];

    intArr *twoDisconFaceCells;

    intArr *multiDisconFaceCells;

    intArr *multiVtxCells; // data structure to hold all the twoVtxCells






};

#endif /* CONTOUR_H_ */
