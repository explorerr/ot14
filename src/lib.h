
#ifndef LIB_
#define LIB_

#include "Project.h"
#include "Exceptions.h"
#include "vtxArr.h"
#include "elemArr.h"
#include "faceArr.h"
#include "intArr.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>

//#define FILEPATH "C:\\Documents and Settings\\zrui.ZRUIBLDSIMGRP\\Desktop\\cProjects\\3DMeshFiles"
//#define FILEPATH "C:\\Documents and Settings\\Rui Zhang\\Desktop\\cProject\\3DMeshFiles"





void getFullFilePath( char** fullFilePath, char* fName, char* postfix);
/*
   Call as "rc = getline_123(&line);"
   These two functions trim off the trailing newline (and any other
   trailing whitespace) for you, so if the user types <space>-H-E-L-
   L-O-<space>-<enter>, all you see is <space>-H-E-L-L-O.
*/
char *getline_123(char **p);
char *fgetline_123(char **p, FILE *stream);
/*
   Same as above, but these two don't trim newlines or whitespace.
   If the user types <space>-H-E-L-L-O-<space>-<enter>, the resulting
   string will contain <space>-H-E-L-L-O-<space>-<newline>.
*/
char *getline_notrim(char **p);
char *fgetline_notrim(char **p, FILE *stream);
/*
   This function is provided for your convenience.
   "getline_123(&p)" behaves as if "trim_123" were called on the
   resulting string before returning from the function.
*/
char *trim_123(char *line);


char ** split(char * line,   __int64 * tokenCnt);
char ** split(char * orignalLine,   __int64 & tokenCnt, char* deliminators);

void freeTokenArray( char ** tokensArray,  __int64 tokenCnt );




template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&));

class Instantiations
 {
 private:
   void Instantiate();
 };


void toRawFile(char* fName, vtxArr* vtx, faceArr* face, elemArr* elem);
void toRawcFile(char* fName, vtxArr* vtx, faceArr* face);

void toRawFile(char* fName, vtxArr* vtx, elemArr* elem);
void toSurfaceMesh(char* fName,  __int64 bnd, vtxArr* vtx, faceArr* face, elemArr* elem);


void toMeshFile(char* fName, vtxArr* vtx, elemArr* elem);

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
void getDup(__int64* a,  __int64 n,  __int64 defa,  __int64 (*dup)[2],  __int64 &cnt);
void getDup(intArr* a,  __int64 defa, intArr* dup, intArr* dupCnt);
void getDup(intArr** a, int numArr, __int64 defa, intArr* dup, intArr* dupCnt);


#endif
