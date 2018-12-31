#include <iostream>
#include <fstream>
//#include <unistd.h>

#include "stdafx.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "Project.h"
#include "lib.h"
#include "geoModel.h"

#include "OcTree.h"
#include "Contour.h"
#include "time.h"


#include "windows.h"

#ifdef _DEBUG
   #ifndef DBG_NEW
      #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
      #define new DBG_NEW
   #endif
#endif  // _DEBUG



using namespace std;


int main(int argc,char *argv[]){


//	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

//	_crtBreakAlloc = 1;

//	printf("Evaluation mesh generation program, will expire by 04/30/2012.\n");

	time_t rawtime;
	tm * ptm;
	time (&rawtime);
	ptm = gmtime(&rawtime);
/*	if( ptm->tm_year>=113 && ptm->tm_mon>=3 ){
		printf("The program is expired after 04/30/2012.\nProgram will exit in 10 seconds...");
		Sleep(10000);
		return 0;
	}
*/

	

	clock_t start = clock();
	cout<<"start clock: "<<start<<endl;
	double userElemSize[2];
	try{
		if(argc<4) //
			throw StoppingException("Not enough arguments!");

		int domMode=1;
		int extMode=0;
		double extMultiplier = 1.0;

		bool debug = false;

		bool binOut = false;
		
		bool uniform= false;

		
		for(int i=0; i<argc; i++){
			if( strcmp(argv[i], "-uniform") == 0 ){
				uniform = true;;
			}
			else if(strcmp(argv[i], "-noAddDomain") == 0 ){
				domMode = 0;
			}
			else if(strcmp(argv[i], "-addDomain") == 0 ){
				domMode = 1;
			}
			else if(strncmp(argv[i], "-extDomain", 10) == 0 ){
				domMode = 2;
				char temp[10];
				int len = strlen(argv[i]);
				for(int j = 11; j<len; j++){
					temp[j-11] = argv[i][j];
				}
				temp[len-11]='\0';
				from_string<double>(extMultiplier, temp, std::dec);
 
			}
			else if( strcmp(argv[i], "-fullTree") == 0 ){
				extMode = 1;
			}
			else if( strcmp(argv[i], "-debug")    == 0 ){
				debug = true;
			}
			else if( strcmp(argv[i], "-binOut")   == 0 ){
				binOut = true;
			}
			else
				continue;
						
		}

		if(binOut)
			cout<<"Output will be in binary"<<endl;

		if( !uniform ){

			from_string<double>(userElemSize[0], argv[2], std::dec);
			from_string<double>(userElemSize[1], argv[3], std::dec);

			if(userElemSize[0]<=0 || userElemSize[1]<=0)
				throw StoppingException("Specified cell sizes are not correct.\n");

		}
		else {
			from_string<double>(userElemSize[0], argv[2], std::dec);
			userElemSize[1] = userElemSize[0];
		}

		char* fName = NULL;
		char* ext = NULL;
		int pos = strlen(argv[1]);

	//	cout<<argv[1]<<endl;
		
		if(pos<4){

			printf("Error parsing geometry input file name <%s>.\n", argv[1]);
			throw StoppingException("\n");
		}
		pos -= 4;
		
		if(  *(argv[1]+sizeof(char)*pos)== '.'){
			
			fName = (char*) malloc( sizeof(char)*(pos+1) );
			ext = (char*) malloc( sizeof(char)*4 );
			strncpy( fName, argv[1], pos );
			fName[pos] = '\0';
			strcpy( ext, argv[1]+ (pos+1)*sizeof(char) );

		}
		else {
			printf("Error parsing geometry input file name <%s>.\n", argv[1]);
			throw StoppingException("\n");
		}
	


		geoModel myGM;
		myGM.buildModel(fName, ext, domMode, extMode, extMultiplier);


		if(debug){
			myGM.printGeoFaces();
			myGM.printGeoFeatureNodes();
		}

		cout<<"Total number of faces read in: \t"<<myGM.getTotolNumberOfFace()<<endl;
		cout<<"Domain size: \t"<<myGM.domainSize[0]<<"\t"<<myGM.domainSize[1]<<"\t"<<myGM.domainSize[2]<<endl;
		cout<<"Min size in domain: \t"<<myGM.shortestDistanceInModel()<<endl;
//		cout<<"Tree center: \t("<<myGM.domainCenter[0]<<", "<<myGM.domainCenter[1]<<", ";
//		cout<<myGM.domainCenter[2]<<")"<<endl;


		/********************TESTING******************************************/
 /*   	__int64 ii(4106);
		ii *= 10000000;
		ii +=  4503855;



		intArr pfc, *fc, *nd;
		pfc.add(43);
		pfc.add(44);
		pfc.add(91);

		double base[3]={-0.463192,13.941081,-0.309641};
		double size=0.499985;
//		double intPnt[3];
		geoNodeArr v;
		__int64 f1= 43;
		__int64 f2= 44;
		__int64 f3= 91;
		
		myGM.getMidPointOfIntersectionLineWithCube(f1, f2, base, size, v);
		myGM.getMidPointOfIntersectionLineWithCube(f1, f3, base, size, v);
		myGM.getMidPointOfIntersectionLineWithCube(f2, f3, base, size, v);

//		myGM.getInterFaceId(base, size, &fc, &pfc);
	//	myGM.getInnerNodeId(x, size, &nd);
		double diag[3];
	    __int64 z1=-1, z2=-1, b1=-1, b2=-1;

	    for(__int64 i=0; i<3; i++)
	        diag[i] = base[i] + size;

	//    myGM.getZoneOrBndId(base, z1, b1);
	//    myGM.getZoneOrBndId(diag, z2, b2);

		/********************TESTING******************************************/
	    /*********************************************************************/
	    /*********************************************************************/
	    /*********************************************************************/



		vtxArr    *vtx;
	    elemArr   *elem;
	    faceArr   *face;
	    vtx  = new vtxArr(1);
	    elem = new elemArr(1);
	    face = new faceArr(1000);


	    OcTree *myOT = new OcTree(myGM, userElemSize, 1, domMode, extMode, uniform);

//	    double move[3]={0.002336,0.360157,-0.12034};
//	    double move[3]={0,0,0};
//		myOT->moveGrid(move);
		myOT->OTConstruct();
		cout<<"finish otConstruction"<<endl;
		
		if(debug)
			myOT->printOtInfo(vtx);


		Contour *myCT = new Contour(&myGM, myOT, vtx, elem, face, fName, binOut);
		myCT->doContour();

		clock_t end = clock();
		clock_t seconds = (end-start)/CLOCKS_PER_SEC;
		cout<<"processor time used (second): "<<seconds<<endl;

		if(debug)
			myOT->printOtInfo(vtx);

		cout<<"Output to file..."<<endl;
	//	toMeshFile(fName, vtx, elem);

		cout<<"finished output to file..."<<endl;


		delete(vtx);
		delete(elem);
//		delete(face);
		delete(myOT);

		if(fName)
			free(fName);
		if(ext)
			free(ext);
/*		if(tokens){
			free(tokens[0]);
			free(tokens[1]);
			free(tokens);
		}
		*/

	}catch(MyApplicationException &e){
		cout<<e.message()<<endl;
	}
	catch(...){

		fprintf(stderr, "Exceptions found.\n");

	}

	char a;
//	std::cin>>a;
	cout<<"program will exit in 30 seconds."<<endl;
	Sleep(30000);


//	_CrtDumpMemoryLeaks();

	return 0;
}
