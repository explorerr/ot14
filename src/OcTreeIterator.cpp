/*
 * OcTreeIterator.cpp
 *
 *  Created on: Jun 4, 2010
 *      Author: zrui
 */
#include "stdafx.h"
#include "OcTreeIterator.h"
#include <math.h>

OcTreeIterator::OcTreeIterator(OcTree *ot) {


	this->ot = ot;

	queue = NULL;
	qCap = 0;

	qCap += (unsigned __int64)1<<((ot->botLvl)*3);
	rtCap = (__int64) ot->numCellAtLvl[ot->topLvl] -1;
//	qCap = (__int64) (pow(8, ot->botLvl+1) - 8)/7 + 1;
	queue = (__int64*)malloc(sizeof(__int64)*qCap);
	if(!queue)
		throw StoppingException("Error in OcTreeIterator, queue initialization failed.\n");
//	memset(queue, 0, sizeof(__int64)*qCap );
	cout<<"qCap is: "<<qCap<<endl;
	cout<<"trying to load iterator"<<endl;

	curTId = -1;
	if(! loadNxtTopLvlTreeLeafIntoQueue()){
		free(queue);
		curOcId = 0;
		curLvl = -1;
		curCell = NULL;
		qs = 0;
		qe = -1;
	}

	curOcId = ot->cells->getOcId(queue[0]);
	curLvl = ot->getLvlOfOcId(curOcId);
	curCell = ot->cells->getCell(queue[0]);

	cout<<"iterator loaded"<<endl;

}

OcTreeIterator::~OcTreeIterator() {

	if(queue)
		free(queue);
}

ocCellArr::ocCell * OcTreeIterator::getNextLeafCell(){

	if(qs<qe){
		curOcId = ot->cells->getOcId(queue[++qs]);
		curLvl = ot->getLvlOfOcId(curOcId);
		curCell = ot->cells->getCell(queue[qs]);
		return curCell;
	}


	if(curTId >= rtCap){ //no more tree roots
		curOcId = 0;
		curLvl = -1;
		curCell = NULL;
		return NULL;
	}


	if( loadNxtTopLvlTreeLeafIntoQueue() ){

		curOcId = ot->cells->getOcId(queue[qs]);
		curLvl = ot->getLvlOfOcId(curOcId);
		curCell = ot->cells->getCell(queue[qs]);
		return curCell;

	}
	else
		return NULL;
}



bool OcTreeIterator::hasNextLeafCell(){

	if(qs<=qe){ //the current queue is not empty
		return true;
	}
	else if(curTId >= rtCap){ //the queue is empty, and there is no more tree to load
		return false;
	}

	else if( curTId < rtCap){ // the queue is empty, but there are more trees to look for

		for( __int64 i= curTId; i<ot->numCellAtLvl[ot->topLvl]; i++){
			if( treeHasLeaf(ot->tree[i]))
				return true;
		}
		return false;
	}
	return false;


}

bool OcTreeIterator::treeHasLeaf(Tree &t){

	if( t.r != -1 )
		return true;
	else
		return false;


}

/**
 * This function will load the ocId of all the leaf node under the root of the tree
 */
bool OcTreeIterator::loadNxtTopLvlTreeLeafIntoQueue(){

//	memset(queue, 0, sizeof(__int64)*qCap );

	qs = 0; qe = -1;

	if(curTId == rtCap)
		return false;

	curTId++;

	Tree* t = & ot->tree[curTId];


	while( t->r == -1 ){// the root is empty
		cout<<"the tree: ["<<curTId<<"] is empty"<<endl;
		t = & ot->tree[++curTId];

		if( curTId >= rtCap)
			return false;
	}

	if((__int64)t->r ==-1){//a parent
		qe++;
		queue[qe] = t->r;
	}

	if(t->cld){
		for( __int64 i=0; i<8; i++)
			loadTreeLeafIntoQueueHelper( t->cld[i], ot->topLvl+1 );
	}
	if(qs>qe)
		return loadNxtTopLvlTreeLeafIntoQueue();
	cout<<"loading tree root: "<<curTId<<" qs="<<qs<<" qe="<<qe<<endl;
//	for( __int64 i=0; i<qe; i++)
//		cout<<" ocId="<<ot->cells->getOcId(queue[i])<<endl;
	return true;

}


void OcTreeIterator::loadTreeLeafIntoQueueHelper(Tree &t,  __int64 lvl){
	if(!(&t)){
		printf("Error: in loadTreeLeafIntoQueue in OcTreeIterator, tree is NULL.\n");
		throw StoppingException("\n");
	}

	if((__int64)t.r ==-1) // a parent
		queue[++qe] = t.r;

	if(t.cld){
		for( __int64 i=0; i<8; i++)
			loadTreeLeafIntoQueueHelper( t.cld[i], lvl+1 );
	}

}



