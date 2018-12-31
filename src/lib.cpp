/*
 * lib.c
 *
 *  Created on: Mar 17, 2009
 *      Author: zrui
 */

#include "stdafx.h"
#include "lib.h"
#include "geoLib.h"
#include <math.h>
#include "stdlib.h"
#include "memory.h"

void getFullFilePath( char** fullFilePath, char* fName, char* postfix){

	char* FILEPATH;
	FILE* pathFile = fopen("path", "r");

	if(!pathFile){
		FILEPATH = (char*) malloc(sizeof(char));
		FILEPATH[0] = '\0';
	}

	else{
	
		fgetline_123( & FILEPATH, pathFile);
	
		if(!FILEPATH){
			printf("error reading path file, file is empty. Please provide the full path of the directory containing the .wrl file.");
			throw StoppingException("\n");
		}

		fclose(pathFile);

	}

	if (postfix) {
		*fullFilePath = (char*) realloc (*fullFilePath, strlen(fName) + strlen(FILEPATH) + strlen(postfix)+3);
		strcpy(*fullFilePath, FILEPATH);
		if(FILEPATH[0] != '\0')
			strcat(*fullFilePath, "\\");
		strcat(*fullFilePath, fName);
		strcat(*fullFilePath, postfix);
	}
	else{
		*fullFilePath = (char*) realloc (*fullFilePath, strlen(fName) + strlen(FILEPATH) +3);
		strcpy(*fullFilePath, FILEPATH);
		if(FILEPATH[0] != '\0')
			strcat(*fullFilePath, "\\");
		strcat(*fullFilePath, fName);

	}

	free(FILEPATH);

}
/*
   This is the implementation of the |getline| library.

   modified June 2007
   modified 24 January 2006 by Arthur O'Dwyer.

   Public domain.
*/


static void try_resize(char **p, size_t *cap);
static char *try_exact_fit(char **p, size_t len);


/*
   The |fgetline_notrim| function reads characters from |stream| and
   stores them into a dynamically allocated buffer pointed to by |*s|.
   (The old value of |*s| is ignored by this function.)
   Reading stops after an |EOF| or a newline. If a newline is read,
   it is stored into the buffer.

   |fgetline_notrim| returns |p| on success; and |NULL| on I/O error,
   or when end-of-file occurs while no characters have been read,
   or when the line length exceeds |(size_t)-1|,
   or when a call to |malloc| returns |NULL|. In all four cases,
   the characters which have been read so far, if any, still
   reside in the dynamically allocated buffer |*p|.
*/
char *fgetline_notrim(char **p, FILE *stream)
{
    size_t cap = 0;
    size_t len = 0;
    char *rc;

    *p = NULL;

    while (1) {
        try_resize(p, &cap);
        if (cap-1 <= len) {
            /* |try_resize| failed */
            return NULL;
        }
        rc = fgets(*p+len, cap-len, stream);
        if (rc == NULL) {
            /*
               EOF or input error. In either case, once |NULL| has
               been returned, the contents of the buffer are unusable.
            */
            (*p)[len] = '\0';
            try_exact_fit(p, len);
            if (len == 0 && feof(stream))
            {
	       free(*p);
	       *p = NULL;
	    }
	    return *p;
        }
        else if (strchr(*p+len, '\n') != NULL) {
            /* a newline has been read */
            return try_exact_fit(p, len+strlen(*p+len));
        }
        else {
            /* we must continue reading */
            len += strlen(*p+len);
            if (feof(stream))
              return try_exact_fit(p, len);
        }
    }
}


/*
   The |try_resize| function tries to resize |*p| so it can hold more
   data. It will always yield a valid, consistent |*p| and |*cap| ---
   |*cap| will never decrease, and no data will be lost from |*p|.
   But if a call to |realloc| fails, |*cap| will be unchanged.
   One important thing to notice: We must never increase |*cap| by
   more than |INT_MAX|, since the second parameter to |fgets| is of
   type |int|.
*/
static void try_resize(char **p, size_t *cap)
{
    /*
       We aren't expecting any really long lines, here. But if the
       current line has exceeded 500 characters, there's probably
       something special going on (like an attempted buffer overflow
       attack), and we'll start increasing the buffer capacity
       geometrically.
    */
    size_t newcap = (*cap < 500)? (*cap + 16):
                    (*cap/2 < INT_MAX)? (*cap + *cap/2):
                    (*cap + INT_MAX);
    char *newp;
    if (newcap < *cap) {
        /* The line length has exceeded |(size_t)-1|. Wow! */
        if (*cap == (size_t)-1) return;
        else newcap = (size_t)-1;
    }

    newp = (char*) realloc(*p, newcap);
    /* Maybe we can't get that much memory. Try smaller chunks. */
    while (newp == NULL) {
        newcap = *cap + (newcap - *cap)/2;
        if (newcap == *cap) break;
        newp = (char*) realloc(*p, newcap);
    }

    if (newp != NULL)
      *p = newp;

    /* At this point, |*p| hasn't lost any data, and |newcap| is valid. */
    *cap = newcap;
    return;
}


/*
   The |try_exact_fit| function tries to resize the given buffer to
   exactly the right size to fit its contents, counting the null
   terminator. If that fails, we just return the original buffer.
*/
static char *try_exact_fit(char **p, size_t len)
{
    char *r = (char*) realloc(*p, len+1);
    if (r != NULL) *p = r;
    return *p;
}



/*
   The function |trim_123| just slaps a zero byte into the position
   after the last non-whitespace character in the given string. It
   is used to strip off newlines from the results of |fgetline_123|
   and |getline_123|. You can also call it explicitly on the output
   of |getline_notrim|, or any other string, for that matter.

   If the parameter is |NULL|, this function will return |NULL|.
   Otherwise, the zero byte is written and the resulting string is
   returned.
*/
char *trim_123(char *s)
{
    char *end;
    if (s == NULL)
      return NULL;
    end = strchr(s, '\0');
    while (end > s && isspace((unsigned char)end[-1]))
      --end;
    *end = '\0';
    return s;
}


/*
   The |fgetline_123| function is just like |fgetline_notrim|, except
   that it calls |trim_123| on the resulting string before returning.
*/
char *fgetline_123(char **line, FILE *fp)
{
    char *rc = fgetline_notrim(line, fp);
    trim_123(*line);
    return rc;
}


/*
   The |getline_notrim| function is the same as the |fgetline_notrim|
   function, except that |stream| is given the value of |stdin|.
*/
char *getline_notrim(char **p)
{
    return fgetline_notrim(p, stdin);
}


/*
   The |getline_123| function is the same as the |fgetline_123|
   function, except that |stream| is given the value of |stdin|.
*/
char *getline_123(char **p)
{
    return fgetline_123(p, stdin);
}



char ** split(char * orignalLine,   __int64 * tokenCnt)
{
	char *line = (char*) malloc( (strlen(orignalLine)+1) *sizeof(char*) );
	strcpy(line, orignalLine);
	char * token;
	char **tokensArray=NULL;
	 __int64 i;
	*tokenCnt = 0;

	token = strtok( line, " ()\t,");

	if (!token) return 0;

	(*tokenCnt)++;
	tokensArray = (char**) realloc(tokensArray, (*tokenCnt)*sizeof(char*));
	tokensArray[(*tokenCnt)-1] = (char*) malloc ( (strlen(token)+1) * sizeof(char) );

	for(i=0; i<(__int64)strlen(token); i++)
		tokensArray[(*tokenCnt)-1][i] = token[i];

	tokensArray[(*tokenCnt)-1][strlen(token)] = '\0';

	while( (token = strtok( NULL, " ()\t")) ) {
		(*tokenCnt)++;
		tokensArray = (char**) realloc(tokensArray, (*tokenCnt)*sizeof(char*));
		tokensArray[(*tokenCnt)-1] = (char*) malloc ( (strlen(token)+1) * sizeof(char) );

		for(i=0; i<(__int64)strlen(token); i++)
			tokensArray[(*tokenCnt)-1][i] = token[i];

		tokensArray[(*tokenCnt)-1][strlen(token)] = '\0';

	}
	free(line);

/*	IF YOU FIND A FIRST TOKEN MaLLOC THE ARRAY TO LENGTH 1
	DEEP COPY THAT TOKEN INTO THe HEAP
	STORE ITS PTR INTO ARRAY[0]
	PARSE REMAINInG TOKENS IN THE sTRING
	WITH EACH TOKEN FOUND REALLOC THE ARRAY OF CHAR *s ONE BIGGER (dont double it)
	MAKE A DEEP COPY OF THE TOKEN AND STORE ITS PTR INTO THE ARRAY

	WHEN FINSIHED YOU WILL HAVE N TOKENS AND THE ARRAY WILL BE LENGTH N
*/
	return tokensArray;
}

char ** split(char * orignalLine,   __int64 & tokenCnt, char* deliminators)
{
	char *line = (char*) malloc( (strlen(orignalLine)+1) *sizeof(char*) );
	strcpy(line, orignalLine);
	char * token;
	char **tokensArray=NULL;
	 __int64 i;
	tokenCnt = 0;

	token = strtok( line, deliminators);

	if (!token) return 0;

	(tokenCnt)++;
	tokensArray = (char**) realloc(tokensArray, (tokenCnt)*sizeof(char*));
	tokensArray[(tokenCnt)-1] = (char*) malloc ( (strlen(token)+1) * sizeof(char) );

	for(i=0; i<(__int64)strlen(token); i++)
		tokensArray[(tokenCnt)-1][i] = token[i];

	tokensArray[(tokenCnt)-1][strlen(token)] = '\0';

	while( (token = strtok( NULL, deliminators)) ) {
		(tokenCnt)++;
		tokensArray = (char**) realloc(tokensArray, (tokenCnt)*sizeof(char*));
		tokensArray[(tokenCnt)-1] = (char*) malloc ( (strlen(token)+1) * sizeof(char) );

		for(i=0; i<(__int64)strlen(token); i++)
			tokensArray[(tokenCnt)-1][i] = token[i];

		tokensArray[(tokenCnt)-1][strlen(token)] = '\0';

	}
	free(line);

/*	IF YOU FIND A FIRST TOKEN MaLLOC THE ARRAY TO LENGTH 1
	DEEP COPY THAT TOKEN INTO THe HEAP
	STORE ITS PTR INTO ARRAY[0]
	PARSE REMAINInG TOKENS IN THE sTRING
	WITH EACH TOKEN FOUND REALLOC THE ARRAY OF CHAR *s ONE BIGGER (dont double it)
	MAKE A DEEP COPY OF THE TOKEN AND STORE ITS PTR INTO THE ARRAY

	WHEN FINSIHED YOU WILL HAVE N TOKENS AND THE ARRAY WILL BE LENGTH N
*/
	return tokensArray;
}

void freeTokenArray( char ** tokensArray,  __int64 tokenCnt )
{
	/* free each token hanging off the array of char *'s
	   then free the array of char *'s
	*/
	 __int64 i;
	for(i=0; i<tokenCnt; i++){
		free(tokensArray[i]);
	}
	free(tokensArray);
	tokensArray = NULL;
	tokenCnt = 0;
}
void Instantiations::Instantiate()
 {
	int i;
	from_string<int>(i, std::string("ff"), std::hex);
	__int64 ii;
	from_string<__int64>(ii, std::string("ff"), std::hex);

	float f;
	from_string<float>(f, std::string("f.f"), std::dec);
	double d;
	from_string<double>(d, std::string("f.f"), std::dec);

 }


template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  if((iss >> f >> t).fail()){
	  printf("Error: Input data convertion failed.\n");
	  return false;
  }
  else
	  return true;
//  return !(iss >> f >> t).fail();
}


void toRawcFile(char* fName, vtxArr* vtx, faceArr* face){

    char* fullFilePath = NULL;
    getFullFilePath(&fullFilePath, fName, "_quad.rawc");
    FILE* file = fopen(fullFilePath, "w");

    fprintf(file, "%d %d \n", vtx->length, face->length);
    float r, g, b;
    for( __int64 i =0; i<vtx->length; i++){
         __int64 bnd= vtx->getBnd(i)==0;
        r=bnd; g=(bnd*10)%255; b=(bnd*100)%255;
        double *x;
        x = vtx->getCoord(i);
        fprintf(file, "%f %f %f %f %f %f\n",
                x[0], x[1], x[2], r, g, b);
    }
    for ( __int64 i=0; i<face->length; i++){
    	__int64 *e;
    	e = face->getFace(i);

        if(e[3] != -1)
            fprintf(file, "%d %d %d %d \n", e[0], e[1], e[2], e[3]);
        else
            fprintf(file, "%d %d %d %d \n", e[0], e[1], e[2], e[2]);

    }

}

void toRawFile(char* fName, vtxArr* vtx, faceArr* face, elemArr* elem){

    char* fullFilePath = NULL;
    getFullFilePath(&fullFilePath, fName, "_quad.raw");
    FILE* file = fopen(fullFilePath, "w");


     __int64 bnd=7;
     __int64 eleCnt= 0;
    for ( __int64 i=0; i<face->length; i++){
    	__int64 *e;
    	e = face->getFace(i);
    	if( (* vtx->getBnd(e[0]))[0] == bnd
    			|| (*vtx->getBnd(e[1]))[0] == bnd
    			|| (*vtx->getBnd(e[2]))[0] == bnd){
    		eleCnt++;
    	}
    }

     __int64 vCnt=0;
    for ( __int64 i=0; i<vtx->length; i++){
    	if( (*vtx->getBnd(i))[0] ==7)
    		vCnt++;
    }

    fprintf(file, "%d %d \n", vtx->length, face->length);
    for( __int64 i =0; i<vtx->length; i++){
        double *x;
        x = vtx->getCoord(i);
//    	if(vtx->getBnd(i) ==7){
//    		double r = sqrt( (x[0]-5)*(x[0]-5)+(x[1]-5)*(x[1]-5)+(x[2]-5)*(x[2]-5));
//   	if(  r < 1.9 ){
//    		fprintf(file, "inside the sphere, r=%f\n", r);

//        if(x[0]>10 || x[0]<0 || x[1] >10 || x[1]<0 || x[2]>10 ||x[2]<0)
//        	fprintf(file, "error this one: i= %d ", i);
          fprintf(file, "%f %f %f\n",
                  x[0], x[1], x[2]);
//  	}
//    	}
    }


   for ( __int64 i=0; i<face->length; i++){
	   __int64 *f;
    	f = face->getFace(i);
 //   	double *x;
//    	 __int64 flg=0;
//    	for( __int64 j=0; j<4; j++){
//        	if(f[j]==-1)continue;
//    		x = vtx->getCoord(f[j]);
//        	if(x[0]>5 || x[2] >6 ){
//        		flg =1;
//        		break;
//        	}
//    	}
//    	if(flg==1)
 //   		continue;

  //  	if(vtx->getBnd(f[0])== 7 && vtx->getBnd(f[1]) ==7 && vtx->getBnd(f[2]) ==7 && vtx->getBnd(f[3]) == 7)

 //   	if(f[0]==615 || f[1] ==615 || f[2] == 615 ||f[3]==615)
  //  		if(f[0]==616 || f[1] ==616 || f[2] ==616 || f[3]==616)

    	fprintf(file, "%d %d %d %d\n", f[0], f[1], f[2], f[3]);

   }





    fclose(file);
    free(fullFilePath);



    char* bndFilePath = NULL;
    getFullFilePath(&bndFilePath, fName, ".bnd");
    FILE* bndFile = fopen(bndFilePath, "w");
    for( __int64 i=0; i<vtx->length; i++)
    	fprintf(bndFile, "%d\n", vtx->getBnd(i));

    fclose(bndFile);
    free(bndFilePath);




}


void toSurfaceMesh(char* fName,  __int64 bnd, vtxArr* vtx, faceArr* face, elemArr* elem){
    char* fullFilePath = NULL;
    getFullFilePath(&fullFilePath, fName, "_surface_quad.raw");
    FILE* file = fopen(fullFilePath, "w");

     __int64 nv=0;
    for( __int64 i=0; i<vtx->length; i++){
    	if( (*vtx->getBnd(i)).has(bnd))
    		nv++;
    }
    fprintf(file, "%d %d \n", nv, face->length);

     __int64 *map = (__int64*) malloc(sizeof(__int64)*vtx->length);
    for( __int64 i=0; i<vtx->length; i++)map[i] = -1;
     __int64 cnt=0;
    double *x;
    for( __int64 i=0; i<vtx->length; i++){

    	if( (vtx->getBnd(i))->has(bnd) ){
    		x = vtx->getCoord(i);
    		fprintf(file, "%f %f %f\n", x[0], x[1], x[2]);
    		map[i] = cnt++;
    	}

    }
    for( __int64 i=0; i<face->length; i++){
    	__int64 *f;
     	f = face->getFace(i);
    	double *x;
     	 __int64 flg=0;
     	for( __int64 j=0; j<4; j++){
         	if(f[j]==-1)continue;
     		x = vtx->getCoord(f[j]);
         	if(x[0]>5 || x[2]>6){
  //       		flg =1;
         		break;
         	}
     	}
     	if(flg==1)
     		continue;
    	if(map[f[0]]!= -1 && map[f[1]]!= -1 && map[f[2]]!= -1 && map[f[3]]!= -1){
        	fprintf(file, "%d %d %d %d\n", map[f[0]], map[f[3]], map[f[2]], map[f[1]]);
     	}
    }

    fclose(file);
    free(map);

}




void toMeshFile(char* fName, vtxArr* vtx, elemArr* elem){

    char* fullFilePath = NULL;
    getFullFilePath(&fullFilePath, fName, ".mesh");
    FILE* file = fopen(fullFilePath, "w");

    fprintf(file, "%lld %lld \n", vtx->length, elem->length);
    for( __int64 i =0; i<vtx->length; i++){
   // 	cout<<"outputing "<<i<<endl;
        double *x;
        x = vtx->getCoord(i);
        fprintf(file, "%f %f %f %lld ",
                x[0], x[1], x[2], vtx->getZone(i));
        intArr* bnd = vtx->getBnd(i);
        if(bnd){
			for(__int64 j=0; j<bnd->length; j++){
				fprintf(file, "%lld ", (*bnd)[j] );
			}
 		}
 		else {
 			fprintf(file, "%lld ", -1 );
 		}

 		fprintf(file, "\n");

    }
     __int64 flg=0;
    for( __int64 i=0; i<elem->length; i++){
    	__int64 *f = elem->getElem(i);
 //    	double *x;
 //    	for( __int64 j=0; j<8; j++){
 //        	if(f[j]==-1)continue;
 //    		x = vtx->getCoord(f[j]);
 //        	if(x[0]>5 || x[2] >6 ){
 //        		flg =1;
 //        		break;
 //        	}
 //    	}
//     	if(flg==0)
//     		continue;


//	    __int64 *f;
//	   f = elem->getElem(i);
     	flg=0;
 /*    	 __int64 vv[14]= {428, 429, 507, 508, 509, 510, 516, 517, 518, 519, 534, 535, 536, 537};
	   for( __int64 j=0; j<8; j++){
		   for( __int64 k=0;k<14;k++){
			   if(f[j]== 537) {
				   flg=1;break;
			   }
		   }
	   }
	   if(flg)*/
		   fprintf(file, "%lld %lld %lld %lld %lld %lld %lld %lld \n", f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7]);

    }
    fclose(file);
    free(fullFilePath);
}


void toMshFile(char* fName, vtxArr* vtx, elemArr* elem){



}




/**
 *
 *  function to find the duplications in the array
 *  @param a array of the int, from which to find the dups
 *  @param n number of ints in the array
 *  @param defa a value that is not in the array, which will be used in this function
 *  @param dup the array to return that contains the distinct ints with number of its instance
 *             this function does not allocate memory, caller function should allocate enough 
 *             memeory, which is assumed to be the same size of original  __int64 array
 *  @param cnt the number of distinct ints
 *
 *
 */
void getDup(__int64* a,  __int64 n,  __int64 defa,  __int64 (*dup)[2],  __int64 &cnt){

    for( __int64 i=0; i<n; i++){
        dup[i][0] = defa; dup[i][1] = 0;
    }
    cnt=0;

    for( __int64 i=0; i<n; i++){
        for( __int64 j=0; j<n; j++){
			if(a[i] == dup[j][0]){
				dup[j][1]++;
				break;
			}
			else if(dup[j][0] == defa){
				dup[cnt][0] = a[i];
				dup[cnt++][1] = 1;
				break;
			}
			else
				continue;
        }
    }
}

void getDup(intArr* a,  __int64 defa, intArr* dup, intArr* dupCnt){

    for( __int64 i=0; i<a->length; i++){
        dup[i] = defa; dupCnt[i] = 0;
    }
    

    for( __int64 i=0; i<a->length; i++){
        for( __int64 j=0; j<a->length; j++){
			if((*a)[i] == (*dup)[j]){
				(*dupCnt)[j] += 1;
				break;
			}
			else if((*dup)[j] == defa){
				dup->add( (*a)[i] );
				dupCnt->add(1);

				break;
			}
			else
				continue;
        }
    }
}

void getDup(intArr** a,  int numArr, __int64 defa, intArr* dup, intArr* dupCnt){


    int dcnt=0;
    for( __int64 i=0; i<numArr; i++){

    	for(__int64 k=0; k<a[i]->length; k++){

    		__int64 tt = (*a[i])[k];

    		bool flg = false;

            for( __int64 j=0; j<dcnt; j++){

            	if( tt == (*dup)[j] ){
    				(*dupCnt)[j] += 1;
    				flg = true;
    				break;
    			}
            }
            if(!flg){
    			(*dup)[dcnt] = tt;
    			(*dupCnt)[dcnt] = 1;
    			dcnt++;
    		}
    	}
    }

}




