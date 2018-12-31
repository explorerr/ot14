/*
 * OcTreeIterator.h
 *
 *  Created on: Jun 4, 2010
 *      Author: zrui
 */

#ifndef OCTREEITERATOR_H_
#define OCTREEITERATOR_H_

#include "OcTree.h"
#include "ocCellArr.h"

class OcTreeIterator {
public:
	OcTreeIterator(OcTree* ot);
	virtual ~OcTreeIterator();

    ocCellArr::ocCell * curCell;

    __int64 curOcId;
     __int64 curLvl;




    ocCellArr::ocCell * getNextLeafCell();
    bool hasNextLeafCell();


private:
    OcTree * ot;
    __int64* queue; // store the cell id 
    __int64 qLen;
    __int64 qCap;
    __int64 qs, qe;
    __int64 curTId;
    __int64 rtCap;

    bool loadNxtTopLvlTreeLeafIntoQueue();
    void loadTreeLeafIntoQueueHelper(Tree &t,  __int64 lvl);
    bool treeHasLeaf(Tree &i);
};

#endif /* OCTREEITERATOR_H_ */
