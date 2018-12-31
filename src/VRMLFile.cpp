/*
 * VRMLFile.cpp
 *
 *  Created on: Jul 9, 2010
 *      Author: zrui
 */
#include "stdafx.h"
#include "VRMLFile.h"
#include <stack>
#include <algorithm>

VRMLFile::VRMLFile( char* fName, geoNodeArr* node, geoFaceArr* face, stringArrNonDup* bndStr ) {

	initialize();

	this->nodes = node;
	this->faces = face;
	this->bndStr = bndStr;
    
	file = new myFile(fName, ".wrl");

	offset[0] = offset[1] = offset[2] = 0;
	scale[0] = scale[1] = scale[2] = 1;
	unitConvt = 0.0254;

}

VRMLFile::~VRMLFile() {
	delete file;
	for(int i=0; i<pat.length; i++)
		delete (Patch*)pat[i];


}


void VRMLFile::loadFile(){

	int    bCnt = 0;
	stack<string> prt;
	char*  token;

	string pre;
	file->tokenize(" ,");

	while (token = file->curToken){

		if( token[0] == '#' ) {
			file->getNxtLineAndTokenize();
			continue;
		}
		else if( strcmp(token, "{") == 0){
			bCnt++;
			prt.push(pre);
//			cout<<"prt stack pushing: "<<prt.top()<<endl;

			if(prt.size()==0){
				printf("Error in VRMLFile::loadFile: file format error, node w/o type: %s", file->curLine);
				throw StoppingException("from VRMLFile::loadFile\n");
			}
			if( prt.top().compare("Shape") == 0 ){
				try{
					readShapeNode();

				}
				catch(...){
					throw StoppingException("From VRMLFile::loadFile Error in readShapeNode.\n");
				}
				continue;
			}
			else if( prt.top().compare("Transform") == 0){
				try{
					readTransformNode();
				}
				catch(...){
					throw StoppingException("From VRMLFile::loadFile Error in readShapeNode.\n");
				}
				continue;

			}
		}
		else if( strcmp(token, "}") == 0 ){

			if( prt.top().compare("Transform") == 0 ){

				scale[0] = scale[1] = scale[2] = 1;
				offset[0] = offset[1] = offset[2] = 0;
			}
		//	cout<<"prt stack poping: "<<prt.top()<<endl;
			prt.pop();
			bCnt--;
		}
		else { // before a new brace

			pre.assign(token);

		}
		file->getNxtToken();
	}


	addPolyPatchToFaceArr();

}

void VRMLFile::readTransformNode(){

	char* token;
	int cnt=0;
	double temp;
	bool flg[2] = {false, false};
	token = file->curToken;

	while ( token ){

		if( token[0] == '#' ) {
			file->getNxtLineAndTokenize();
			token = file->curToken;
			continue;
		}
		else if( strcmp(token, "children") == 0 ){
			return;
		}

		else if( strcmp(token, "translation") == 0 ){
			token = file->getNxtToken();

			for(int i=0; i<3; i++){
				if( from_string<double>(temp, token, std::dec) ){

					offset[cnt++] = temp*unitConvt;
					token = file->getNxtToken();

				}
			}
			flg[0] = true;
			continue;
		}
		else if( strcmp(token, "scale") == 0 ){

			token = file->getNxtToken();

			cnt=0;
			for(int i=0; i<3; i++){
				if( from_string<double>(temp, token, std::dec) ){

					scale[cnt++] = temp;
					token = file->getNxtToken();

				}
			}
			flg[1] = true;
			continue;


		}

		else if( strcmp(token, "rotation") == 0 ){

			token = file->getNxtToken();

			cnt=0;

			double tempRo[4];
			for(int i=0; i<4; i++){
				if( from_string<double>(temp, token, std::dec) ){

					scale[cnt++] = temp;
					token = file->getNxtToken();

				}
			}

			if(cnt!=4){
				printf("Error reading roation node in VRML file, expecting 4 values for the ration value, but got %d values.\n", cnt);
				throw StoppingException("\n");
			}

			flg[1] = true;
			continue;


		}
		else if(flg[0] && flg[1]){
//			cout<<"got translation: ("<<offset[0]<<", "<<offset[1]<<", "<<offset[2]<<")"<<endl;
//			cout<<"got scale:       ("<<scale[0] <<", "<<scale[1] <<", "<<scale[2] <<")"<<endl;
			return;
		}

		token = file->getNxtToken();

	}

}




void VRMLFile::readShapeNode(){

	int    bCnt = 0;
	char*  token;
	char  curBnd[1000];

//	cout<<"getting face: ["<<faces->length<<"]"<<endl;

	while( token = file->curToken ){

		if( token[0] == '#' ) {
			file->getNxtLineAndTokenize();
			continue;
		}
		else if( strcmp(token, "{") == 0){
			bCnt++;
		}
		else if( strcmp(token, "}") == 0){
			bCnt--;
			if(bCnt==0){
				return;
			}
		}
		else if( strcmp(token, "appearance") == 0 ){
			try{

				readAppearance(curBnd);

			}
			catch(...){
				throw StoppingException("From VRMLFeil::readShapeNode.\n");
			}
		}
		else if( strcmp(token, "IndexedFaceSet") == 0 ){
			try{

				readIndexedFaceSet(curBnd);

			}
			catch (...){
				throw StoppingException("From VRMLFeil::readShapeNode.\n");
			}
		}
		else if( strcmp(token, "Sphere") == 0 ){
			try{

				readSphere(curBnd);

			}
			catch (...){
				throw StoppingException("From VRMLFeil::readShapeNode.\n");
			}
		}
		file->getNxtToken();

	}

}

void VRMLFile::readAppearance(char* curBnd){


	char* token;
	curBnd[0] ='\0' ;
	token = file->curToken;
	while( file->curToken && curBnd[0]=='\0' ){

		if( strcmp(token,"DEF")==0 || strcmp(token,"USE")==0 ){
			token = file->getNxtToken();
			int offset=0;
			if(strncmp(token, "COL_", 4)==0 )
				offset = 4;

			strcpy(curBnd, token+sizeof(char)*offset);
			break;
		}
		token =  file->getNxtToken();
	}

}

void VRMLFile::readSphere(char* curBnd){

	double rr = -1;
	int bCnt=0;
	char* token;

	while(file->curToken ){
		
		token = file->curToken;

		if( token[0] == '#' ) {
			file->getNxtLineAndTokenize();
			continue;
		}		
		else if( strcmp(token, "{") == 0){
			bCnt++;
		}
		else if( strcmp(token, "}") == 0){
			bCnt--;
			if(bCnt==0){
				break;
			}
		}
		else if(strcmp(token, "radius") == 0){
			token = file->getNxtToken();
			try{
				from_string<double>(rr, token, std::dec);
				rr = rr*unitConvt;
			}
			catch (...){
				throw StoppingException("From VRMLFile::readSphere.\n");
			}
		}
		token =  file->getNxtToken();
	}

	vector<double> info;

	info.push_back(offset[0]);
	info.push_back(offset[1]);
	info.push_back(offset[2]);
	info.push_back(offset[0]+rr);
	info.push_back(offset[1]);
	info.push_back(offset[2]);
	info.push_back(0);  //zone+
	info.push_back(-1); //zone-
	//need to figure out the bnd number, which is the id of the bndStr in the bndStr Array
	int bndId = bndStr->add(curBnd); // deep copy
	info.push_back(bndId);
	// put in the zDefTag
	// if the bndStr starts with "int_" then it is not a zoneDefFace, else it is
	if( strncmp(curBnd, "int_", 4)==0 )
		info.push_back(0);
	else
		info.push_back(1);

	Face* f= new Face(0x8, info);

	faces->add(f);



	






}


void VRMLFile::readIndexedFaceSet( char* curBndStr){

	vector<double>	coord;
	vector<int>		id;
	
	int		bCnt=0;
	char*	token;

	vector<int> vArrId;

	convex = true;

	while(file->curToken && (id.size()==0 || coord.size()==0 ) ){

		token = file->curToken;

		if( token[0] == '#' ) {
			file->getNxtLineAndTokenize();
			continue;
		}
		else if( strcmp(token, "{") == 0){
			bCnt++;
		}
		else if( strcmp(token, "}") == 0){
			bCnt--;
			if(bCnt==0){
				break;
			}
		}
		else if(strcmp(token, "Coordinate") == 0){
			try{
				readCoord(coord);
			}
			catch (...){
				throw StoppingException("From VRMLFile::readIndexedFaceSet.\n");
			}
		}

		else if( strcmp(token, "coordIndex") == 0 ){
			try{
				readCoordIndex(id);
			}
			catch (...){
				throw StoppingException("From VRMLFile::readIndexedFaceSet.\n");
			}
		}
		else if( strcmp(token, "convex") == 0 ){

			token = file->getNxtToken();

			if( strcmp(token, "FALSE") == 0 ) {

				convex = false;
			}


		}

		file->getNxtToken();
	}



	//check if the points are colinar, may only happen in triangles generated by sketchUp
	if(coord.size()==9 && triangleCheckColiner(coord)){
		return;
	}
/*	if(coord.size() == 9){
		
		double cross[3];
		double l[3], ll[3];
			
			l[0]  = coord[3]-coord[0];
			l[1]  = coord[4]-coord[1];
			l[2]  = coord[5]-coord[2];

			ll[0] = coord[6]-coord[3];
			ll[1] = coord[7]-coord[4];
			ll[2] = coord[8]-coord[5];
			Cross(l, ll, cross);
			if( fabs(cross[0]) + fabs(cross[1]) + fabs(cross[2]) < 0.00001 ) // colinar
				return;


	}*/

	//put the data into the geoModel arrays
	if(!convex){//non convex


		std::vector<Triangle> tris;
		std::vector<double> vertices;

		Tessellator tess;
		vector<double> temp;
		tess.Tessellate(coord, temp, tris, vertices);
		
		
		vector<int> tempId;
		tempId.push_back(0);
		tempId.push_back(1);
		tempId.push_back(2);
		tempId.push_back(0);
		for(
		std::vector<Triangle>::const_iterator triangle = tris.begin();
		triangle != tris.end();
		++triangle
		) {		
			vector<double> tempCoord;
			int vv[3];
			double tt[3];
			vv[0] = triangle->FirstVertexIndex;
			vv[1] = triangle->SecondVertexIndex;
			vv[2] = triangle->ThirdVertexIndex;
			for(int i=0; i<3; i++){
				tt[0] = vertices[vv[i]*3];
				tt[1] = vertices[vv[i]*3+1];
				tt[2] = vertices[vv[i]*3+2];
				tempCoord.push_back( tt[0] );
				tempCoord.push_back( tt[1] );
				tempCoord.push_back( tt[2] );
				int nn ;
				nn = nodes->addNonDup( tt ) ;
				vArrId.push_back( nn );

			}
		
			if ( checkFaceDup(vArrId) || vArrId.size()<3){
    			return;
			}
			
			if(tempCoord.size()==9 && triangleCheckColiner(tempCoord)){
				vArrId.clear(); continue;
			}
			addToFaceArr(tempId, tempCoord, vArrId, curBndStr);
			vArrId.clear();
		}
	}

	else{ //convex
		
		double tt[3];
		for(int i=0; i< (int)coord.size()/3; i++){
			tt[0] = coord[i*3];
			tt[1] = coord[i*3+1];
			tt[2] = coord[i*3+2];
			int nn ;
			nn = nodes->addNonDup( tt ) ;
			vArrId.push_back( nn );
		}
		if ( checkFaceDup(vArrId) ){
    		return;
		}

		if(coord.size()==9 && triangleCheckColiner(coord)){
			return;
		}
		addToFaceArr(id, coord, vArrId, curBndStr);

	}
}




void VRMLFile::addToFaceArr(vector<int> id, vector<double> coord, vector<int> &vId, char* curBndStr){

	vector<double> info;
	int type;
	int idCnt = (int) id.size();
	if( idCnt == 1){
		printf("In addToFaceArr: find face with one node, will ignor.\n");
		return;
	}
	if( idCnt == 2){
		printf("In addToFaceArr: find face with two node, will ignor.\n");
		return;
	}
	if( idCnt >2 ){
		if(idCnt == 4)
			type = 3; // put in the type : 3 Triangular
		else if(idCnt == 5)
			type = 4;
		else {
			type = 5;
			info.push_back(idCnt-1); //put in number of vertices
		}


		//this program will make all the face facing up, normal vector with positive value in z direction

		double n[4];
		polyPlaneEquation(coord, idCnt-1, n);
		if(n[2]<0){
			for(int i=0; i<idCnt-1; i++){ // put in the coordinates of the nodes
				int cur = id[idCnt-2-i];
				if(cur<0){
					printf("Error: coordIndex is minus.\n");
					throw StoppingException("From VRMLFile::readIndexedFaceSet.\n");
				}
				info.push_back(coord[cur*3   ]);
				info.push_back(coord[cur*3 +1]);
				info.push_back(coord[cur*3 +2]);
			}

		}

		for(int i=0; i<idCnt-1; i++){ // put in the coordinates of the nodes
			int cur = id[i];
			if(cur<0){
				printf("Error: coordIndex is minus.\n");
				throw StoppingException("From VRMLFile::readIndexedFaceSet.\n");
			}
			info.push_back(coord[cur*3   ]);
			info.push_back(coord[cur*3 +1]);
			info.push_back(coord[cur*3 +2]);
		}
		// put in zone and bnd information
		// zone+ zone- bnd
		info.push_back(0);
		info.push_back(-1);

		//need to figure out the bnd number, which is the id of the bndStr in the bndStr Array
		int bndId = bndStr->add(curBndStr); // deep copy
		info.push_back(bndId);
		// put in the zDefTag
		// if the bndStr starts with "int_" then it is not a zoneDefFace, else it is
		if( strncmp(curBndStr, "int_", 4)==0 )
			info.push_back(0);
		else
			info.push_back(1);



		Face * curF = new Face(type, info);
		

		addToPolyArr(curF, vId);
	}
	info.clear();

}

/**
 * This function will read in the coordinates of all the nodes,
 * Add the node to the geoNodeArr,
 * and will return all the coordinates, that will be used to add
 * the face later on when the coordinate index has been read in.
 */


void VRMLFile::readCoord(vector<double>& coord){

	int    bCnt=0;
	bool   start = false;
	char*  token;


	while( coord.size()==0 && file->curToken ){ // find the first '['
		token = file->curToken;


		if( token[0] == '#' ) {
			file->getNxtLineAndTokenize();
			continue;
		}
		else if( strcmp(token, "{") == 0 ){
			bCnt ++;
			start = true;
		}

		else if( strcmp(token, "}") == 0 ){
			break;
		}

		else if(start){
			try{
				readDataBlockDouble(coord, '[', ']');
			}
			catch(...){
				throw StoppingException("From VRMLFile::readCoord\n");
			}
		}
		file->getNxtToken();
	}


	if(coord.size()%3 != 0){
		printf("Error in readCoord: total number of data read in is not a multiply of 3: %d",
				coord.size());

		throw StoppingException("\n");
	}

	int pntCnt = coord.size()/3;

	for(int i=0; i<pntCnt; i++){
		coord[i*3]   = (coord[i*3  ]*unitConvt+offset[0])*scale[0];
		coord[i*3+1] = (coord[i*3+1]*unitConvt+offset[1])*scale[1];
		coord[i*3+2] = (coord[i*3+2]*unitConvt+offset[2])*scale[2];
	}

}



void VRMLFile::readCoordIndex(vector<int> &cId){

	try{
		readDataBlockInt(cId, '[', ']');
	}
	catch(...){
		throw StoppingException("From VRMLFile::readCoordIndex.\n");
	}



}

void VRMLFile::readDataBlockDouble(vector<double> &data, char ds, char de){

	double temp;
	bool   start = false;
	char   dss[2] = {ds,'\0'};
	char   dee[2] = {de,'\0'};
	char*  token;


	while(file->curToken){

		token = file->curToken;

		if( token[0] == '#' ) {
			file->getNxtLineAndTokenize();
			continue;
		}

		else if( strcmp(token, dss) == 0 )
			start = true;

		else if( strcmp(token, dee) == 0 ){
			break;
		}

		else if(start){
			if( from_string<double>(temp, token, std::dec) ){
				data.push_back(temp);
			}
			else {
				printf("Error in readDataBlockDouble: can not parse to double: %s", token);
				throw StoppingException("From VRMLFile::readDataBlockDouble.\n");
			}
		}
		file->getNxtToken();
	}

}

void VRMLFile::readDataBlockInt(vector<int> &data, char ds, char de){

	int    temp;
	bool   start = false;
	char   dss[2] = {ds,'\0'};
	char   dee[2] = {de,'\0'};
	char*  token;


	while(file->curToken){

		token = file->curToken;

		if( token[0] == '#' ) {
			file->getNxtLineAndTokenize();
			continue;
		}
		else if( strcmp(token, dss) == 0 )
			start = true;
		else if( strcmp(token, dee) == 0 ){
			break;
		}
		else if(start){
			if( from_string<int>(temp, token, std::dec) ){
				data.push_back(temp);
			}
			else {
				printf("Error in readDataBlockInt: can not parse to int: %s", token);
				throw StoppingException("From VRMLFile::readDataBlockInt.\n");
			}
		}
		file->getNxtToken();
	}

}


void VRMLFile::initialize(){

//	curNode = NULL;
	shapeCnt = 0;

	strcpy((char*)geometryLookUpTbl[5],"Box");
	strcpy((char*)geometryLookUpTbl[6],"Cone");
	strcpy((char*)geometryLookUpTbl[7],"Cylinder");
	strcpy((char*)geometryLookUpTbl[3],"ElevationGrid");
	strcpy((char*)geometryLookUpTbl[4],"Extrusion");
	strcpy((char*)geometryLookUpTbl[0],"IndexedFaceSet");
	strcpy((char*)geometryLookUpTbl[1],"IndexedLineSet");
	strcpy((char*)geometryLookUpTbl[2],"PointSet");
	strcpy((char*)geometryLookUpTbl[8],"Sphere");
	strcpy((char*)geometryLookUpTbl[9],"Text");




}




bool VRMLFile::checkFaceDup(vector<int> &cdId){


    bool flg = false;

    vector<int> ff;//copy the ids and then sort in accending order
    ff = cdId;
    sort(ff.begin(), ff.end());
    int cdCnt = (int) cdId.size();

    for(int i=0; i<(int)fNIds.size(); i++){

        int cnt=0;
        for(int j=0; j<cdCnt; j++){
            if( ff[j] != fNIds[i][j] ){
                break;
            }
            cnt++;
        }
        if( cnt == cdCnt+1 )
        	flg = true;
    }

    if ( !flg )
        addToFNIds(ff);

    return flg;

}








void VRMLFile::addToPolyArr(Face*curF, vector<int>& cdId){

	int nv = cdId.size();
	vector<double> tt;
	
	Polygon* pp = (Polygon*)curF->f;
	//make sure the polygon is facing up
	if(pp->func[2]<0){
		for(int i=0; i<nv; i++)
			for(int j=0; j<3; j++)
				tt.push_back( pp->v[(nv-i-1)][j] );
	}
	else{
		for(int i=0; i<nv; i++)
			for(int j=0; j<3; j++)
				tt.push_back ( pp->v[i][j] );
	}

	Polygon* p = new Polygon(nv, tt);
	poly.add(p);
	addToPatches(poly.length-1, curF, cdId, nv);
	free(curF);


}


void VRMLFile::addToPatches(int fId, Face* fc, vector<int> nId, int numN){

	vector<int> pp;
	for(int i=0; i<pat.length; i++){


		if (  ((Patch*)pat[i])->attchedToPatchCheck(fc, nId, numN) ) {
			pp.push_back(i);
		}
	}

	if (pp.size() == 0){// not attached to any

		Patch* np = new Patch();
		pat.add(np);
		np->addFace(fId, nId, numN);
		np->setBnd(fc->bnd);
		np->setZDefFace(fc->zDefFace);
		Polygon* p = (Polygon*)fc->f;
		np->setPlaneEqua(p->func);
		return;
	}

	( (Patch*)pat[pp[0]] )->addFace(fId, nId, numN);

	for(int i=pp.size()-1; i>0; i--){
		stitchPatch( pp[0], pp[i]);
	}



}


/**
 * This function will merge ii and jj
 * and delete jj
 */
void VRMLFile::stitchPatch(int ii, int jj){


	Patch* p = (Patch*)pat[ii];
	Patch* q = (Patch*)pat[jj];

	intArrSorted* fl = q->getFaceList();
	intArrSorted* nd = q->getNodeList();

	for(int i=0; i<fl->length; i++){

		p->addToFC((*fl)[i] );

	}

	for(int i=0; i<nd->length; i++)
		p->addToND( (*nd)[i] );

	delete(q);

	pat.deleteData(jj);

}


void VRMLFile::addPolyPatchToFaceArr(){

	vector<int> info;
	intArrSorted* tt;

	for(int p=0; p<pat.length; p++){

		tt = ( (Patch*)pat[p] )->getFaceList();

		int numP = tt->length;

		info.clear();
		info.push_back(1); // operation type 1:=addition
		info.push_back(numP); // number of polys
		info.push_back(0);    // neighZone +
		info.push_back(-1);    // neighZone -
		info.push_back( ( (Patch*)pat[p] )->getBnd() ); // bnd
		int zTag = ( (Patch*)pat[p] )->getZDefFace();
		info.push_back( ( (Patch*)pat[p] )->getZDefFace() );   // zDefFace

		vector<Polygon*> ff;
		tt = ( (Patch*)pat[p] )->getFaceList();

		for(int i=0; i<tt->length; i++){
			ff.push_back(  (Polygon*) poly[ (*tt)[i] ] );

		}
		Face* f = new Face(10, info, ff);

		faces->add(f);

	}
}


void VRMLFile::addToFNIds(vector<int> cdId){

    fNIds.push_back(cdId);

}


/*
void VRMLFile::stitchPatch(){

	int numP = 0;
	intArrSorted** patch = NULL;
	intArr* curFL = NULL;
	intArr* pid = new intArr();


	for(int i=0; i<nfMap->size(); i++){

		curFL = nfMap->getFaceList(i);

		for(int j=0; j<curF->length; j++){
			// the sorted intArr will perform non-dup add and sorted
			pid->insert( findInPatch(curFL[j], patch, numP) );
		}
		//if only one pid found, either -1 then create new
		//                       else attach to the existing






		if(pid->length == 1){ // only one type of patch id


			if( (*pid)[0] == -1 ){ // if it is -1 means that it is not attached to any of the existing patches
				                   // should create a new patch

				numP++;
				patch = (intArrSorted**) realloc(patch, sizeof(intArrSorted*)*numP);
				patch[numP-1] = new intArrSorted();
				attachToPatch ( numP-1, patch, curFL);

			}

			else { // it is attached to an existing patch, should attached it to that patch
				attachToPatch( (*pid)[0], patch, curFL);
			}
		}
		else if(dup->length > 1) { // if there are multiple patches found


			// start with the first non -1 patch
			// attach the face to the first one
			// then stitch the rest to the first one

			int p = ( (*pid)[0] == -1)? 1 : 0;
			attachToPatch ( (*pid)[p], patch, curFL);
			for( ; p<dup->length-2; p++)
				stitch( patch, numP, p, p+1);

		}


	}




}*/



/**
  * Try to find the patch that the fc belongs to
  * return the patch id, if fc has already in one of the patches
  * return -1 if the fc is not in the patches
  */
/*
int VRMLFile::findInPatch( int fc, intArrSorted** patch, int numP){

	intArr* cur;
	for(int i=0; i<numP; i++){
		cur = patch[i];
		float id = cur->find(fc);
		int temp;
		float frac = modf(id, &temp);
		if(frac==0)
			return temp;
	}

	return -1;


}

*/

/**
  * Put all the face into the patch[id]
  * will check if the fc is a duplicate in the array
  * will perform a non-dup add
  */
/*
int VRMLFile::attachToPatch( int id, intArrSorted** patch, intArr* fl){

	for(int i=0; i<fl->length; i++)
		patch[id]->insert(fl->get(i));

}



void VRMLFile::stitch(intArrSorted** patch, int& numP, int ii, int jj){


	int sm = (ii<jj) ? ii : jj;
	int bg = (ii>jj) ? ii : jj;
	for(int i=0; i<patch[bg]->length; i++){

		patch[sm]->insert(patch[bg]->get(i) );

	}

	delete(patch[bg]);
	for(int i=bg; i<numP; i++)
		patch[i] = patch[i++];
	patch = (intArrSorted**)realloc(patch, sizeof(intArrSorted*)*(--numP) );

}

*/




















/*
void VRMLFile::stitchPatch(){

	int numP = 0;
	intArrSorted** patch = NULL;
	intArr* curFL = NULL;
	intArr* pid = new intArr();


	for(int i=0; i<nfMap->size(); i++){

		curFL = nfMap->getFaceList(i);
		
		for(int j=0; j<curF->length; j++){
			// the sorted intArr will perform non-dup add and sorted
			pid->insert( findInPatch(curFL[j], patch, numP) );
		}
		//if only one pid found, either -1 then create new
		//                       else attach to the existing

	
		
		
		
		
		if(pid->length == 1){ // only one type of patch id
			         

			if( (*pid)[0] == -1 ){ // if it is -1 means that it is not attached to any of the existing patches
				                   // should create a new patch

				numP++;
				patch = (intArrSorted**) realloc(patch, sizeof(intArrSorted*)*numP);
				patch[numP-1] = new intArrSorted();
				attachToPatch ( numP-1, patch, curFL);

			}
				
			else { // it is attached to an existing patch, should attached it to that patch
				attachToPatch( (*pid)[0], patch, curFL);
			}
		}
		else if(dup->length > 1) { // if there are multiple patches found


			// start with the first non -1 patch
			// attach the face to the first one
			// then stitch the rest to the first one

			int p = ( (*pid)[0] == -1)? 1 : 0;
			attachToPatch ( (*pid)[p], patch, curFL);
			for( ; p<dup->length-2; p++)
				stitch( patch, numP, p, p+1);
			
		}


	}
	

		

}*/



/**
  * Try to find the patch that the fc belongs to 
  * return the patch id, if fc has already in one of the patches
  * return -1 if the fc is not in the patches
  */
/*
int VRMLFile::findInPatch( int fc, intArrSorted** patch, int numP){

	intArr* cur;
	for(int i=0; i<numP; i++){
		cur = patch[i];
		float id = cur->find(fc);
		int temp;
		float frac = modf(id, &temp);
		if(frac==0)
			return temp;
	}

	return -1;


}

*/

/**
  * Put all the face into the patch[id]
  * will check if the fc is a duplicate in the array
  * will perform a non-dup add
  */
/*
int VRMLFile::attachToPatch( int id, intArrSorted** patch, intArr* fl){

	for(int i=0; i<fl->length; i++)
		patch[id]->insert(fl->get(i));

}



void VRMLFile::stitch(intArrSorted** patch, int& numP, int ii, int jj){


	int sm = (ii<jj) ? ii : jj;
	int bg = (ii>jj) ? ii : jj;
	for(int i=0; i<patch[bg]->length; i++){

		patch[sm]->insert(patch[bg]->get(i) );

	}

	delete(patch[bg]);
	for(int i=bg; i<numP; i++)
		patch[i] = patch[i++];
	patch = (intArrSorted**)realloc(patch, sizeof(intArrSorted*)*(--numP) );

}

*/


















/**
>>>>>>> .r256
 * Function will set all the pointers to NULL, and delete all the memory allocated
 * child node will be deleted
 */
/*
void VRMLFile::resetNode(){
	if(curNode->type) {
		free(curNode->type);
		curNode->type = NULL;
	}
	if(curNode->name){
		free(curNode->name);
		curNode->name = NULL;
	}
	if(curNode->fldTyp){
		for(int i=0; i<curNode->numFld; i++){
			free( (curNode->fldTyp)[i] );
			free( (curNode->fldVal)[i] );
		}
	}
	free(curNode->fldTyp);
	curNode->fldTyp = NULL;
	free(curNode->fldVal);
	curNode->fldVal = NULL;

	if(curNode->cld){
		for(int i=0; i<curNode->numCld; i++){
			deleteNode(curNode->cld[i]); //delete the memory allocated inside the node
			free(curNode->cld[i]);
		}
	}
	free(curNode->cld);
	curNode->cld = NULL;




}
*/
/**
 * Recursive function will delete all the memory that a node allocated
 * also delete the memory allocated by child node
 */
/*
void VRMLFile::deleteNode(Node* n){

	if(curNode->type) {
		free(curNode->type);
		curNode->type = NULL;
	}
	if(curNode->name){
		free(curNode->name);
		curNode->name = NULL;
	}
	if(curNode->fldTyp){
		for(int i=0; i<curNode->numFld; i++){
			free( (curNode->fldTyp)[i] );
			free( (curNode->fldVal)[i] );
		}
	}
	free(curNode->fldTyp);
	curNode->fldTyp = NULL;
	free(curNode->fldVal);
	curNode->fldVal = NULL;

	if(curNode->cld){
		for(int i=0; i<curNode->numCld; i++){
			deleteNode(curNode->cld[i]); //delete the memory allocated inside the node
			free(curNode->cld[i]);
		}
	}
	free(curNode->cld);
	curNode->cld = NULL;



}
*/
